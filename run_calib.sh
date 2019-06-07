#!/bin/tcsh

#set iStep=1
#set time=(10 "System time: %S  User time: %U  Elapsed time: %E")

#mv StETofCalib.hst.root StETofCalib.hst_old.root

#while ( 2>$iStep )

#	echo execute main loop at $iStep. iteration started at $time

#root4star -b -q -l 'bfc.C( 500, "in, Event, ry2018a, AgML, tpcDB, btofDat, vpdCalib, etofDat, etofCalib, etofHit, etofMatch, etofQa, EvOut", "/star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500001.daq; /star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500005.daq; /star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500006.daq; /star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500007.daq;" )'


#	cp st_physics_*.etofQA.root StETofQA.hst.root
#	root4star -b -q -l 'pl_HitQa.C(18,3,3)'
#	mv pl_HitQA_18_3_3.pdf pl_HitQA_18_3_3_Iter_${iStep}.pdf
#	cp StETofCalib.hst.root StETofCalib_Iter_${iStep}.hst.root

#	@ iStep += 1
#end

#hadd -f ./StETofCalib.hst.root ./out/*/*.etofCalib.root

root4star -b -q -l 'bfc.C( 10000, "in, ry2018a, AgML, tpcDB, btofDat, vpdCalib, etofDat, etofCalib, etofHit, etofMatch, etofQa, cMuDst", "/star/data06/ETOF/muDstAna/st_mtd_19167005_raw_5500003.MuDst.root" )'

#	cp st_physics_*.etofQA.root StETofQA.hst.root
#	root4star -b -q -l 'pl_HitQa.C(18,3,3)'
#	mv pl_HitQA_18_3_3.pdf pl_HitQA_18_3_3_Iter_Final.pdf
#	cp StETofCalib.hst.root StETofCalib_Iter_Final.hst.root


#root4star -b -q -l 'pl_HitQa.C(18,1,1)'
#root4star -b -q -l 'pl_HitQa.C(18,1,2)'
#root4star -b -q -l 'pl_HitQa.C(18,1,3)'
#root4star -b -q -l 'pl_HitQa.C(18,2,1)'
#root4star -b -q -l 'pl_HitQa.C(18,2,2)'
#root4star -b -q -l 'pl_HitQa.C(18,2,3)'
#root4star -b -q -l 'pl_HitQa.C(18,3,1)'
#root4star -b -q -l 'pl_HitQa.C(18,3,2)'
#root4star -b -q -l 'pl_HitQa.C(18,3,3)' 

