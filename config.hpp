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
 * File:    config.hpp
 * Author:  raymond@burkholder.net
 * Project: web.boost
 * Created: 2026/08/01 09:52:42
 */

#pragma once

#include <cstdint>

#include <string>
#include <vector>

namespace config {

struct Values {

  uint16_t nThreads;

  std::string sStaticHost;
  std::string sStaticDirectory; // generally pictures and such

  using vStaticExtensions_t = std::vector<std::string>;
  vStaticExtensions_t vStaticExtensions; // allowed file extensions

  std::string sContentDirectory; // static and dynamic files

  uint16_t nPortHttp;
  uint16_t nPortHttps;

  std::string sListenAddress;

  std::string sCertificatePathFullChain;
  std::string sCertfificatePathPrivKey;
};

bool Load( const std::string& sFileName, Values& );

} // namespace config