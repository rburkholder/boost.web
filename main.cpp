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

//#include <iostream>

//#include "Server.hpp"

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

#include "handle_signals.hpp"
#include "listen.hpp"
#include "server_certificate.hpp"
#include "task_group.hpp"

namespace net       = boost::asio;
namespace ssl       = boost::asio::ssl;

using executor_type = net::strand<net::io_context::executor_type>;

int
main( int argc, char* argv[] )
{

  static const std::string c_sConfigFilename( "web.boost.cfg" );

  std::cout << "(c)2026 One Unified Net Limited" << std::endl;

  //config::Values choices;

  //if ( Load( c_sConfigFilename, choices ) ) {
  //}
  //else {
  //  return EXIT_FAILURE;
  //}

  //try {
  //  Server server;
  //}
  //catch(...) {
  //  return EXIT_FAILURE;
  //}

  // Check command line arguments.
  if(argc != 5) {
    std::cerr << "Usage: boost.web <address> <port> <doc_root> <threads>\n"
              << "Example:\n"
              << "    boost.web 0.0.0.0 8080 . 1\n";
    return EXIT_FAILURE;
  }
  auto const address  = net::ip::make_address( argv[ 1 ] );
  auto const port     = static_cast<unsigned short>( std::atoi(argv[2]) );
  auto const endpoint = net::ip::tcp::endpoint{ address, port };
  auto const doc_root = beast::string_view{ argv[3] };
  auto const threads  = std::max<int>( 1, std::atoi( argv[4] ) );

  // The io_context is required for all I/O
  net::io_context ioc{ threads };

  // The SSL context is required, and holds certificates
  ssl::context ctx{ ssl::context::tlsv12 };

  // This holds the self-signed certificate used by the server
  load_server_certificate( ctx );

  // Track coroutines
  task_group task_group{ ioc.get_executor() };

  // Create and launch a listening coroutine
  net::co_spawn(
    net::make_strand(ioc),
    listen( task_group, ctx, endpoint, doc_root ),
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
  vThread.reserve( threads - 1 );
  for( auto i = threads - 1; i > 0; --i ) {
    vThread.emplace_back( [&ioc] { ioc.run(); } );
  }

  ioc.run();

  // Block until all the threads exit
  for ( auto& thread : vThread )
    thread.join();

  return EXIT_SUCCESS;
}

