#!/bin/bash
echo "do make clean and make"
make clean
make
echo "run the httpproxy"
./httpproxy &
echo "run the httpproxy successfully"
while true ;
do
  continue ;
done