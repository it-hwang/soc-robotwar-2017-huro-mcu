#!/bin/sh

if [[ ! $1 =~ ^COM[0-9]+$ ]]
then
	echo "Invalid parameter#1: Please check port number. $1"
	exit 1
fi
portNo=${1:3}
let ttyNo=$portNo-1
devName="/dev/ttyS$ttyNo"

if [[ ! $2 =~ ^[0-9]+$ ]]
then
	echo "Invalid parameter#2: Please check speed."
	exit 1
fi
speed=$2

if [ -e $devName ]
then
	echo "The device can not be found: $devName"
	exit 1
fi


function hasErrorForCommand {
	devName=$1

	exec 3<>$devName
	printf "root\r" >&3
	result=""
	read -t 1 -n 5 result<&3
	if [[ $result == "" ]]
	then
		exec 3>&-
		return 1
	fi
	read -t 1 result<&3
	exec 3>&-
	if [[ $result != "root: not found" ]]
	then
		return 2
	fi
	return 0
}

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


echo "SoC board is entering the download mode..."
error="$(stty -F $devName $speed -parity cs8 -cstopb 2>&1 > /dev/null)"
if [[ $error =~ "Permission denied" ]]
then
	echo "$error"
	exit 1
fi
writeCommand $devName "root"

nTries=10
isSuccess=false
while [ $isSuccess == false ]
do
	hasErrorForCommand $devName
	result=$?
	if [ $result == 0 ]
	then
		isSuccess=true
	elif [ $result == 1 ] || [ $nTries -le 0 ]
	then
		echo "SoC board failed to enter download mode."
		isSuccess=false
	else
		let nTries=nTries-1
		sleep 0.1
		isSuccess=false
		continue
	fi

	if [ $isSuccess == false ]
	then
		while :
		do
			echo "Would you like to try again? (y or n)"
			read WORD
			case $WORD in
				y|Y)
					nTries=10
					break
					;;
				n|N)
					exit 1
					;;
			esac
		done
	fi
done
writeCommand $devName "usb_download"

echo "Download program to SoC Board..."
isSuccess=false
while [ $isSuccess == false ]
do
	result="$(./tools/RemoteManCLI.exe -target usb -rfw /mnt/f0/main main -run 0 -q <<< q)"

	if [[ $result =~ "You can not do anything anymore" ]]
	then
		echo ""
		echo "$result"
		echo ""
		echo "Download Success!"
		isSuccess=true
	else
		echo ""
		echo "$result"
		echo ""
		echo "Download Failed."
		isSuccess=false
	fi

	if [ $isSuccess == false ]
	then
		while :
		do
			echo "Would you like to try again? (y or n)"
			read WORD
			case $WORD in
				y|Y)
					break
					;;
				n|N)
					exit 2
					;;
			esac
		done
	fi
done
