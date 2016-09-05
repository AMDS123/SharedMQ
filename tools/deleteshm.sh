#!/bin/bash

shmid=`cat /tmp/shmmq/shmid`
echo "close shm $shmid"
ipcrm -m $shmid
