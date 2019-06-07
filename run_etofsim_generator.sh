#!/bin/sh
#starver DEV

Nevents=$1
echo "# events: $Nevents"

job=$2

# Generate a random seed for the job
# # crypto quality random number - should also work in parallel since thread safe
random_seed="$(od -vAn -N4 -tu4 < /dev/urandom | tr -d '[:space:]')"
#random_seed=$3

echo "random seed: $random_seed"

mkdir -p /star/data06/ETOF/etof-sim/files_$job
mkdir -p /star/data06/ETOF/etof-sim/files_$job/gen

# ---- Producing sim file .fzd
echo ".... running generator & starsim ...."
root4star -b -q -l 'starsim.C( '"$Nevents"', '"$random_seed"' )'

mv generator* /star/data06/ETOF/etof-sim/files_$job/gen/
