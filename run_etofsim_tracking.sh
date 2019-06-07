#!/bin/sh

Nevents=$1
echo "# events: $Nevents"

job=$2

mkdir -p /star/data06/ETOF/etof-sim/files_$job/chain1

# ---- 1st chain (TPC based)
echo ".... running first chain ...."
inputfile=/star/data06/ETOF/etof-sim/files_$job/gen/generator.starsim.fzd

#all chain options for track reconstruction 
chain=fzin,y2019,Event,McEvent,MuDst,sim_T,gen_T,UseXgeom,AgML,StarMagField,ReverseField,ITTF,StiCA,NoSsdIt,NoSvtIt,NoSstIt,Idst,BAna,l0,Tree,logger,VFMinuit,tpcDB,IdTruth,big,McEvout,McAna,MiniMcMk,ReadAll,TpxClu,TpcFastSim,TpcHitMover,-HitFilt,GeantOut,EvOut,clearmem,vpdSim,btofSim,btofMatch
echo "chain options: $chain"

root4star -b -q -l 'bfc.C( '"$Nevents"', "'$chain'", "'$inputfile'" )'

mv generator* /star/data06/ETOF/etof-sim/files_$job/chain1/
mv TrackMap*  /star/data06/ETOF/etof-sim/files_$job/chain1/

