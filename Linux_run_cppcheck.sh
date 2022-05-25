#!/bin/sh

rm -rf dir_cppcheck_*
mkdir dir_cppcheck_work
mkdir dir_cppcheck_html

libraries_to_check=`find /usr/share/cppcheck/cfg ! -type d | xargs -i -r -n1 basename "{}" | rev | cut -d '.' -f 2- | rev | awk '{print " --library=" $s " "}' | tr -d '\n'`

echo "Libraries to check: ${libraries_to_check}"

# platforms to try: pic16, cray_sv1, arm32-wchar_t4, arm64-wchar_t4
sh -c "\
cppcheck $1 --platform=pic16 \
    --cppcheck-build-dir=./dir_cppcheck_work/ \
    --inline-suppr \
    --force \
    --xml \
    -v \
    --std=c++20 \
    --enable=all \
    ${libraries_to_check}" \
    2> err.xml

cppcheck-htmlreport --source-encoding="iso8859-1" --file=err.xml --report-dir=./dir_cppcheck_html/ --source-dir=./

xdg-open ./dir_cppcheck_html/stats.html &
xdg-open ./dir_cppcheck_html/index.html &
