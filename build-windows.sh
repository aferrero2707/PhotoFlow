#! /bin/bash

TRAVIS_BRANCH="stable"
TRAVIS_COMMIT=""
#rm -rf windows && git clone https://github.com/aferrero2707/pf-build-win.git windows
(rm -rf windows && mkdir windows && cp -a ../../pf-build-win/*-msys2.sh windows) || exit 1

#docker run -it -e "TRAVIS_BUILD_DIR=/sources" -e "TRAVIS_BRANCH=${TRAVIS_BRANCH}" -e "TRAVIS_COMMIT=${TRAVIS_COMMIT}" -v $(pwd):/sources photoflow/docker-buildenv-mingw bash #-c /sources/windows/package-w64.sh

docker run -it -e "TRAVIS_BUILD_DIR=/sources" -e "TRAVIS_BRANCH=${TRAVIS_BRANCH}" -e "TRAVIS_COMMIT=${TRAVIS_COMMIT}" -v $(pwd):/sources photoflow/docker-buildenv-mingw-manjaro bash #-c /sources/windows/package-w64.sh
