sudo docker build --network=host -f appimage/Dockerfile-trusty -t phf-appimage .
sudo docker run --network=host -it -v /home/anna/PhotoFlow/PhotoFlow:/sources phf-appimage bash /sources/appimage/build.sh
