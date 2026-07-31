#include <string>

//#include <boost/beast/http/field.hpp>
//#include <boost/beast/http/status.hpp>
//#include <boost/beast/http/message_fwd.hpp>
//#include <boost/beast/http/string_body_fwd.hpp>

#include "handle_methods.hpp"

namespace {
  static const std::string c_sVersion( "ounl web server with lua v1.0");
}

void bad_request( response_t& response, const boost::beast::string_view why ) {
  response.set( http::field::server, c_sVersion);
  response.set( http::field::content_type, "text/html");
  response.body() = std::string( why );
}

void not_found( response_t& response, const boost::beast::string_view target ) {
  response.set( http::field::server, c_sVersion);
  response.set( http::field::content_type, "text/html");
  response.body() = "The resource '" + std::string(target) + "' was not found.";
}

void server_error( response_t& response, const boost::beast::string_view what ) {
  response.set( http::field::server, c_sVersion);
  response.set( http::field::content_type, "text/html");
  response.body() = "An error occurred: '" + std::string(what) + "'";
}
