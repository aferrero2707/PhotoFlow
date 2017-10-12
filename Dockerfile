FROM centos:6
RUN yum -y install epel-release git make autoconf automake libtool \
        libzip-devel libxml2-devel libxslt-devel libsqlite3x-devel \
        libudev-devel libusbx-devel libcurl-devel libssh2-devel mesa-libGL-devel sqlite-devel \
        tar gzip which make autoconf automake gstreamer-devel mesa-libEGL coreutils grep wget
