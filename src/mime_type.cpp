#include "mime_type.hpp"

mime_type::mime_type()
: m_kwmMimeType( "application/text" , 25 )
{
  m_kwmMimeType.AddPattern( "htm",  "text/html" );
  m_kwmMimeType.AddPattern( "html", "text/html" );
  m_kwmMimeType.AddPattern( "php",  "text/html" );
  m_kwmMimeType.AddPattern( "css",  "text/css" );
  m_kwmMimeType.AddPattern( "txt",  "text/plain" );
  m_kwmMimeType.AddPattern( "text", "text/plain" );
  m_kwmMimeType.AddPattern( "js",   "application/javascript" );
  m_kwmMimeType.AddPattern( "json", "application/json" );
  m_kwmMimeType.AddPattern( "xml",  "application/xml" );
  m_kwmMimeType.AddPattern( "swf",  "application/x-shockwave-flash" );
  m_kwmMimeType.AddPattern( "flv",  "video/x-flv" );
  m_kwmMimeType.AddPattern( "png",  "image/png" );
  m_kwmMimeType.AddPattern( "jpe",  "image/jpeg" );
  m_kwmMimeType.AddPattern( "jpeg", "image/jpeg" );
  m_kwmMimeType.AddPattern( "jpg",  "image/jpeg" );
  m_kwmMimeType.AddPattern( "gif",  "image/gif" );
  m_kwmMimeType.AddPattern( "bmp",  "image/bmp" );
  m_kwmMimeType.AddPattern( "ico",  "image/vnd.microsoft.icon" );
  m_kwmMimeType.AddPattern( "tiff", "image/tiff" );
  m_kwmMimeType.AddPattern( "tif",  "image/tiff" );
  m_kwmMimeType.AddPattern( "svg",  "image/svg+xml" );
  m_kwmMimeType.AddPattern( "svgz", "image/svg+xml" );
}

mime_type::~mime_type() {
}

const beast::string_view mime_type::lu( const beast::string_view path ) const {

  auto const ext = [&path] {
    auto const pos = path.rfind( '.' );
    if( pos == beast::string_view::npos )
      return beast::string_view{};
    if ( pos == ( path.size() - 1 ) ) {
      return beast::string_view();
    }
    return path.substr( pos + 1 );
  }();

  return m_kwmMimeType.FindMatch( ext );

}

