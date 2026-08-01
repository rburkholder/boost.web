#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast.hpp>

#include "task_group.hpp"

namespace beast     = boost::beast;
namespace net       = boost::asio;
namespace ssl       = boost::asio::ssl;

using executor_type = net::strand<net::io_context::executor_type>;
using acceptor_type = typename net::ip::tcp::acceptor::rebind_executor<executor_type>::other;
using stream_type   = typename beast::tcp_stream::rebind_executor<executor_type>::other;

net::awaitable<void, executor_type>
listen(
  task_group& task_group,
  ssl::context& ctx,
  net::ip::tcp::endpoint endpoint,
  beast::string_view doc_root
);
