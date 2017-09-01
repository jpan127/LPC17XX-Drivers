#!/bin/bash

function HelpMenu()
{
	echo -e "Usage: ./build [-e|--entity][-t|--threads][-d|--debug] name_of_project"
	echo -e "       ./build --help"
	echo -e "       ./build spotless"
	echo -e "       ./build clean name_of_project"
}

### if number of arguments is less than 1, exit
if [ "$#" -lt 1 ]
then
	HelpMenu
	exit 1
fi

### Setup Project Target
APPLICATION="CMPE146"
THREADS="-j1"
ENTITY_NAME=DBG
DEBUG=0
SILENT="-s"

### if first argument is spotless, make spotless
if [ "$1" == "spotless" ]
then
	make spotless
	exit 0
### else if first argument is clean, make clean
elif [ "$1" == "clean" ]
then
	make clean PROJ=$2
	exit 0
fi

# some arguments don't have a corresponding value to go with it such
# as in the --default example).
# note: if this is set to -gt 0 the /etc/hosts part is not recognized ( may be a bug )


### Parse arguments
while [[ $# -gt 1 ]]
do
	key="$1"

	case $key in
		-e|--entity)
			### If argument 2 is not empty and matches the regex, save argument
			if [[ ! -z "$2" ]] && [[ "$2" =~ ^[a-zA-Z0-9]+ ]]
			then
				ENTITY_NAME="$2"
			else
				echo "Illegal entity: $2"
				exit 1
			fi
			shift # past argument
		;;
		-t|--threads)
			if [[ ! -z "$2" ]] && [[ "$2" =~ ^[0-9]+ ]]
			then
				THREADS="-j$2"
			else
				echo "Illegal thread count: $2"
				exit 1
			fi
			shift # past argument
		;;
		-d|--debug)
			DEBUG=1
		;;
		-v|--verbose)
			SILENT=""
		;;
		-h|--help)
			HelpMenu
			exit 0
		;;
		*)
		        # unknown option
		;;
	esac
	shift # past argument or value
done

### If not silent print information
if [[ -z "$SILENT" ]]
then
	echo
	echo "==========VERBOSE INFORMATION=============="
	echo "    APPLICATION=$APPLICATION"
	echo "    THREADS=$THREADS"
	echo "    ENTITY_NAME=$ENTITY_NAME"
	echo "    DEBUG=$DEBUG"
	echo "==========================================="
	echo
fi

### If application name not empty save it 
if [[ ! -z "$1" ]] && [[ ! "$1" =~ ^-.* ]]
then
	APPLICATION=$1
else
	echo -e "\nInvalid Application name: $1\nlast argument must be application name"
	exit 0
fi

### Make target
make $SILENT PROJ=$APPLICATION ENTITY=$ENTITY_NAME $THREADS DEBUG=$DEBUG