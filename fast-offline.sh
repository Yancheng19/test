
#2018 Fast Offline Chain with etof added

#fochain="P2018a StiCA btof mtd PicoVtxDefault BEmcChkStat OSpaceZ2 OGridLeak3D -hitfilt"

#chain="P2018a StiCA btof mtd etofDat EvOut PicoVtxDefault BEmcChkStat OSpaceZ2 OGridLeak3D -hitfilt"

#chain="P2018a StiCA btof mtd etofDat etofCalib etofHit etofQa etofMatch EvOut PicoVtxDefault BEmcChkStat OSpaceZ2 OGridLeak3D -hitfilt"

chain="DbV20180730,P2018a,StiCA,-beamline3D,btof,mtd,BEmcChkStat,CorrX,OPr40,-OPr13,OSpaceZ2,OGridLeakFull,-hitfilt,picowrite,picoVtxDefault,etofDat,EvOut,KeepTpcHit"

#chain="in Event AgML etofMatch EvOut"
root4star -b -q -l 'bfc.C( 50, "'"$chain"'", "'"$1"'" )'

