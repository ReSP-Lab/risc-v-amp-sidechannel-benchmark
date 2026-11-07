#!/bin/bash

#
# This file run each benchmark code and output results
#
# Copyright (C) 2024-2025 Cédrick Austa <cedrick.austa@ulb.be>
# Copyright (C) 2020 Shuwen Deng <shuwen.deng@yale.edu>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
#
#

set -e

input_folder=$1
output_folder=$2
output_name=$3

mkdir -p $output_folder

i=0
for file in `find "$input_folder" -maxdepth 1 -type f -name '*AccOne*AccTwo*AccThr*' | sort -n`
do
  echo "i is $i"
  let i=$i+1
  filename=$(echo "$file" | sed 's_.*/__')
  echo "Executing: file $filename"
  ${file} $output_folder/$output_name > /dev/null &

  ps
  PID2=$(ps -ef | grep ${file} | awk '{print $2}')
  j=0 
  pid=0
  ori=1
  for word in $PID2
  do
    let j=$j+1
      echo $word
      if [ "$j" == "$ori" ]
    then
        pid=$word
    fi
  done
  echo $pid
  echo "Before finishing ${filename}"
  while ps -ef | grep ${file} | grep -v grep > /dev/null; do sleep 0.5; done
  echo "Finished ${filename}"

done
echo "Finished"
