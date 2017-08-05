#! /bin/bash
arch=$1

echo "$HOME/inst/bin/crossroad $arch phf-build"
$HOME/inst/bin/crossroad $arch phf-build <<EOF
echo "Hello"
EOF
