#!/bin/bash
# Basic while loop

declare -a test_array

test_array=(440169493 440112534	440179444 440119993	440179577 440179604 440172391	440195256 440178999	440196730 440179393	440235155 440169493)


for i in ${test_array[@]}
do

echo -e "Starting to program Board "
echo $i
  #commander flash outputfile.hex -s $i
  commander device reset -s $i #-d BG22
echo -n "Device Reset "
echo $i

done

