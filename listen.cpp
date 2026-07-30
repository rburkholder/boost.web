#include <boost/log/trivial.hpp>

#include <boost/url.hpp>

#include <boost/beast/ssl.hpp>

#include "listen.hpp"
#include "mime_type.hpp"

namespace http      = beast::http;
namespace websocket = beast::websocket;

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string
path_cat(
  beast::string_view base,
  beast::string_view path
)
{
  if(base.empty())
    return std::string(path);
  std::string result(base);

#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
  char constexpr path_separator = '/';
  if ( result.back() == path_separator )
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());
#endif
  return result;
}

using response_t = http::response<http::string_body>;

// Returns a bad request response
template<class Body, class Allocator>
response_t bad_request( http::request<Body, http::basic_fields<Allocator>>& request, beast::string_view why ) {
  http::response<http::string_body> response{ http::status::bad_request, request.version() };
  response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response.set(http::field::content_type, "text/html");
  response.keep_alive( request.keep_alive() );
  response.body() = std::string(why);
  response.prepare_payload();
  return response;
};

// Returns a not found response
template<class Body, class Allocator>
response_t not_found( http::request<Body, http::basic_fields<Allocator>>& request, beast::string_view target ) {
  http::response<http::string_body> response{ http::status::not_found, request.version() };
  response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response.set(http::field::content_type, "text/html");
  response.keep_alive( request.keep_alive() );
  response.body() = "The resource '" + std::string(target) + "' was not found.";
  response.prepare_payload();
  return response;
};

// Returns a server error response
template<class Body, class Allocator>
response_t server_error( http::request<Body, http::basic_fields<Allocator>>& request, beast::string_view what ) {
  http::response<http::string_body> response{ http::status::internal_server_error, request.version() };
  response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  response.set(http::field::content_type, "text/html");
  response.keep_alive( request.keep_alive() );
  response.body() = "An error occurred: '" + std::string(what) + "'";
  response.prepare_payload();
  return response;
};

struct state_t {

  bool bSsl;
  const char* ssl_name;
  net::ip::tcp::endpoint endpoint;
  beast::string_view doc_root;

  state_t(): bSsl( true ), ssl_name( nullptr ) {}
  state_t( boost::string_view doc_root_, net::ip::tcp::endpoint endpoint_ )
  : bSsl( true ), ssl_name( nullptr )
  , doc_root( doc_root_ ), endpoint( endpoint_ ) {}
};

// Return a response for the given request.
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template<class Body, class Allocator>
http::message_generator
handle_request(
  state_t state,
  http::request<Body, http::basic_fields<Allocator>>&& request
)
{

  // Make sure we can handle the method
  if( request.method() != http::verb::get &&
      request.method() != http::verb::head
  ) {
    BOOST_LOG_TRIVIAL(warning)
      << state.endpoint.address() << ':' << state.endpoint.port() << " "
      << "unknown method: '"
      << request.method() << "'," << request.has_content_length()
      ;
    return bad_request( request, "Unknown HTTP-method" );
  }

  boost::system::result<boost::urls::url_view> url = boost::urls::parse_origin_form( request.target() );
  if ( url.has_error() ) {
    BOOST_LOG_TRIVIAL(warning)
      << state.endpoint.address() << ':' << state.endpoint.port() << " "
      << "server error: " << request.target();
    return server_error( request, request.target() );
  }

  const auto path_raw( url->path() );
  // Request path must be absolute and not contain "..".
  if( path_raw.empty() ||
      path_raw[0] != '/' ||
      path_raw.find("..") != beast::string_view::npos
  ) {
    BOOST_LOG_TRIVIAL(warning)
      << state.endpoint.address() << ':' << state.endpoint.port() << " "
      << "illegal target: "
      << path_raw;
    return bad_request( request, "Illegal request-target" );
  }

  std::string path = path_cat( state.doc_root, path_raw );
  if ( '/' == path.back() ) {
    path.append( "index.html" );
  }

  BOOST_LOG_TRIVIAL(info)
    << state.endpoint.address() << ':' << state.endpoint.port() << " "
    << "request: "
    << ( state.bSsl ? ( ( nullptr == state.ssl_name ) ? "unnamed" : state.ssl_name ) : ( "http") ) << ", "
    << request.method() << ", '" << request.target() << "', '" << path << "', '" << url->query() << "'";

  // Attempt to open the file
  beast::error_code ec;
  http::file_body::value_type body;
  body.open( path.c_str(), beast::file_mode::scan, ec );

  if( ec ) {
    // Handle the case where the file doesn't exist
    if( ec == beast::errc::no_such_file_or_directory ) {
      // similar for ads.txt, sitemap.xml
      if ( 0 == request.target().compare( "/robots.txt" ) ) {
        static const std::string content( "User-agent: *\nAllow: /\n" );
        http::response<http::string_body> response{
          std::piecewise_construct,
          std::make_tuple( content ),
          std::make_tuple( http::status::ok, request.version() )
        };
        response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        response.set(http::field::content_type, mime_type( path ));
        response.content_length( content.size() );
        response.keep_alive( request.keep_alive() );
        return response;
      }
      else {
        return not_found( request, request.target() );
      }
    }
    else {
      // Handle an unknown error
      return server_error( request, ec.message() );
    }
  }

  // Cache the size since we need it after the move
  auto const size = body.size();

  // Respond to HEAD request
  if ( request.method() == http::verb::head ) {
    http::response<http::empty_body> response{ http::status::ok, request.version() };
    response.set( http::field::server, BOOST_BEAST_VERSION_STRING );
    response.set( http::field::content_type, mime_type( path ) );
    response.content_length( size );
    response.keep_alive( request.keep_alive() );
    return response;
  }

  // Respond to GET request
  http::response<http::file_body> response{
    std::piecewise_construct,
    std::make_tuple( std::move( body ) ),
    std::make_tuple( http::status::ok, request.version() )
  };

  response.set( http::field::server, BOOST_BEAST_VERSION_STRING );
  response.set( http::field::content_type, mime_type( path ) );
  response.content_length( size );
  response.keep_alive( request.keep_alive() );
  return response;
}

template<typename Stream>
net::awaitable<void, executor_type>
run_websocket_session(
  Stream& stream,
  beast::flat_buffer& buffer,
  http::request<http::string_body> req
)
{

  auto cs = co_await net::this_coro::cancellation_state;
  auto ws = websocket::stream<Stream&>{ stream };

  // Set suggested timeout settings for the websocket
  ws.set_option(
    websocket::stream_base::timeout::suggested(beast::role_type::server));

  // Set a decorator to change the Server of the handshake
  ws.set_option(
    websocket::stream_base::decorator(
      [](websocket::response_type& res) {
        res.set(
          http::field::server,
          std::string(BOOST_BEAST_VERSION_STRING) +
              " advanced-server-flex");
      }
    )
  );

  // Accept the websocket handshake
  co_await ws.async_accept(req);

  while ( !cs.cancelled() ) {
    // Read a message
    auto [ec, _] = co_await ws.async_read(buffer, net::as_tuple);

    if( ( ec == websocket::error::closed ) || ( ec == ssl::error::stream_truncated ) )
      co_return;

    if(ec)
      throw boost::system::system_error{ ec };

    // Echo the message back
    ws.text(ws.got_text());
    co_await ws.async_write(buffer.data());

    // Clear the buffer
    buffer.consume( buffer.size() );
  }

  // A cancellation has been requested, gracefully close the session.
  auto [ec] = co_await ws.async_close(
    websocket::close_code::service_restart, net::as_tuple);

  if( ec && ( ec != ssl::error::stream_truncated ) )
    throw boost::system::system_error{ ec };
}

template<typename Stream>
net::awaitable<void, executor_type>
run_session(
  Stream& stream,
  beast::flat_buffer& buffer,
  state_t state
)
{
  auto cs = co_await net::this_coro::cancellation_state;

  while ( !cs.cancelled() ) {

    http::request_parser<http::string_body> parser;
    parser.body_limit(10000);

    auto [ec, _] =
      co_await http::async_read( stream, buffer, parser, net::as_tuple );

    if ( http::error::end_of_stream == ec )
      co_return;

    if ( websocket::is_upgrade(parser.get())) {

      // The websocket::stream uses its own timeout settings.
      beast::get_lowest_layer(stream).expires_never();

      co_await run_websocket_session(
        stream, buffer, parser.release());

      co_return;
    }

    BOOST_LOG_TRIVIAL(trace) << "----------";

    auto request( parser.release() );

    if ( http::verb::unknown == request.method() ) {
      BOOST_LOG_TRIVIAL(trace) << "request: '" << request << "'";
      BOOST_LOG_TRIVIAL(info)
        << state.endpoint.address() << ':' << state.endpoint.port() << " unknown request"; // probably ssl session timeout
      co_return;
    }
    else {
      //bool u( false );
      for ( auto b = request.begin(); b != request.end(); b++ ) {
        if ( beast::http::field::unknown == b->name() ) {
        BOOST_LOG_TRIVIAL(trace) << "fieldu: " << b->name_string() << '=' << b->value();
        // example:
        // fieldu: x-openai-host-hash=53160607
        // field: From=oai-searchbot(at)openai.com
        }
        else {
          BOOST_LOG_TRIVIAL(trace) << "field: " << b->name() << '=' << b->value();
        }
        // NOTE: will need to find the enum / name lookup table
      }

      BOOST_LOG_TRIVIAL(trace)
        << "body: '" << request.body() << "'";

      //if ( u ) { // try to determine unknown fields
      //  BOOST_LOG_TRIVIAL(trace)
      //    << "request: '" << request << "'";
      //}

      auto response = handle_request( state, std::move( request ) );

      if ( !response.keep_alive() ) {
        co_await beast::async_write( stream, std::move( response ) );
        co_return;
      }

      co_await beast::async_write( stream, std::move( response ) );
    }
  }
}

net::awaitable<void, executor_type>
detect_session(
  stream_type stream,
  ssl::context& ctx,
  beast::string_view doc_root
)
{

  beast::flat_buffer buffer;

  // Allow total cancellation to change the cancellation state of this
  // coroutine, but only allow terminal cancellation to propagate to async
  // operations. This setting will be inherited by all child coroutines.
  co_await net::this_coro::reset_cancellation_state(
    net::enable_total_cancellation(), net::enable_terminal_cancellation()
  );

  // We want to be able to continue performing new async operations, such as
  // cleanups, even after the coroutine is cancelled. This setting will be
  // inherited by all child coroutines.
  co_await net::this_coro::throw_if_cancelled( false );

  stream.expires_after(std::chrono::seconds(30));

  state_t state( doc_root, stream.socket().remote_endpoint() );

  if( co_await beast::async_detect_ssl( stream, buffer ) ) {

    ssl::stream<stream_type> ssl_stream{ std::move( stream ), ctx };

    auto bytes_transferred = co_await ssl_stream.async_handshake(
      ssl::stream_base::server, buffer.data()
    );

    buffer.consume( bytes_transferred );

    { // attempt ssl server name query
      auto handle = ssl_stream.native_handle();   // ssl_st
      state.ssl_name = SSL_get_servername( handle, TLSEXT_NAMETYPE_host_name );
    }

    co_await run_session( ssl_stream, buffer, state );

    if( !ssl_stream.lowest_layer().is_open() )
      //BOOST_LOG_TRIVIAL(info) << "ssl session closed (1)";
      co_return;

    // Gracefully close the stream
    auto [ec] = co_await ssl_stream.async_shutdown(net::as_tuple);
    if( ec && ( ec != ssl::error::stream_truncated ) ) {
      throw boost::system::system_error{ ec };
    }

    //BOOST_LOG_TRIVIAL(info) << "ssl session closed (2)";
  }
  else {

    state.bSsl = false;
    co_await run_session( stream, buffer, state );

    if( !stream.socket().is_open() )
      co_return;

    stream.socket().shutdown( net::ip::tcp::socket::shutdown_send );
  }
}

net::awaitable<void, executor_type>
listen(
  task_group& task_group,
  ssl::context& ctx,
  net::ip::tcp::endpoint endpoint,
  beast::string_view doc_root
)
{
  // BOOST_LOG_TRIVIAL(info) << "starting listen" << std::endl; // only a 1 count

  auto cs       = co_await net::this_coro::cancellation_state;
  auto executor = co_await net::this_coro::executor;
  auto acceptor = acceptor_type{ executor, endpoint };

  // Allow total cancellation to propagate to async operations.
  co_await net::this_coro::reset_cancellation_state(
    net::enable_total_cancellation()
  );

  while( !cs.cancelled() ) {

    auto socket_executor = net::make_strand(executor.get_inner_executor() );
    auto [ ec, socket ] =
      co_await acceptor.async_accept( socket_executor, net::as_tuple );

    if( ec == net::error::operation_aborted )
      co_return;

    if( ec )
      throw boost::system::system_error{ ec };

    net::co_spawn(
      std::move( socket_executor ),
      detect_session( stream_type{ std::move( socket ) }, ctx, doc_root ),
      task_group.adapt(
        []( std::exception_ptr e ) {
          if ( e ) {
            try {
              std::rethrow_exception(e);
            }
            catch( boost::system::system_error& ec ) {
              static const std::string s1( "The socket was closed due to a timeout" );
              const std::string& s2( ec.what() );
              const auto result = s1.compare( 0, s1.size(), s2, 0, s1.size() );
              if ( 0 == result ) {}
              else {
                BOOST_LOG_TRIVIAL(error) << "listen (1) " << ec.code() << ": " << ec.what();
              }
            }
            catch( std::exception& e ) {
              BOOST_LOG_TRIVIAL(error) << "listen (2): " << e.what();
            }
          }
        }
      )
    );
  }
}

