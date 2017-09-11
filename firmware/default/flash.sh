#!/bin/bash

##################################################################################
######### Just combining some commands in a file that is shorter to type #########
##################################################################################

PROJECT="CMPE146"
PORT="/dev/ttyUSB0"

function HelpMenu()
{
    echo "-----------------------------------------------------------------------"
    echo "This script flashes the SJSU-DEV board then opens the terminal."
    echo "How to use:"
    echo "The project is defaulted to [$PROJECT] and serial port [$PORT]."
    echo "To use default:    ./flash.sh"
    echo "To change project: ./flash.sh --project <PROJECT>"
    echo "To change port:    ./flash.sh --port <PORT>"
    echo "-----------------------------------------------------------------------"
}

while [ "$#" -gt 0 ]
do
    param=$1

    case $param in
        --project)
            # If string, not null
            if [[ ! -z "$2" ]] && [[ "$2" =~ ^[a-zA-Z0-9]+ ]]
            then
                PROJECT="$2"
            else
                echo "Illegal project name: $2"
                exit 1
            fi
            shift
        ;;
        --port)
            # If string, not null, and follows format /dev/ttyUSB<N>
            if [[ ! -z "$2" ]] && [[ "$2" =~ ^\/dev\/ttyUSB[0-9] ]]
            then
                PORT="$2"
            else
                echo "Illegal port name: $2"
                exit 1
            fi
            shift
        ;;
        -h|--help)
            HelpMenu
            exit 0
        ;;
    esac

    shift
done

# Flash first
echo "-----------------------------------------------------------------------"
echo "Launching hyperload..."
echo "-----------------------------------------------------------------------"
./hyperload.py $PORT bin/$PROJECT/$PROJECT.hex
echo "Exited hyperload."

# Open terminal
echo "-----------------------------------------------------------------------"
echo "Launching gtkterm $PORT baud: 38400"
echo "-----------------------------------------------------------------------"
gtkterm -p $PORT -s 38400
echo "Exited gtkterm."
echo "-----------------------------------------------------------------------"