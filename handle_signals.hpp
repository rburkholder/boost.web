
#pragma once

#include <iostream>

#include <boost/asio.hpp>

#include <boost/beast.hpp>

#include "task_group.hpp"

namespace net       = boost::asio;

using executor_type = net::strand<net::io_context::executor_type>;

net::awaitable<void, executor_type>
handle_signals(task_group& task_group) {

  auto executor   = co_await net::this_coro::executor;
  auto signal_set = net::signal_set{ executor, SIGINT, SIGTERM };

  auto sig = co_await signal_set.async_wait();

  if(sig == SIGINT)
  {
    std::cout << "Gracefully cancelling child tasks...\n";
    task_group.emit(net::cancellation_type::total);

    // Wait a limited time for child tasks to gracefully cancell
    auto [ec] = co_await task_group.async_wait(
        net::as_tuple(net::cancel_after(std::chrono::seconds{ 10 })));

    if(ec == net::error::operation_aborted) { // Timeout occurred
      std::cout << "Sending a terminal cancellation signal...\n";
      task_group.emit(net::cancellation_type::terminal);
      co_await task_group.async_wait();
    }

    std::cout << "Child tasks completed.\n";
  }
  else // SIGTERM
  {
    net::query(
      executor.get_inner_executor(),
      net::execution::context).stop();
  }
}

