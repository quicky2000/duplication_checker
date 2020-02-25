#!/bin/bash
echo "$# parameters"
launch_directory=$PWD
target_directory=$PWD
if [ $# == 1 ]
then target_directory=$1
fi
echo "Launch directory = $launch_directory"
echo "Target directory = $target_directory"

# list all files
cd $target_directory
rm $launch_directory/all_files.log
find * -name "*" -type f | grep -v "Divers/Logiciels_externes" > $launch_directory/all_files.log
rm $launch_directory/cmd_all_files
quote=\'
echo "rm sha1sum.log sorted_sha1sum.log" > $launch_directory/cmd_all_files
echo "cd $target_directory" >> $launch_directory/cmd_all_files
sed -e 's/ /\\ /g' $launch_directory/all_files.log | sed -e 's/^/sha1sum /g' | sed -e 's/$/ >> sha1sum.log/g' | sed -e 's/(/\\(/g' | sed -e 's/)/\\)/g' | sed -e 's/&/\\&/g' | sed -e 's/\$/\\$/g' | sed -e 's/;/\\;/g' | sed -e 's/`/\\`/g' | sed -e "s/'/_putaindequote_/g" | sed -s 's/_putaindequote_/\\_putaindequote_/g' |  sed -s "s/_putaindequote_/'/g" >> $launch_directory/cmd_all_files && echo "sort sha1sum.log > $launch_directory/sorted_sha1sum.log" >> $launch_directory/cmd_all_files && echo "mv sha1sum.log $launch_directory/"  >> $launch_directory/cmd_all_files
echo "cd $launch_directory"  >> $launch_directory/cmd_all_files
echo "Command generated"
echo "Execute: "
echo "source cmd_all_files; ./main.exe ; cp duplicata.log updated_duplicata.log ; kate updated_duplicata.log"
#EOF
