#!/bin/bash

if [ ! -n "$2" ]
then
  echo "Usage: `basename $0 $1` <plan|noplan> <foldername>"
  exit $E_BADARGS
fi  

make > $3

if [ $1 = "plan" ]
then
    echo 'Will plan in this run'
    rm -rf $2/*.txt
fi

echo "bpp_planner" >> $3
/var/www/mp/bpp_planner DEBUG4 $2 &>> $3 &&
echo "bpp_stat" >> $3
/var/www/mp/bpp_stat $2 &>> $3 &&
echo "bpp_pose" >> $3
/var/www/mp/bpp_pose $2 &>> $3 &&
echo "bpp_simplex_planner" >> $3
/var/www/mp/simplex_planner greedy $2 &>> $3 &&
echo "bpp_viewer" >> $3
/var/www/mp/bpp_viewer $2 &>> $3 &&
echo "bpp_cp" >> $3
cp $2/index.html /var/www/palletviewer/
cp $2/packlist.xml /var/www/palletviewer/
