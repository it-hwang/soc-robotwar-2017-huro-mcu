#!/bin/sh

if [[ ! $1 =~ ^COM[0-9]+$ ]]
then
    echo "Invalid parameter#1: Please check port number."
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


function isReadyForCommand {
    devName=$1

    exec 3<>$devName
    printf "root\r" >&3
    result=""
    read -t 1 -n 5 result<&3
    if [[ $result == "" ]]
    then
        exec 3>&-
        return 0
    fi
    read -t 1 result<&3
    exec 3>&-
    if [[ $result != "root: not found" ]]
    then
        return 0
    fi
    return 1
}


echo "SoC board is entering the download mode..."
error="$(stty -F $devName $speed -parity cs8 -cstopb 2>&1 > /dev/null)"
if [[ $error =~ "Permission denied" ]]
then
    echo "$error"
    exit 1
fi
echo "root" > /dev/ttyS2

isReadyForCommand $devName
if [[ $? == 0 ]]
then
    echo "SoC board failed to enter download mode."
    exit 1
fi
echo "usb_download" > $devName

echo "Download program to SoC Board..."
isSuccess=false
while [[ $isSuccess == false ]]
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

    if [[ $isSuccess == false ]]
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
