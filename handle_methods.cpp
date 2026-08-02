#include <string>

//#include <boost/beast/http/field.hpp>
//#include <boost/beast/http/status.hpp>
//#include <boost/beast/http/message_fwd.hpp>
//#include <boost/beast/http/string_body_fwd.hpp>

#include "handle_methods.hpp"

namespace {
  static const std::string c_sVersion( "ounl web server with lua v1.0");
}

void response_bad_request( response_t& response, const boost::beast::string_view why ) {
  response.set( http::field::server, c_sVersion);
  response.set( http::field::content_type, "text/plain");
  response.body() = std::string( why );
}

void response_not_found( response_t& response, const boost::beast::string_view target ) {
  response.set( http::field::server, c_sVersion);
  response.set( http::field::content_type, "text/plain");
  response.body() = "The resource '" + std::string( target ) + "' was not found.\n";
}

void response_server_error( response_t& response, const boost::beast::string_view what ) {
  response.set( http::field::server, c_sVersion);
  response.set( http::field::content_type, "text/plain");
  response.body() = "An error occurred: '" + std::string( what ) + "'\n";
}

void resource_robots_txt( response_t& response ) {
  static const std::string content( "User-agent: *\nAllow: /\n" );
  response.set( http::field::server, c_sVersion);
  //response.set( http::field::content_type, "text/html");
  response.body() = content;
  response.content_length( content.size() );
}

void method_get( http::response<http::file_body>& response ) {
  response.set( http::field::server, c_sVersion);
}

void method_post( response_t& response ) {
  response.set( http::field::server, c_sVersion);
  response.body() = "your post was accepted\n";
}

void method_head( http::response<http::empty_body>& response ) {
  response.set( http::field::server, c_sVersion);
}