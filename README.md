# Daemon
Simple daemon for linux, which checks files in folder1 and copy them to folder2.

# Description
Daemon's script: Daemon deletes all files in folder2 and then copy png files from folder1 to folder2/IMG and other files to folder2/OTHERS.  
Daemon monitoring changes in folder1 and, if some files was moved to folde1 or created in folder1, it will perform it's script.

# Using: 
set full path for folder1 and folder2 in config.txt  
build using  
make  
use commands  
./deamon start   
./daemon stop   
for starting and stopping daemon  

# Testing:
make  
./test.sh

# Notes
also you can send SIGHUP signal to deamon for rereading config file  
