FROM rikorose/gcc-cmake:gcc-7
RUN apt-get -y install build-essential git cmake pkg-config libgtk2.0-dev
