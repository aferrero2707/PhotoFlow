FROM centos:6
RUN yum -y install centos-release-scl-rh
RUN yum -y install devtoolset-3-gcc devtoolset-3-gcc-c++

RUN yum -y install epel-release git make autoconf automake libtool \
        libzip-devel libxml2-devel libxslt-devel libsqlite3x-devel \
        libudev-devel libusbx-devel libcurl-devel libssh2-devel mesa-libGL-devel sqlite-devel \
        tar gzip which make autoconf automake gstreamer-devel mesa-libEGL coreutils grep wget \
        cmake gtkmm24-devel lcms2-devel libpng10-devel libtiff-devel libjpeg-turbo-devel \
        gobject-introspection-devel swig libexif-devel libxml2-devel fftw-devel

RUN scl enable devtoolset-3 'g++ -v'
