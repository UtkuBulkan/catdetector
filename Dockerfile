FROM rikorose/gcc-cmake:gcc-7
RUN apt-get -y install build-essential git cmake pkg-config libgtk2.0-dev

RUN sudo apt-get -y update
RUN sudo apt-get -y install build-essential git cmake pkg-config libgtk2.0-dev

RUN sudo apt-get -y purge wolfram-engine
RUN sudo apt-get -y purge libreoffice*
RUN sudo apt-get -y clean
RUN sudo apt-get -y autoremove

RUN sudo apt-get -y remove x264 libx264-dev
 
## Install dependencies
RUN sudo apt-get -y install build-essential checkinstall cmake pkg-config yasm
RUN sudo apt-get -y install git gfortran
RUN sudo apt-get -y install libjpeg8-dev libjasper-dev libpng12-dev
RUN sudo apt-get -y install libtiff5-dev
RUN sudo apt-get -y install libtiff-dev
RUN sudo apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libdc1394-22-dev
RUN sudo apt-get -y install libxine2-dev libv4l-dev

RUN sudo apt-get -y install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
RUN sudo apt-get -y install libgtk2.0-dev libtbb-dev qt5-default
RUN sudo apt-get -y install libatlas-base-dev
RUN sudo apt-get -y install libmp3lame-dev libtheora-dev
RUN sudo apt-get -y install libvorbis-dev libxvidcore-dev libx264-dev
RUN sudo apt-get -y install libopencore-amrnb-dev libopencore-amrwb-dev
RUN sudo apt-get -y install libavresample-dev
RUN sudo apt-get -y install x264 v4l-utils
 
# Optional dependencies
RUN sudo apt-get -y install libprotobuf-dev protobuf-compiler
RUN sudo apt-get -y install libgoogle-glog-dev libgflags-dev
RUN sudo apt-get -y install libgphoto2-dev libeigen3-dev libhdf5-dev doxygen

RUN sudo apt-get install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
RUN sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
RUN sudo apt-get install libxvidcore-dev libx264-dev
RUN sudo apt-get install libatlas-base-dev gfortran

RUN git clone https://github.com/opencv/opencv.git
RUN cd opencv
RUN git checkout 4.2.0
RUN cd ..
 
RUN git clone https://github.com/opencv/opencv_contrib.git
RUN cd opencv_contrib
RUN git checkout 4.2.0
RUN cd ..

RUN cd opencv
RUN mkdir build
RUN cd build

RUN cmake -D CMAKE_BUILD_TYPE=RELEASE \
            -D CMAKE_INSTALL_PREFIX=/usr/local \
            -D WITH_V4L=ON \
            -D WITH_QT=ON \
            -D WITH_OPENGL=ON \
            -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
            -D WITH_TBB=ON \
            -D BUILD_TBB=ON \
            -D BUILD_opencv_legacy=OFF ..

RUN make -j7
RUN make install

RUN cd ../../

RUN git clone https://github.com/UtkuBulkan/catdetector.git
RUN cd catdetector
RUN git checkout develop

RUN mkdir release
RUN cd release
RUN cmake ..
RUN make


