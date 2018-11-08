# daemon
Simple daemon for linux, which cheks files in folder1 and copy files from folder1 to folder2.
Daemon's script: Daemon delete all files in folder2 and then copy png files to folder2/IMG and other file to folder2/OTHERS.
Daemon monitoring changes in folde1 and, if some files was moved to folde1 or created in folder1 it will perform it's script.

Using: 
set full path for folder1 and folder2 in config txt
build using make command
use commands./deamon start .daemon stop for starting and stopping daemon

Testing:
make
./test.sh
