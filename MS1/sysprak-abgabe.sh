#!/bin/bash
# VERSION: 1.1.0
set -e

## Config
GAME_TYPE_URL=bashni
GAME_TYPE_NAME=Bashni

function finish() {
	echo
	echo "################################"
	echo "# BONUS: $PUNKTE / 2                 #"
	echo "################################"
}

function usage() {
	echo "USAGE: $(basename "$0") [BUILD_DIR] [LOG] [ZIP-FILE] [OPTIONS]" >&2
    echo "OPTIONS:"
    echo -e "\t --spectate \t Open default browser to spectate the game"
	exit
}


PUNKTE=0


PLAYER1=1
PLAYER2=2
EXECNAME=sysprak-client
SPECTATE=false

if [[ $# -lt 3 ]] || [[ $# -gt 4 ]]; then
	usage
fi

trap finish EXIT

BUILD_DIR=$1
VALGRIND_LOG=$2
FILE=$3

if [[ $4 == "--spectate" ]]; then
    SPECTATE=true
fi

## Test 1: Check size maximum of 500 KiB
if [ $(unzip -p "$FILE" | wc -c) -lt 512000 ]; then
	echo "Filesize: OK"
else
	echo "Filesize too big!"
	exit
fi

## create empty build dir
if [ -d "$BUILD_DIR" ]; then
	echo "$BUILD_DIR already exists"
	exit 1
fi
mkdir -p $BUILD_DIR

## unzip files and compile
unzip -q $FILE -d $BUILD_DIR
cd $BUILD_DIR
make -s clean

## Test 2: makeflags
MAKEFLAGS=$(make -n)
if [[ $MAKEFLAGS == *"-Wall"* ]] \
&& [[ $MAKEFLAGS == *"-Wextra"* ]] \
&& [[ $MAKEFLAGS == *"-Werror"* ]]; then
	echo "Makeflags: OK"
else
	echo "Makeflags do not contain -Wall, -Wextra or -Werror!"
	exit
fi

## Test 3: Check if the project compiles without errors or warnings
if make -s; then
	echo "Compilation: OK"
else
	echo "Compilation did not complete without warnings or errors!"
	exit
fi

## create a new game
ID=$(curl http://sysprak.priv.lab.nm.ifi.lmu.de/api/v1/matches \
-H "Content-Type: application/json" \
-X POST \
-d '{"type":"'$GAME_TYPE_NAME'","gameGeneric":{"name":"","timeout":3000},"gameSpecific":{},"players":[{"name":"White Player","type":"COMPUTER"},{"name":"Black Player","type":"COMPUTER"}]}' 2>/dev/null | grep -Eow '([a-z0-9]{13})')

if [[ $ID != "" ]]; then
  echo "Generated new game with ID \"$ID\"."
else
  echo "Error creating new Game-ID. Exiting..."
  exit
fi


## start PLAYER1
GAME_ID=$ID PLAYER=$PLAYER1 make play &>/dev/null &

## check Valgrind for PLAYER2
rm -f $VALGRIND_LOG
valgrind --log-file=$VALGRIND_LOG -q --leak-check=full --trace-children=yes ./$EXECNAME -g $ID -p $PLAYER2 &>/dev/null &

## launch browser
if $SPECTATE; then
    xdg-open http://sysprak.priv.lab.nm.ifi.lmu.de/$GAME_TYPE_URL/\#$ID &>/dev/null &
fi

wait

while true; do
	read -p "Did the game finish successfully? [y/n] " yn
	case $yn in
		[Yy]* ) PUNKTE=$((PUNKTE+1)); break;;
		[Nn]* ) exit;;
		* ) echo "Please answer yes or no.";;
	esac
done

# check valgrind errors
if [[ $(cat $VALGRIND_LOG) == "" ]]; then
	echo "Valgrind: OK"
	((PUNKTE++))
else
	echo "Valgrind did find errors:"
	cat $VALGRIND_LOG
fi

cd ..
if [ -d "$BUILD_DIR" ]; then
	rm -r "$BUILD_DIR"
fi
