# libcatner

- A thin wrapper on top of `libxml2`, to suit a very specific purpose
- Pretty early work in progress, you might want to check back later

## Dependencies

- `libxml2`

## Build

One of these provided scripts should get the job done, accordingly:

    ./build-shared
    ./build-static

## Install

On Debian, this is how I install the shared library after compilation:

    cp ./lib/libcatner.h /usr/include/
    cp ./lib/libcatner.so /usr/lib/x86_64-linux-gnu/
    ldconfig -v -n /usr/lib

## License

I haven't made up my mind yet. Contact me if you want to use this commercially and we'll figure something out.
