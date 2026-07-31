#pragma once

#include <functional>

#include <boost/beast.hpp>
//#include <boost/beast/http/message.hpp>
//#include <boost/beast/http/message_fwd.hpp>
//#include <boost/beast/http/string_body_fwd.hpp>
//#include <boost/beast/core/string_type.hpp>

// each needs to be thread safe, maintain state in a different structure

namespace http = boost::beast::http;

using response_t = http::response<http::string_body>;

using fMethodHead_t = std::function<void()>;
using fMethodGet_t = std::function<void()>;
using fMethodPost_t = std::function<void()>;

using fResponse_t = std::function<void( response_t& )>;

using fResponseSv_t = std::function<void( response_t&, const boost::beast::string_view )>;

void bad_request(  response_t&, const boost::beast::string_view why );
void not_found(    response_t&, const boost::beast::string_view target );
void server_error( response_t&, const boost::beast::string_view what );

void robots_txt( response_t& );
