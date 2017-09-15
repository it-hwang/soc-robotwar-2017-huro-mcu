#!/bin/sh

if [[ ! $1 =~ ^COM[0-9]+$ ]]
then
	echo "Invalid parameter#1: Please check port number."
	exit 0
fi
portNo=${1:3}
let ttyNo=$portNo-1
devName="/dev/ttyS$ttyNo"

if [[ ! $2 =~ ^[0-9]+$ ]]
then
	echo "Invalid parameter#2: Please check speed."
	exit 0
fi
speed=$2


function writeCommand {
	devName=$1
	commandLine=$2

	exec 3<>$devName
	for i in $(seq 1 ${#commandLine})
	do
		letter=${commandLine:i-1:1}
		printf "$letter" >&3
	done
	printf "\r" >&3
	exec 3>&-
}


stty -F $devName $speed -parity cs8 -cstopb
writeCommand $devName "root"
writeCommand $devName "cd /mnt/f0"
writeCommand $devName "./main"
writeCommand $devName "1"	# direct start command
