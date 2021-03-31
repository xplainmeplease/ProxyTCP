#!/bin/bash

PATH_TO_PROXYTCP=$1

sudo cmake -DCMAKE_BUILD_TYPE=Debug .
sudo cmake --build .

./bin/proxytcp_tests

cd Tests/RegressionTests/
./LaunchAllTests.sh $1
cd ../..

mkdir CodeCoverage
cd CodeCoverage
mkdir html

sudo lcov -d "../CMakeFiles/" --capture -o proxytcp.info -c
sudo lcov --remove proxytcp.info "$PATH_TO_PROXYTCP/ProxyTCP/Tests/UnitTests/*" "$PATH_TO_PROXYTCP/ProxyTCP/googletest/*" "/usr/include/*" -o filtered.info
sudo genhtml -o html filtered.info

cd ..

