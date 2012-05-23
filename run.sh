#!/bin/bash

if [ ! -n "$2" ]
then
  echo "Usage: `basename $0 $1` <plan|noplan> <foldername>"
  exit $E_BADARGS
fi  

make &&

if [ $1 = "plan" ]
then
    echo 'Will plan in this run'
    rm -rf $2/*.txt
fi

./bpp_planner $2
./bpp_stat $2 &&
./bpp_pose $2 
./simplex_planner greedy $2
./bpp_viewer $2
cp $2/index.html /var/www/palletviewer/
