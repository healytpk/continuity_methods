#!/bin/sh

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root"
   exit 1
fi

set -x

mkdir -p /tmp/thomas_egypt/

g++ -o /tmp/thomas_egypt/thomas_egypt_get_scope -std=c++11 -O3 thomas_egypt_get_scope.cpp
g++ -o /tmp/thomas_egypt/thomas_egypt_set_new_root -std=c++11 -O3 thomas_egypt_set_new_root.cpp

install -m 775 -o root -g root ./thomas_egypt /usr/bin/
install -m 775 -o root -g root ./thomas_egypt_Keep_If_In_Scopes.bash /usr/bin/
install -m 775 -o root -g root /tmp/thomas_egypt/thomas_egypt_get_scope /usr/bin/
install -m 775 -o root -g root /tmp/thomas_egypt/thomas_egypt_set_new_root /usr/bin/

tar -zxf egypt-1.10.tar.gz --directory /tmp/thomas_egypt/
cd /tmp/thomas_egypt/egypt-1.10
perl Makefile.PL
make
make install
cd -

rm -rf /tmp/thomas_egypt/

apt install -y librsvg2-bin graphviz
