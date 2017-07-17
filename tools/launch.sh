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

stty -F $devName $speed -parity cs8 -cstopb
echo "root" > $devName
echo "/mnt/f0/main" > $devName
