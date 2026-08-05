cmake_minimum_required (VERSION 3.14)

PROJECT(boost.web)

if (POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

# https://cmake.org/cmake/help/v3.6/module/FindBoost.html
#set(Boost_ARCHITECTURE "-x64")
#set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#set(Boost_DEBUG 1)
#set(Boost_REALPATH ON)
#set(Boost_DETAILED_FAILURE_MSG ON)

#find_package(Boost 1.81.0 COMPONENTS program_options regex thread filesystem log log_setup json REQUIRED )
find_package(Boost REQUIRED COMPONENTS log url program_options)

set(
  file_hpp
    config.hpp
    handle_methods.hpp
    handle_signals.hpp
    KeyWordMatch.hpp
    listen.hpp
    mime_type.hpp
    server_certificate.hpp
    task_group.hpp
  )

set(
  file_cpp
    config.cpp
    handle_methods.cpp
    handle_signals.cpp
    listen.cpp
    main.cpp
    mime_type.cpp
    server_certificate.cpp
  )

add_executable(
  ${PROJECT_NAME}
    ${file_hpp}
    ${file_cpp}
  )

target_compile_definitions( ${PROJECT_NAME} PUBLIC BOOST_LOG_DYN_LINK )

target_compile_definitions( ${PROJECT_NAME} PUBLIC SOL_LUAJIT=1 )
target_compile_definitions( ${PROJECT_NAME} PUBLIC SOL_ALL_SAFETIES_ON=1 )

target_include_directories(
  ${PROJECT_NAME} PUBLIC
    /usr/local/include/luajit-2.1
  )

target_link_directories(
  ${PROJECT_NAME} PUBLIC
    /usr/local/lib
  )

target_link_libraries(
  ${PROJECT_NAME}
    pthread
    ${Boost_LIBRARIES}
    luajit-5.1
    crypto
    ssl
  )