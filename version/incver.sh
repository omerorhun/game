#/bin/bash

DIR=$(dirname $0)

if [ -f "$DIR/cver.sh" ]
then
	. $DIR/cver.sh
else
	MAJOR_VERSION_NO=0
	MINOR_VERSION_NO=1
	BUGFIX_NO=0
	BUILD_NO=0
	VERSION_SUFFIX_STR=0.1.0.0
fi

NEW_BUILD_NO=$(($BUILD_NO + 1))

rm $DIR/cver.sh $DIR/version.h
touch $DIR/cver.sh $DIR/version.h

echo MAJOR_VERSION_NO=$MAJOR_VERSION_NO >> $DIR/cver.sh
echo MINOR_VERSION_NO=$MINOR_VERSION_NO >> $DIR/cver.sh
echo BUGFIX_NO=$BUGFIX_NO >> $DIR/cver.sh
echo BUILD_NO=$NEW_BUILD_NO >> $DIR/cver.sh
VERSION_SUFFIX_STR=$MAJOR_VERSION_NO.$MINOR_VERSION_NO.$BUGFIX_NO.$NEW_BUILD_NO 
echo VERSION_SUFFIX_STR=$VERSION_SUFFIX_STR >> $DIR/cver.sh

echo Version: $VERSION_SUFFIX_STR

echo "/************************************" >> $DIR/version.h
echo " *                                  *" >> $DIR/version.h
echo " *       Version information        *" >> $DIR/version.h
echo " *                                  *" >> $DIR/version.h
echo " ***********************************/" >> $DIR/version.h
echo  >> $DIR/version.h
echo \#define "VERSION_PRODUCTVERSION_STR  "\"$MAJOR_VERSION_NO.$MINOR_VERSION_NO.$BUGFIX_NO\" >> $DIR/version.h
echo \#define "VERSION_FILEVERSION_STR     "\"$MAJOR_VERSION_NO.$MINOR_VERSION_NO.$BUGFIX_NO.$NEW_BUILD_NO\" >> $DIR/version.h
echo \#define "VERSION_MAJOR_NO            "$MAJOR_VERSION_NO >> $DIR/version.h
echo \#define "VERSION_MINOR_NO            "$MINOR_VERSION_NO >> $DIR/version.h
echo \#define "VERSION_BUGFIX_NO           "$BUGFIX_NO >> $DIR/version.h
echo \#define "VERSION_BUILD_NO            "$NEW_BUILD_NO >> $DIR/version.h
