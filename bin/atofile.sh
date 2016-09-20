#!/bin/bash

#atofile.sh file content_to_append
if [ -f $1 -a -s $1 ]; then
  sed -i -e "a $2" $1
else
  echo $2 > $1
fi




