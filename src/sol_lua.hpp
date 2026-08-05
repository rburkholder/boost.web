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
 * File:    sol_lua.hpp
 * Author:  raymond@burkholder.net
 * Project: boost.web
 * Created: August 5, 2026 15:47:57
 */

#pragma once

#include <sol/sol.hpp>

class sol_lua_t {
public:
  sol_lua_t();
  ~sol_lua_t();
protected:
private:
  sol::state sol_lua;
};