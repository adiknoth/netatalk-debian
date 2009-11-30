#!/bin/bash

# This script was found here:
# http://netatalk.sourceforge.net/wiki/index.php/DebianUpgradeto2

DIR="$1"
NEWDIR="${DIR}/.AppleDB"

# Name binaries
DBDUMPORIG=/usr/bin/db4.2_dump
DBDUMPNEW=/usr/bin/db4.8_dump
DBLOADNEW=/usr/bin/db4.8_load
CNIDCREATOR=/usr/bin/cnid2_create

( [ -d "${DIR}" ] || [ -d "${NEWDIR}" ] ) || {
	echo 'Directory not supplied or does not exist. Aborting'
	exit 1
}

([ -x ${DBDUMPORIG} ] && [ -x ${DBDUMPNEW} ] && [ -x ${DBLOADNEW} ] && [ -x ${CNIDCREATOR} ]) || {
	echo 'Could not find database binaries. Aborting'
	exit 1
}

echo "About to process \"${DIR}\""
echo -n "Press ^C to abort "

for ii in [ 0..6 ]; do
	echo -n "."
	sleep 1
done
echo "."

echo "Changing to \"${NEWDIR}\" directory ..."
cd "${NEWDIR}"

echo -n "Current directory "
pwd

sleep 2
echo "------------------------------------------------------------------"

echo "Dumping cnid.db ..."
${DBDUMPORIG} -f cnid.dump cnid.db

echo "Dumping didname.db ..."
${DBDUMPORIG} -f didname.dump didname.db

echo "Making new cnid2 database ..."
${CNIDCREATOR} ../

echo "Deleting old log and info files ..."
rm -f __db.* log.*

echo "Zipping original cnid.db and cnid.dump files."
echo "(This avoids uniconv failing on finding a version 1 cnid database)"
gzip cnid.db
gzip cnid.dump

echo

echo "Loading new cnid2 database ..."
${DBLOADNEW} -f cnid2.dump cnid2.db

echo
echo "------------------------------------------------------------------"
echo "Done."
echo

echo "Please now check the permissions of the cnid2.db file."
echo "The cnid2.db must have the same uid:gid as the volume owner."

exit 0
