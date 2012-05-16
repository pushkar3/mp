#!/bin/bash

if [ ! -n "$1" ]
then
  echo "Usage: `basename $0` <foldername>"
  exit $E_BADARGS
fi  

make && 
rm -rf $1/*.txt
./bpp_planner $1 &&
./bpp_stat $1 &&
./bpp_pose $1 
#./simplex_planner $1
