#!/bin/bash
set -e -o pipefail

ISSUE=$(cat /etc/issue | xargs echo -n)
if [[ "$ISSUE" == *"Ubuntu"* ]]; then
  UBUNTU_VERSION=$(echo $ISSUE | cut -c7-12 | xargs echo -n)
  echo "Ubuntu: $UBUNTU_VERSION"
else
  echo "This script is not supported by the current Linux distro (works on Ubuntu only)"
  exit 1
fi

hash add-apt-repository || not_found=true
if [[ $not_found ]]; then
    sudo apt install -y software-properties-common
fi

sudo apt update && sudo apt install -y --fix-missing --no-install-recommends build-essential
sudo add-apt-repository ppa:ubuntugis/ubuntugis-unstable

# Check if node is installed
hash node 2>/dev/null || not_found=true 
if [[ $not_found ]]; then
    curl -sL https://deb.nodesource.com/setup_16.x | sudo -E bash -
    sudo apt install -y nodejs
fi

# Check if cmake-js is installed
hash cmake-js 2>/dev/null || not_found=true 
if [[ $not_found ]]; then
    sudo npm install -g cmake-js

    # For building bindings and tests
    sudo npm install nan mocha
fi

if [[ ! -f /usr/lib/libnxs.so ]]; then
    curl -L https://github.com/DroneDB/libnexus/releases/download/v1.0.0/nxs-ubuntu-$UBUNTU_VERSION-amd64.deb --output /tmp/nxs-ubuntu-$UBUNTU_VERSION-amd64.deb
    sudo dpkg-deb -x /tmp/nxs-ubuntu-$UBUNTU_VERSION-amd64.deb /usr
    rm /tmp/nxs-ubuntu-$UBUNTU_VERSION-amd64.deb
fi

sudo apt install -y --fix-missing --no-install-recommends ca-certificates cmake git sqlite3 spatialite-bin libgeos-dev libgdal-dev g++-10 gcc-10 libpdal-dev pdal libzip-dev
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 1000 --slave /usr/bin/g++ g++ /usr/bin/g++-10

# For dist
sudo apt install -y --no-install-recommends musl-dev python3-pip
sudo pip3 install exodus-bundler