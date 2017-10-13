FROM centos:6
RUN yum -y install centos-release-scl-rh
RUN yum -y install devtoolset-3-gcc devtoolset-3-gcc-c++

RUN yum -y install epel-release 

RUN yum -y --enablerepo=epel install sudo git make autoconf automake libtool coreutils \
        libzip-devel libxml2-devel \
        tar gzip which grep wget \
        cmake3 gtkmm24-devel lcms2-devel libpng10-devel libtiff-devel libjpeg-turbo-devel \
        gobject-introspection swig libexif-devel fftw-devel expat-devel orc-devel

RUN scl enable devtoolset-3 'g++ -v'
RUN rm -f /usr/bin/cmake
RUN ln -s $(which cmake3) /usr/bin/cmake
