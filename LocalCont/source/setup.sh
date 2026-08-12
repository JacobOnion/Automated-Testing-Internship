#!/bin/bash
set -e

apt-get update

apt-get install -y \
    software-properties-common \
    build-essential \
    libseccomp2 \
    libseccomp-dev \
    libc6-dev \
    libglu1-mesa \
    libglfw3-dev \
    mesa-utils \
    libglu1-mesa-dev \
    libgl1-mesa-dev \
    libgl1-mesa-dri \
    xvfb \
    cowsay \
    jq

add-apt-repository ppa:ubuntu-toolchain-r/test -y

apt-get update

apt-get install -y \
    gcc-16 \
    g++-16

update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100

#SETUP USER

echo "=== SETUP USER ==="
groupadd student_group
useradd -m -g student_group student

#echo "export student_gid=$(id -g student_group)" >> 
#echo "export student_uid=$(id -u student)" >>

#SETUP FILE PERMS

echo "=== SETUP FILE PERMS ==="
chown -R root:root /LocalCont
chmod -R o-rwx /LocalCont


chmod 711 /LocalCont
chmod 711 /LocalCont/source
chmod 711 /LocalCont/source/build
chmod 711 /LocalCont/source/build/autograder
chmod 755 /LocalCont/source/build/autograder/libScreenGrab.so

#chmod -R 733 /autograder/source/screenshots

chmod -R 755 /LocalCont/source/OpenGL

#chmod -R 777 /autograder/submission

# allow others to navigate to /autograder, but not read/write files
chmod 751 /LocalCont


echo "=== SETUP COMPLETE ==="

#EOF