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
 * File:    main.cpp
 * Author:  raymond@burkholder.net
 * Project: boost.web
 * Created: July 20, 2026 18:36
 */

//------------------------------------------------------------------------------
//
// Example: Advanced server, flex (plain + SSL)
//
//------------------------------------------------------------------------------

#include <iostream>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast.hpp>

#include "config.hpp"
#include "listen.hpp"
#include "task_group.hpp"
#include "handle_signals.hpp"
#include "server_certificate.hpp"

namespace net       = boost::asio;
namespace ssl       = boost::asio::ssl;

using executor_type = net::strand<net::io_context::executor_type>;

int main( int argc, char* argv[] ) {

  std::string sConfigFilename( "web.boost.cfg" );

  std::cout << "(c)2026 One Unified Net Limited" << std::endl;

  // Check command line arguments.
  if( 2 == argc ) {
    sConfigFilename = argv[ 1 ];
  }

  config::Values choices;

  if ( Load( sConfigFilename, choices ) ) {}
  else {
    return EXIT_FAILURE;
  }

  auto const address  = net::ip::make_address( choices.sListenAddress );
  auto const endpoint = net::ip::tcp::endpoint{ address, choices.nPortHttps };

  // The io_context is required for all I/O
  net::io_context ioc{ choices.nThreads };

  // The SSL context is required, and holds certificates
  ssl::context ctx{ ssl::context::tlsv12 };

  // This holds the self-signed certificate used by the server
  load_server_certificate( ctx, choices.sCertificatePathFullChain, choices.sCertfificatePathPrivKey );

  // Track coroutines
  task_group task_group{ ioc.get_executor() };

  // Create and launch a listening coroutine
  net::co_spawn(
    net::make_strand(ioc),
    listen( task_group, ctx, endpoint, choices ),
    task_group.adapt(
      []( std::exception_ptr e ) {
        if( e ) {
          try {
              std::rethrow_exception(e);
          }
          catch ( std::exception& e ) {
            std::cerr << "Error in listener: " << e.what() << "\n";
          }
        }
      })
    );

  // Create and launch a signal handler coroutine
  net::co_spawn(
    net::make_strand( ioc ),
    handle_signals(task_group), net::detached
  );

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> vThread;
  vThread.reserve( choices.nThreads - 1 );
  for( auto i = choices.nThreads - 1; i > 0; --i ) {
    vThread.emplace_back( [&ioc] { ioc.run(); } );
  }

  ioc.run();

  // Block until all the threads exit
  for ( auto& thread : vThread )
    thread.join();

  return EXIT_SUCCESS;
}

