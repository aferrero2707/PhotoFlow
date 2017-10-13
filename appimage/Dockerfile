FROM centos:6
RUN yum -y install centos-release-scl-rh
RUN yum -y install devtoolset-3-gcc devtoolset-3-gcc-c++

RUN yum -y install epel-release 

RUN yum -y --enablerepo=epel install git make autoconf automake libtool coreutils \
        libzip-devel libxml2-devel \
        tar gzip which grep wget \
        cmake gtkmm24-devel lcms2-devel libpng10-devel libtiff-devel libjpeg-turbo-devel \
        gobject-introspection swig libexif-devel fftw-devel

RUN scl enable devtoolset-3 'g++ -v'
