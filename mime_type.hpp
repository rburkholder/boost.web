#pragma once

#include <boost/beast.hpp>

#include "KeyWordMatch.hpp"

namespace beast = boost::beast;

class mime_type {
public:
  mime_type();
  ~mime_type();

  // Return a reasonable mime type based on the extension of a file.
  const beast::string_view lu( const beast::string_view ) const;
protected:
private:
  ou::KeyWordMatch<beast::string_view> m_kwmMimeType;
};
