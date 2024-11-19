#!/bin/bash
ok_to_rm=1

# Keep only : "dir2/triple2.txt"
if [ ! -f dir2/triple2.txt -o -L dir2/triple2.txt ]
then
    ok_to_rm=0
    if [ ! -f dir2/triple2.txt ]
    then
        echo "File dir2/triple2.txt is missing"
    else
        echo "File dir2/triple2.txt is a link"
    fi
fi
if [ $ok_to_rm -eq 1  ]
then
    rm dir1/triple1.txt
    rm triple.txt
fi
#EOF
