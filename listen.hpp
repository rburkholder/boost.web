#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast.hpp>

#include "task_group.hpp"
#include "handle_methods.hpp"

namespace beast     = boost::beast;
namespace net       = boost::asio;
namespace ssl       = boost::asio::ssl;

using executor_type = net::strand<net::io_context::executor_type>;
using acceptor_type = typename net::ip::tcp::acceptor::rebind_executor<executor_type>::other;
using stream_type   = typename beast::tcp_stream::rebind_executor<executor_type>::other;

struct method_handlers {

  fResponseSv_t fNotFound;
  fResponseSv_t fBadRequest;
  fResponseSv_t fServerError;

  fResponse_t fRobotsTxt;

  fMethodHead_t fMethodHead;
  fMethodGet_t fMethodGet;
  fMethodPost_t fMethodPost;
};

net::awaitable<void, executor_type>
listen(
  task_group& task_group,
  ssl::context& ctx,
  net::ip::tcp::endpoint endpoint,
  method_handlers& handlers,
  beast::string_view doc_root
);
