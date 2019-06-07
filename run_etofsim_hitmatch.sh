#!/bin/sh
#starver DEV

Nevents=$1
job=$2

#Nstart=$1
#Nstop=$2
#job=$3

mkdir -p /star/data06/ETOF/etof-sim/files_$job/chain2


# ---- 2nd chain (ETOF matching)
echo ".... running second chain ...."

inputfile=/star/data06/ETOF/etof-sim/files_$job/chain1/generator.event.root


chain=in,event,y2019,AgML,ReadAll,etofSim,etofMatch,cMuDst
echo "chain options: $chain2"
root4star -b -q -l 'bfcRunner.C( '"$Nevents"', "'$chain'", "'$inputfile'" )'

mv generator* /star/data06/ETOF/etof-sim/files_$job/chain2/
