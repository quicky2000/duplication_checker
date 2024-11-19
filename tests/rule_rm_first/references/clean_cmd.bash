#!/bin/bash

# Rule : RM_FIRST "dir1" "dir2"
if [ ! -L dir2/toto.txt -a -f dir2/toto.txt ]
then
    rm dir1/toto.txt
elif [ -L dir2/toto.txt  ]
then
    echo "dir2/toto.txt" is a link
else
    echo "dir2/toto.txt" do not exist
fi
#EOF
