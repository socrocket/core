#!/bin/bash
find build/software -name "*.log" | xargs -I {} bash -c '\
  echo $(basename {} | sed "s/-/./g" | cut -d"." -f 1),\
       $(basename {} | sed "s/-/./g" | cut -d"." -f 3), \
       $(tail {} | grep Delta | cut -f1-2 -d" " | sed "s/@//g") \
  '
