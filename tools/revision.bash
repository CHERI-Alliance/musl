#!/bin/sh

cwd=${1}

cd ${cwd}

if test -d .git ; then
if type git >/dev/null 2>&1 ; then
git describe --always
else
sed 's/$/-git/' < VERSION
fi
else
cat VERSION
fi
