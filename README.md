# web.boost

## inttroduction

* Use Boost Beast Advanced server, flex (plain + SSL) as template for handling web operations.
* Currently developed on Debian Linux platform

## sample configuration file (web.boost.cfg)
```
thread_count = 2
port_http = 80
port_https = 443
listen_address = 0.0.0.0
content_directory = web/content
static_host = static.example.com
static_directory = web/static
static_extension = jpg
static_extension = jpeg
static_extension = png
static_extension = css
static_extension = txt
static_extension = ico
static_extension = gif
certificate_path_fullchain = certs/fullchain.pem
certificate_path_privkey = certs/privkey.pem
```
## build outline
* compile environment: C++20
* requires libssl-dev, libboost-dev (json, log, program_options, serialization, url)
* add luajit and sol2

### build lua modules: luajit, sol2
```
# acquire, build & install luajit
git clone --depth=1 https://github.com/LuaJIT/LuaJIT.git
pushd LuaJIT
sed -i 's/#XCFLAGS+= -DLUAJIT_ENABLE_LUA52COMPAT/XCFLAGS+= -DLUAJIT_ENABLE_LUA52COMPAT/' src/Makefile
make
sudo make install
popd

# acquire & install sol2 (lua helpers for c++)
git clone --depth=1 https://github.com/ThePhD/sol2.git
sudo mv -n sol2/include/sol /usr/local/include/
```

### build project
```
mkdir build
cd build
cmake ..
make
```

## security, run
To run on a port under 1024, requires something like:
```
sudo setcap CAP_NET_BIND_SERVICE=+eip ~/projects/web.boost/build/src/boost.web
~/projects/web.boost/build/src/boost.web
```

## current features
* 2026/08/02
  * GET static html and support files from a directory
  * supports HTTP and HTTPS

## alternatives
* [wt web toolkit](https://www.webtoolkit.eu/wt/) - designed for single page applications with total control over page generation
* [drogon web framework](https://drogon.org/) - excellent functionality but comes up short on SSL capability and reliability
* [openresty](https://github.com/openresty/openresty) - Web Platform Based on Nginx and LuaJIT - may come back to this
