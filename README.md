# web.boost

Use Boost Beast Advanced server, flex (plain + SSL) as template for handling web operations.

Sample Cnfiguration File (web.boost.cfg)
```
thread_count = 2
static_host = static.example.com
static_directory = web/static
static_extension = jpg
static_extension = jpeg
static_extension = png
static_extension = css
static_extension = txt
static_extension = ico
static_extension = gif
content_directory = web/content
port_http = 80
port_https = 443
listen_address = 0.0.0.0
certificate_path_fullchain = certs/fullchain.pem
certificate_path_privkey = certs/privkey.pem
```

* compile environment: C++20
* requires libssl-dev, libboost-dev (json, log, program_options, serialization, url)

To run on a port under 1024, requires something like:
```
sudo setcap CAP_NET_BIND_SERVICE=+eip ~/projects/web.boost/build/boost.web
```