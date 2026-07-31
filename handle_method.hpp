#pragma once

#include <functional>

// each needs to be thread safe, maintain state in a different structure

using fMethodHead_t = std::function<void()>;
using fMethodGet_t = std::function<void()>;
using fMethodPost_t = std::function<void()>;

using fServerError_t = std::function<void()>;
using fNotFound_t = std::function<void()>;
using fBadRequest_t = std::function<void()>;