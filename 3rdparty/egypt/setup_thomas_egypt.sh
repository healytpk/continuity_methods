#!/bin/sh

if [ $EUID -ne 0 ]; then
   echo "This script must be run as root in order to run 'apt install', so trying to use 'sudo'"
   sudo apt install -y librsvg2-bin graphviz
   if [ $? -ne 0 ]; then
      echo "The 'sudo apt install' command failed. Bailing out."
      exit 1
   fi
else
   apt install -y librsvg2-bin graphviz
   if [ $? -ne 0 ]; then
      echo "The 'apt install' command failed. Bailing out."
      exit 1
   fi
fi

set -x

cd 3rdparty/egypt/

rm -rf work/
mkdir -p work/
chmod 777 work/
cd work/

g++ -o thomas_egypt_get_scope    -std=c++11 -O3 ../thomas_egypt_get_scope.cpp
g++ -o thomas_egypt_set_new_root -std=c++11 -O3 ../thomas_egypt_set_new_root.cpp

tar -zxf ../egypt-1.10.tar.gz
cd egypt-1.10/
perl Makefile.PL
make
ln -sf ./work/egypt-1.10/egypt ../../egypt
cd ../../

chmod -R o+r,o+w egypt work/

cd ../..
