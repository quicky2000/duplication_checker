# Duplication_checker

Continuous integration with [Travis-Ci](https://travis-ci.org/quicky2000/duplication_checker) : ![Build Status](https://travis-ci.org/quicky2000/duplication_checker.svg?branch=master)

Please see LICENSE for info on the license.

This tool detect Duplications by performing the following steps:
* List all files
* Generate a command file that will compute all files SHA1sum and store them in a file
* Sort SHA1sum
* An executable will examine sorted SHA1sum and generate a command to clean duplications according to rules

## How to use it

`./check_duplication.bash [target directory]`
Run command displayed by check_duplication.bash
Run clean_cmd.bash to remove duplication according to defined rules

## check_duplication.bash

### Inputs

None

### Outputs

* all_files.log : list of all files presents on disk 
* cmd_all_files : commande file to generate SHA1sum of listed files

## cmd_all_files

### Inputs

None

### Outputs

* sha1sum.log : list of all SHA1sum with their associated file

## main.exe

### Inputs

* sorted_sha1sum.log

### Outputs

* duplicata.log : List of duplicated files
* clean_cmd.bash : command file to remove duplications according to rules

