#!/bin/bash

#Title           :assignment2_package.sh
#description     :This script will package assignment2 for submission.
#Author          :Swetank Kumar Saha <swetankk@buffalo.edu>
#Version         :1.0
#===================================================================================================


# https://gist.github.com/davejamesmiller/1965569
function ask {
    while true; do
 
        if [ "${2:-}" = "Y" ]; then
            prompt="Y/n"
            default=Y
        elif [ "${2:-}" = "N" ]; then
            prompt="y/N"
            default=N
        else
            prompt="y/n"
            default=
        fi
 
        # Ask the question
        read -p "$1 [$prompt] " REPLY
 
        # Default?
        if [ -z "$REPLY" ]; then
            REPLY=$default
        fi
 
        # Check if the reply is valid
        case "$REPLY" in
            Y*|y*) return 0 ;;
            N*|n*) return 1 ;;
        esac
 
    done
}

echo
echo -n "Enter your UBIT username (without the @buffalo.edu part) and press [ENTER]: "
read ubitname

if [ -d "./${ubitname}" ]; 
then
    echo "Directory with given UBITname exists"
else
    echo "No directory named ${ubitname} found. Try again!"
    exit 0
fi

echo "Verifying contents ..."

echo
echo "ABT: "
FILE=`find ./$ubitname/src/ -name "abt.c" -o -name "abt.cpp"`
if [ -n "$FILE" ];
then
    echo "File $FILE exists"
else
    echo "Missing abt.c or file named incorrectly!"
    exit 0
fi

echo
echo "GBN: "
FILE=`find ./$ubitname/src/ -name "gbn.c" -o -name "gbn.cpp"`
if [ -n "$FILE" ];
then
    echo "File $FILE exists"
else
    echo "Missing gbn.c or file named incorrectly!"
    exit 0
fi

echo
echo "SR: "
FILE=`find ./$ubitname/src/ -name "sr.c" -o -name "sr.cpp"`
if [ -n "$FILE" ];
then
    echo "File $FILE exists"
else
    echo "Missing sr.c or file named incorrectly!"
    exit 0
fi

echo
echo "Makefile: "
FILE=./$ubitname/Makefile
if [ -f $FILE ];
then
    echo "File $FILE exists"
else
    echo "Missing Makefile or file named incorrectly!"
    exit 0
fi

echo
echo "Analysis file: "
FILE=`find ./$ubitname/ -name "Analysis_Assignment2.pdf"`
if [ -n "$FILE" ];
then
        echo "File $FILE exists"
else
        echo "Missing Analysis_Assignment2.pdf or file named incorrectly!"
        exit 0
fi

echo
echo "Packaging ..."
cd ${ubitname}/ && tar --exclude='./scripts' -zcvf ../${ubitname}_pa2.tar * && cd ..
echo "Done!"
echo "IMPORTANT: Your submission is NOT done!"
echo "You need to use the submit_cse489/submit_cse589 (available on CSE servers) script to submit this tarball."