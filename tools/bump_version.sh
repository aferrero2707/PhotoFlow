#! /bin/bash

version="$1"

echo "$version" > VERSION

cp CHANGELOG /tmp/__CHANGELOG__

echo "=========================================" > CHANGELOG
echo "Version $version" >> CHANGELOG
echo "" >> CHANGELOG
cat /tmp/__CHANGELOG__ >> CHANGELOG
rm /tmp/__CHANGELOG__
