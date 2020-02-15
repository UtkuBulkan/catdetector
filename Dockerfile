FROM ubuntu:18.04

RUN apt-get -y update
RUN apt-get -y install build-essential git cmake pkg-config libgtk2.0-dev

## Install dependencies
RUN apt-get -y install build-essential checkinstall cmake pkg-config yasm
RUN apt-get -y install git gfortran
RUN apt-get -y install libjpeg8-dev
RUN apt-get -y install libtiff5-dev
RUN apt-get -y install libtiff-dev
RUN apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libdc1394-22-dev
RUN apt-get -y install libxine2-dev libv4l-dev

RUN apt-get -y install libgtk2.0-dev libtbb-dev qt5-default
RUN apt-get -y install libatlas-base-dev
RUN apt-get -y install libmp3lame-dev libtheora-dev
RUN apt-get -y install libvorbis-dev libxvidcore-dev libx264-dev
RUN apt-get -y install libopencore-amrnb-dev libopencore-amrwb-dev
RUN apt-get -y install libavresample-dev
RUN apt-get -y install x264 v4l-utils
 
# Optional dependencies
RUN apt-get -y install libprotobuf-dev protobuf-compiler
RUN apt-get -y install libgoogle-glog-dev libgflags-dev
RUN apt-get -y install libgphoto2-dev libeigen3-dev libhdf5-dev doxygen

RUN apt-get -y install libjpeg-dev libtiff5-dev
RUN apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
RUN apt-get -y install libxvidcore-dev libx264-dev
RUN apt-get -y install libatlas-base-dev gfortran

RUN git clone https://github.com/opencv/opencv.git
RUN cd /opencv && git checkout 4.2.0
 
RUN git clone https://github.com/opencv/opencv_contrib.git
RUN cd /opencv_contrib && git checkout 4.2.0

RUN mkdir /opencv/build && cd /opencv/build && cmake -D CMAKE_BUILD_TYPE=RELEASE \
            -D CMAKE_INSTALL_PREFIX=/usr/local \
            -D WITH_V4L=ON \
            -D WITH_QT=ON \
            -D WITH_OPENGL=ON \
            -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
            -D WITH_TBB=ON \
            -D BUILD_TBB=ON \
            -D BUILD_opencv_legacy=OFF .. && make -j7 && make install

RUN git clone https://github.com/UtkuBulkan/catdetector.git
RUN cd /catdetector && git checkout develop

RUN mkdir /catdetector/release && cd /catdetector/release && cmake .. && make
RUN mkdir /catdetector/release/data/


RUN apt -y install wget

RUN cd /catdetector/data/yolo/ && wget https://pjreddie.com/media/files/yolov3.weights

RUN apt-get -y install lsb-release && \
    export GCSFUSE_REPO=gcsfuse-`lsb_release -c -s` && \
    echo "deb http://packages.cloud.google.com/apt $GCSFUSE_REPO main" | tee /etc/apt/sources.list.d/gcsfuse.list && \
    curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key add -

RUN apt-get update
RUN apt-get -y install gcsfuse

RUN export CLOUD_SDK_REPO="cloud-sdk-$(lsb_release -c -s)" && \
    echo "deb http://packages.cloud.google.com/apt $CLOUD_SDK_REPO main" | tee -a /etc/apt/sources.list.d/google-cloud-sdk.list && \
    curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key add - && \
    apt-get update && apt-get -y install google-cloud-sdk

RUN cd  /
RUN git clone https://UtkuBulkan:Gribeyaz1@github.com/UtkuBulkan/catdetector_gcloud_certificates
RUN export GOOGLE_APPLICATION_CREDENTIALS='/catdetector_gcloud_certificates/service-cert/supereye.co.uk-storage.json'

RUN cd /catdetector && git pull
RUN cd /catdetector/release && make
RUN chmod 0777 /catdetector/catdetector_script.sh


ENTRYPOINT ["/bin/bash", "-c", "/catdetector/catdetector_script.sh"]

#ENTRYPOINT ["/bin/bash"]
#CMD ["sleep", "infinity"]


