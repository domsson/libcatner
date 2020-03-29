# libcatner

A wrapper on top of `libxml2` that makes it easy to create, load, edit and 
write XML files as required by the kloeckner online shop's product importer. 

kloeckner requires product files to use the BMEcat XML format, but only a 
subset of the format's features are being used. `libcatner` specifically 
supports the kloeckner flavor of BMEcat files. In other words, if you're 
looking for a general purpose, fully featured BMEcat library, this ain't it. 

If, however, you want to write a tool that can deal with BMEcat files as 
required by the kloeckner online shop, then this library might come in handy. 

Note, however, that this project is currently beign developed on and for 
Linux. I assume it could be compiled on Windows just fine, but I have no idea 
as to how one compiles C code on Windows. Feel free to report your attempts.

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

I haven't made up my mind yet. Contact me if you want to use this commercially 
and I'm sure we can figure something out.

Also note that I've already written a CLI tool that can convert simple CSV 
files with product information into BMEcat files, ready for import. It sits 
in a private repository, but I wouldn't mind to open source it if someone 
wanted to sponsor the development time that went into it... :-)
