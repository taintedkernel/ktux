#!/bin/sh

CHANGELOG="CHANGELOG"
TMPCHANGELOG="./CHANGELOG.new"
EDITOR="nano"

SNAPSHOT_FN="$(basename `pwd`)-snapshot-$(date -I)"
ARCHIVE_CNT=$(ls -l $1 | grep $SNAPSHOT_FN | wc -l)

# If no snapshots taken yet today, do not number
if [ $ARCHIVE_CNT -eq 0 ]; then
	SNAPSHOT_FN="$(basename `pwd`)-snapshot-$(date -I).tar.bz2"
# If some exist add a counter
else
	SEQUENCE_NUM=`expr $ARCHIVE_CNT + 1`
	SNAPSHOT_FN="$(basename `pwd`)-snapshot-$(date -I)-build$SEQUENCE_NUM.tar.bz2"
fi

make clean

cp $CHANGELOG $TMPCHANGELOG
echo "" >> $TMPCHANGELOG
echo "$SNAPSHOT_FN:" >> $TMPCHANGELOG

#sleep 5
#cat $TMPCHANGELOG

#echo $PRE_EDIT_MD5
PRE_EDIT_MD5=$(md5sum $TMPCHANGELOG | awk '{print $1}')
$EDITOR $TMPCHANGELOG
POST_EDIT_MD5=$(md5sum $TMPCHANGELOG | awk '{print $1}')
#echo $POST_EDIT_MD5

if [[ $PRE_EDIT_MD5 == $POST_EDIT_MD5 ]]; then
	echo "CHANGELOG not modified, aborting snapshot"
	rm $CHANGELOG
	mv $TMPCHANGELOG $CHANGELOG
	exit
else
	echo "Updating CHANGELOG"
	rm $TMPCHANGELOG
fi

if [[ $1 != "" ]]; then
	tar cvjf $1/$SNAPSHOT_FN --exclude='*.tar.bz2' --exclude='snapshots' .
else
	tar cvjf $SNAPSHOT_FN --exclude='*.tar.bz2' --exclude='snapshots' .
fi

if [ $? -ne 0 ]; then
	echo "Error creating archive"
	exit 1;
fi

if [[ $2 != "" ]]; then
	if [[ $1 != "" ]]; then
		cp $1/$SNAPSHOT_FN $2
	else
		cp $SNAPSHOT_FN $2
	fi
fi