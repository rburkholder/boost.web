/************************************************************************
 * Copyright(c) 2026, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

/*
 * File:    handle_methods.cpp
 * Author:  raymond@burkholder.net
 * Project: boost.web
 * Created: July 22, 2026 18:36
 */

#include <string>

#include "handle_methods.hpp"

namespace {
  static const std::string c_sVersion( "ounl web server with lua v1.0");
}

void response_bad_request( response_t& response, const boost::beast::string_view why ) {
  response.set( http::field::server, c_sVersion );
  response.set( http::field::content_type, "text/plain");
  response.body() = std::string( why );
}

void response_not_found( response_t& response, const boost::beast::string_view target ) {
  response.set( http::field::server, c_sVersion );
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
  response.set( http::field::server, c_sVersion );
  //response.set( http::field::content_type, "text/html");
  response.body() = content;
  response.content_length( content.size() );
}

void method_head( http::response<http::empty_body>& response ) {
  response.set( http::field::server, c_sVersion);
}

void method_get( http::response<http::file_body>& response ) {
  response.set( http::field::server, c_sVersion);
}

void method_post( response_t& response ) {
  response.set( http::field::server, c_sVersion);
  response.body() = "your post was accepted\n";
}

void lua_method_get( std::string& path, sol_lua_t& sol_lua, http::response<http::string_body>& response ) {
  response.set( http::field::server, c_sVersion );
  sol_lua.m_sol[ "mime_type" ] = "text/html";
  sol_lua.m_sol[ "content" ] = "";
  sol::load_result script = sol_lua.m_sol.load_file( path, sol::load_mode::text );
  if ( script.valid() ) {
    auto result = script();
    response.body() = sol_lua.m_sol[ "content" ];
    response.set( http::field::content_type, std::string( sol_lua.m_sol[ "mime_type" ] ) );
  }
  else {
    response.result( http::status::not_found );
    response.set( http::field::content_type, "text/plain");
    response.body() = "The resource '" + path + "' was not found.\n";
  }
}

void lua_method_post( std::string& path, sol_lua_t& sol_lua, http::response<http::string_body>& response ) {
}
