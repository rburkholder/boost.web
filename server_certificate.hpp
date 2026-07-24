//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_EXAMPLE_COMMON_SERVER_CERTIFICATE_HPP
#define BOOST_BEAST_EXAMPLE_COMMON_SERVER_CERTIFICATE_HPP

#include <cstddef>

//#include <iostream>

#include <openssl/tls1.h>

#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl/context.hpp>

//#include <memory>
ssl_ctx_st* phandle;
void ssl_callback_ctrl( SSL *ssl, int *ad, void *arg ) {
  const char* name = SSL_get_servername( ssl, TLSEXT_NAMETYPE_host_name );
  //std::cout << "  ssl callback: " << name << std::endl;
}

/*  Load a signed certificate into the ssl context, and configure
    the context for use with a server.

    For this to work with the browser or operating system, it is
    necessary to import the "Beast Test CA" certificate into
    the local certificate store, browser, or operating system
    depending on your environment Please see the documentation
    accompanying the Beast certificate for more details.
*/
inline
void
load_server_certificate( boost::asio::ssl::context& ctx ) {
  /*
      The certificate was generated from bash on Ubuntu (OpenSSL 1.1.1f) using:

      openssl dhparam -out dh.pem 2048
      openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 10000 -out cert.pem -subj "/C=US/ST=CA/L=Los Angeles/O=Beast/CN=www.example.com"
  */

  std::string const dh =
    "-----BEGIN DH PARAMETERS-----\n"
    "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
    "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
    "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
    "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
    "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
    "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
      "-----END DH PARAMETERS-----\n";

  ctx.set_password_callback(
    [](std::size_t,
        boost::asio::ssl::context_base::password_purpose)
    {
        return "test";
    });

  phandle = ctx.native_handle();
  SSL_CTX_set_tlsext_servername_callback( phandle, &ssl_callback_ctrl );

  ctx.set_options(
    boost::asio::ssl::context::default_workarounds |
    boost::asio::ssl::context::no_sslv2 |
    boost::asio::ssl::context::single_dh_use);

  ctx.use_certificate_chain_file (
    "certs/fullchain.pem"
  );

  ctx.use_private_key_file(
    "certs/privkey.pem",
    boost::asio::ssl::context::file_format::pem
  );

  ctx.use_tmp_dh(
    boost::asio::buffer(dh.data(), dh.size())
  );
}

#endif
