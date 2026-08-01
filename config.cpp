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
 * File:    config.cpp
 * Author:  raymond@burkholder.net
 * Project: web.boost
 * Created: 2026/08/01 09:52:42
 */

#include <fstream>
#include <exception>
//#include <type_traits>

#include <boost/log/trivial.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "config.hpp"

namespace {

  static const std::string sValue_thread_count( "thread_count" );

  static const std::string sValue_Static_Host(      "static_host" );
  static const std::string sValue_Static_Directory( "static_directory" );
  static const std::string sValue_Static_Extension( "static_extension" );

  static const std::string sValue_Content_Directory( "content_directory" );

  static const std::string sValue_Port_Http(  "port_http" );
  static const std::string sValue_Port_Https( "port_https" ); //

  static const std::string sValue_listen_address( "listen_address" );

  static const std::string sValue_Certificate_Path_FullChain( "certificate_path_fullchain" );
  static const std::string sValue_Certificate_Path_PrivKey(   "certificate_path_privkey" );

  //template<typename T>
  //void log( const std::string& name, typename std::enable_if<std::is_pod<T&>::value>::type& dest ) {
  //  BOOST_LOG_TRIVIAL(info) << name << " = " << dest;
  //}

  void log( const std::string& name, uint16_t dest ) {
    BOOST_LOG_TRIVIAL(info) << name << " = " << dest;
  }

  void log( const std::string& name, const std::string& dest ) {
    BOOST_LOG_TRIVIAL(info) << name << " = " << dest;
  }

  void log( const std::string& name, const config::Values::vStaticExtensions_t& dest ) {
    for ( const config::Values::vStaticExtensions_t::value_type& value: dest ) {
      BOOST_LOG_TRIVIAL(info) << name << " = " << value;
    }
  }

  template<typename T>
  bool parse( const std::string& sFileName, po::variables_map& vm, const std::string& name, T& dest ) {
    bool bOk = true;
    if ( 0 < vm.count( name ) ) {
      dest = vm[name].as<T>();
      //BOOST_LOG_TRIVIAL(info) << name << " = " << dest;
      log( name, dest );
    }
    else {
      BOOST_LOG_TRIVIAL(error) << sFileName << " missing '" << name << "='";
      bOk = false;
    }
    return bOk;
  }
}

namespace config {

bool Load( const std::string& sFileName, Values& values ) {

  bool bOk( true );

  try {

    po::options_description config( "web.boost config file" );
    config.add_options()

      ( sValue_thread_count.c_str(), po::value<uint16_t>( &values.nThreads)->default_value( 1 ), "number of servicing threads" )

      ( sValue_Static_Host.c_str(), po::value<std::string>( &values.sStaticHost ), "target host name" )
      ( sValue_Static_Directory.c_str(), po::value<std::string>( &values.sStaticDirectory )->default_value( "./" ), "directory for static content" )

      ( sValue_Static_Extension.c_str(), po::value<Values::vStaticExtensions_t>( &values.vStaticExtensions ), "permitted extensions for static content" )

      ( sValue_Content_Directory.c_str(), po::value<std::string>( &values.sContentDirectory ), "regular content directory" )

      ( sValue_Port_Http.c_str(), po::value<uint16_t>( &values.nPortHttp )->default_value( 80 ), "http port" )
      ( sValue_Port_Https.c_str(), po::value<uint16_t>( &values.nPortHttps )->default_value( 443 ), "https port" )

      ( sValue_listen_address.c_str(), po::value<std::string>( &values.sListenAddress )->default_value( "0.0.0.0" ), "listen address" )

      ( sValue_Certificate_Path_FullChain.c_str(), po::value<std::string>( &values.sCertificatePathFullChain ), "path for certificate full chain" )
      ( sValue_Certificate_Path_PrivKey.c_str(), po::value<std::string>( &values.sCertfificatePathPrivKey ), "path for certificate private key" )

      ;
    po::variables_map vm;
    //po::store( po::parse_command_line( argc, argv, config ), vm );

    std::ifstream ifs( sFileName.c_str() );

    if ( !ifs ) {
      BOOST_LOG_TRIVIAL(error) << "config file " << sFileName << " does not exist";
      bOk = false;
    }
    else {
      po::store( po::parse_config_file( ifs, config), vm );

      bOk &= parse<uint16_t>( sFileName, vm, sValue_thread_count, values.nThreads );
      if ( 0 < values.nThreads ) {}
      else {
        BOOST_LOG_TRIVIAL(error) << "thread count must be 1 or greater (" << values.nThreads << ')';
        bOk = false;
      }

      bOk &= parse<std::string>( sFileName, vm, sValue_Static_Host, values.sStaticHost );
      bOk &= parse<std::string>( sFileName, vm, sValue_Static_Directory, values.sStaticDirectory );
      bOk &= parse<Values::vStaticExtensions_t>( sFileName, vm, sValue_Static_Extension, values.vStaticExtensions );

      bOk &= parse<std::string>( sFileName, vm, sValue_Content_Directory, values.sContentDirectory );

      bOk &= parse<uint16_t>( sFileName, vm, sValue_Port_Http, values.nPortHttp );
      bOk &= parse<uint16_t>( sFileName, vm, sValue_Port_Https, values.nPortHttps );

      bOk &= parse<std::string>( sFileName, vm, sValue_listen_address, values.sListenAddress );

      bOk &= parse<std::string>( sFileName, vm, sValue_Certificate_Path_FullChain, values.sCertificatePathFullChain );
      bOk &= parse<std::string>( sFileName, vm, sValue_Certificate_Path_PrivKey, values.sCertfificatePathPrivKey );

    }
  }
  catch( std::exception& e ) {
    BOOST_LOG_TRIVIAL(error) << sFileName << " parse error: " << e.what();
    bOk = false;
  }

  return bOk;
}

} // namespace config