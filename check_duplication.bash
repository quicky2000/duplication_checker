#!/bin/bash
# list all files
rm all_files.log
find * -name "*" -type f | grep -v "Divers/Logiciels_externes" > all_files.log
rm cmd_all_files
quote=\'
sed -e 's/ /\\ /g' all_files.log | sed -e 's/^/sha1sum /g' | sed -e 's/$/ >> sha1sum.log/g' | sed -e 's/(/\\(/g' | sed -e 's/)/\\)/g' | sed -e 's/&/\\&/g' | sed -e 's/\$/\\$/g' | sed -e 's/;/\\;/g' | sed -e 's/`/\\`/g' | sed -e "s/'/_putaindequote_/g" | sed -s 's/_putaindequote_/\\_putaindequote_/g' |  sed -s "s/_putaindequote_/'/g" > cmd_all_files
echo "Command generated"
echo "Execute: "
echo "rm sha1sum.log sorted_sha1sum.log ; source cmd_all_files; sort sha1sum.log > sorted_sha1sum.log ; ./main.exe ; cp duplicata.log updated_duplicata.log ; kate updated_duplicata.log"
#EOF
