#!/bin/bash

# Rule : RM_SECOND "dir1" "dir2"
if [ ! -L dir1/toto.txt -a -f dir1/toto.txt ]
then
    rm dir2/toto.txt
elif [ -L dir1/toto.txt  ]
then
    echo "dir1/toto.txt" is a link
else
    echo "dir1/toto.txt" do not exist
fi
#EOF
