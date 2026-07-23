#include <iostream>

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

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template<class Body, class Allocator>
http::message_generator
handle_request(
  beast::string_view doc_root,
  http::request<Body, http::basic_fields<Allocator>>&& req
)
{
    // Returns a bad request response
    auto const bad_request =
    [&req](beast::string_view why)
    {
      http::response<http::string_body> res{http::status::bad_request, req.version()};
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "text/html");
      res.keep_alive(req.keep_alive());
      res.body() = std::string(why);
      res.prepare_payload();
      return res;
    };

    // Returns a not found response
    auto const not_found =
    [&req](beast::string_view target)
    {
      http::response<http::string_body> res{http::status::not_found, req.version()};
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "text/html");
      res.keep_alive(req.keep_alive());
      res.body() = "The resource '" + std::string(target) + "' was not found.";
      res.prepare_payload();
      return res;
    };

    // Returns a server error response
    auto const server_error =
    [&req](beast::string_view what)
    {
      http::response<http::string_body> res{http::status::internal_server_error, req.version()};
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, "text/html");
      res.keep_alive(req.keep_alive());
      res.body() = "An error occurred: '" + std::string(what) + "'";
      res.prepare_payload();
      return res;
    };

    // Make sure we can handle the method
    if( req.method() != http::verb::get &&
        req.method() != http::verb::head
      )
      return bad_request("Unknown HTTP-method");

    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos
      )
      return bad_request("Illegal request-target");

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if(req.target().back() == '/')
        path.append("index.html");

    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open( path.c_str(), beast::file_mode::scan, ec );

    // Handle the case where the file doesn't exist
    if( ec == beast::errc::no_such_file_or_directory )
      return not_found(req.target());

    // Handle an unknown error
    if( ec )
      return server_error(ec.message());

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if(req.method() == http::verb::head) {
      http::response<http::empty_body> res{http::status::ok, req.version()};
      res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(http::field::content_type, mime_type(path));
      res.content_length(size);
      res.keep_alive(req.keep_alive());
      return res;
    }

    // Respond to GET request
    http::response<http::file_body> res{
      std::piecewise_construct,
      std::make_tuple(std::move(body)),
      std::make_tuple(http::status::ok, req.version())
    };

    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return res;
}

template<typename Stream>
net::awaitable<void, executor_type>
run_websocket_session(
    Stream& stream,
    beast::flat_buffer& buffer,
    http::request<http::string_body> req)
{
    auto cs = co_await net::this_coro::cancellation_state;
    auto ws = websocket::stream<Stream&>{ stream };

    // Set suggested timeout settings for the websocket
    ws.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws.set_option(websocket::stream_base::decorator(
        [](websocket::response_type& res)
        {
            res.set(
                http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " advanced-server-flex");
        }));

    // Accept the websocket handshake
    co_await ws.async_accept(req);

    while(!cs.cancelled())
    {
        // Read a message
        auto [ec, _] = co_await ws.async_read(buffer, net::as_tuple);

        if(ec == websocket::error::closed || ec == ssl::error::stream_truncated)
            co_return;

        if(ec)
            throw boost::system::system_error{ ec };

        // Echo the message back
        ws.text(ws.got_text());
        co_await ws.async_write(buffer.data());

        // Clear the buffer
        buffer.consume(buffer.size());
    }

    // A cancellation has been requested, gracefully close the session.
    auto [ec] = co_await ws.async_close(
        websocket::close_code::service_restart, net::as_tuple);

    if(ec && ec != ssl::error::stream_truncated)
        throw boost::system::system_error{ ec };
}

template<typename Stream>
net::awaitable<void, executor_type>
run_session(
  Stream& stream,
  beast::flat_buffer& buffer,
  beast::string_view doc_root
)
{
  auto cs = co_await net::this_coro::cancellation_state;

  while(!cs.cancelled() ) {

    http::request_parser<http::string_body> parser;
    parser.body_limit(10000);

    auto [ec, _] =
      co_await http::async_read(stream, buffer, parser, net::as_tuple);

    if(ec == http::error::end_of_stream)
      co_return;

    if( websocket::is_upgrade(parser.get())) {

      // The websocket::stream uses its own timeout settings.
      beast::get_lowest_layer(stream).expires_never();

      co_await run_websocket_session(
        stream, buffer, parser.release());

      co_return;
    }

    auto res = handle_request(doc_root, parser.release());
    if(!res.keep_alive())
    {
        co_await beast::async_write(stream, std::move(res));
        co_return;
    }

    co_await beast::async_write(stream, std::move(res));
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
      net::enable_total_cancellation(), net::enable_terminal_cancellation());

  // We want to be able to continue performing new async operations, such as
  // cleanups, even after the coroutine is cancelled. This setting will be
  // inherited by all child coroutines.
  co_await net::this_coro::throw_if_cancelled(false);

  stream.expires_after(std::chrono::seconds(30));

  if(co_await beast::async_detect_ssl(stream, buffer))
  {
    ssl::stream<stream_type> ssl_stream{ std::move(stream), ctx };

    auto bytes_transferred = co_await ssl_stream.async_handshake(
        ssl::stream_base::server, buffer.data());

    buffer.consume(bytes_transferred);

    co_await run_session(ssl_stream, buffer, doc_root);

    if(!ssl_stream.lowest_layer().is_open())
        co_return;

    // Gracefully close the stream
    auto [ec] = co_await ssl_stream.async_shutdown(net::as_tuple);
    if(ec && ec != ssl::error::stream_truncated)
        throw boost::system::system_error{ ec };
  }
  else
  {
    co_await run_session(stream, buffer, doc_root);

    if(!stream.socket().is_open())
        co_return;

    stream.socket().shutdown(net::ip::tcp::socket::shutdown_send);
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
  auto cs       = co_await net::this_coro::cancellation_state;
  auto executor = co_await net::this_coro::executor;
  auto acceptor = acceptor_type{ executor, endpoint };

  // Allow total cancellation to propagate to async operations.
  co_await net::this_coro::reset_cancellation_state(
      net::enable_total_cancellation());

  while(!cs.cancelled()) {

    auto socket_executor = net::make_strand(executor.get_inner_executor());
    auto [ec, socket] =
        co_await acceptor.async_accept(socket_executor, net::as_tuple);

    if(ec == net::error::operation_aborted)
        co_return;

    if(ec)
        throw boost::system::system_error{ ec };

    net::co_spawn(
      std::move(socket_executor),
      detect_session( stream_type{ std::move(socket) }, ctx, doc_root),
      task_group.adapt(
        []( std::exception_ptr e ) {
          if ( e ) {
            try {
              std::rethrow_exception(e);
            }
            catch( std::exception& e ) {
              static const std::string s1( "The socket was closed due to a timeout" );
              const std::string s2( e.what() );
              const auto result = s1.compare( 0, s1.size(), s2, 0, s1.size() );
              if ( 0 == result ) {}
              else {
                std::cerr << "Error in session: " << e.what() << "\n";
              }
            }
          }
        }
      )
    );
  }
}

