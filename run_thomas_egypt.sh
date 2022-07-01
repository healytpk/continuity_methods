#!/bin/sh

rm -rf 3rdparty/egypt/work/*
find 3rdparty/egypt/ -iname "*\.egypt" -or -iname "*\.dot" -or -iname "*\.png" -or -iname "*\.svg" | xargs -i -r -n1 rm -rf "{}"
find 3rdparty/egypt/ -mindepth 1 -maxdepth 1 -type l | xargs -i -r -n1 rm -rf "{}"

cat .github/workflows/linux_g++.yml | grep run | tail -2 | cut -d ' ' -f 8- | cut -d '|' -f 3 | xargs -i -r -n1 sh -c "{}"
xdg-open ./3rdparty/egypt/main_as_root.png &
