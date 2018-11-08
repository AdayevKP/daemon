#!/bin/bash

curDir=$(pwd)
echo $curDir

dir1="/folder1"
dir2="/folder2"

isOk="it's working!"

path1=$curDir$dir1
path2=$curDir$dir2

echo "creating folders and files"
#creating folders for daemon
mkdir $path1
mkdir $path2

#create config for daemon
cd $curDir
touch config.txt
echo $path1 >> config.txt
echo $path2 >> config.txt

#fill with some files
cd $path1
touch textFile.txt
touch picture.png

cd $path2
touch someVideo.mp4

echo "daemon started"
#starting daemon
cd $curDir
./daemon start

#waiting while daemon doing his work
sleep 2

echo "tests start"
#check if files was moved to folder1 and file was removed from folder2
cd $path2
if test -f ./OTHERS/textFile.txt; 
then echo "test1 ok";
else isok="it's not working";
fi

if test -f ./IMG/picture.png; 
then echo "test2 ok";
else isok="it's not working";
fi

if test -f ./someVideo.mp4; 
then isok="it's not working";
else echo "test3 ok"
fi

#now create new file in folder1 (daemon already runing)
cd $path1
touch audioFile.mp3

#wait while daemon do his work
sleep 2

#check file in folde2
cd $path2

if test -f ./OTHERS/audioFile.txt; 
then echo "test4 ok";
else isok="it's not working";
fi

echo "tests end"
echo "result???"
#hopefully all tests was passed
echo $isOk

echo "stop daemon"
#stop daemon
cd $curDir
./daemon stop

#removin some my stuff)
rm -rf $path1
rm -rf $path2
echo "don't worry. all files was deleted(except config.txt, you might need it)"

