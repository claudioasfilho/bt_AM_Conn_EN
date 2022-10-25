#!/bin/bash
# Basic while loop

declare -a test_array

test_array=(440112534 440179444 440172391 440179577 440178999 440179604 440179393 )


for i in ${test_array[@]}
do

echo -e "Starting to program Board "
echo $i
  #commander flash outputfile.hex -s $i
  commander flash bt_AM_Conn_EN.s37 -s $i
echo -n "programming concluded on Board "
echo $i

done

