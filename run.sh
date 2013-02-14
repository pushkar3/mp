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

echo "<b>Starting Planner</b>" >> $3
/var/www/mp/bpp_planner DEBUG4 $2 &>> $3 &&
echo "<b>Stats</b>" >> $3
/var/www/mp/bpp_stat $2 &>> $3 &&
echo "<b>Posing the Order for Palletizing</b>" >> $3
/var/www/mp/bpp_pose $2 &>> $3 &&
echo "<b>Starting Simplex Planner</b>" >> $3
/var/www/mp/simplex_planner greedy $2 &>> $3 &&
echo "<b>Transferring Plan to Viewer</b>" >> $3
/var/www/mp/bpp_viewer $2 &>> $3 &&
echo "<b>Check the Pallet Viewer</b>" >> $3
cp $2/index.html /var/www/palletviewer/
cp $2/order.xml /var/www/palletviewer/
cp $2/packlist.xml /var/www/palletviewer/
