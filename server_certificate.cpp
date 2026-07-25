//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#include <cstddef>

//#include <iostream>

#include <openssl/tls1.h>

#include <boost/asio/buffer.hpp>

#include "server_certificate.hpp"
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

ssl_ctx_st* phandle;

static int ssl_callback_ctrl( SSL* ssl, int* ad, void* arg ) {

  if ( ssl == nullptr ) {
    return SSL_TLSEXT_ERR_NOACK;
  }

  const char* servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
  //ASSERT(servername && servername[0]);
  if ( !servername || ( '\0' == servername[0] ) )
    return SSL_TLSEXT_ERR_NOACK;

  //std::cout << "  ssl callback: " << name << std::endl;
  return SSL_TLSEXT_ERR_OK;
}

void
load_server_certificate( boost::asio::ssl::context& ctx ) {

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
