#!/bin/bash

timestamp() {
  date +"%T"
}

date=`date '+%Y-%m-%d-%H-%M-%S'`

gzfile=${date}_unrealgo.tar.gz
tar -zcf ../${gzfile} . --exclude='.git' --exclude='.idea' --exclude='cmake-build-debug' --exclude='bin'
cd ..
cp ${gzfile} $1