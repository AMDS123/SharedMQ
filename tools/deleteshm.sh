#!/bin/bash

shmid=`ipcs -m | awk '/Share/{getline;print $2}'`
ipcrm -m $shmid
