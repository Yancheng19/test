
# create the digis inside StEvent
#root4star -b -q -l 'bfc.C( 100000, "in, Event, tpcDB, btofDat, vpdCalib, etofDat, EvOut, cMuDst", "/star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500001.daq" )'

#mv st_physics_*.event.root in/
#mv st_physics_*.MuDst.root in/


# 1st iteration
root4star -b -q -l 'bfc.C( 100000, "in, Event, tpcDB, etofCalib, etofHit, etofQa, EvOut, cMuDst", "in/st_physics_19156044_raw_5500001.event.root" )'

rm StETofQA.hst.root
hadd StETofQA.hst.root st_physics_*.etofQA.root

root4star -b -q -l 'pl_HitQa.C(18,3,1)'
mv pl_HitQA_18_3_1.pdf pl_HitQA_18_3_1_Iter1.pdf
cp StETofCalib.hst.root StETofCalib_Iter1.hst.root


#2nd iteration
root4star -b -q -l 'bfc.C( 100000, "in, Event, tpcDB, etofCalib, etofHit, etofQa, EvOut, cMuDst", "in/st_physics_19156044_raw_5500001.event.root" )'

rm StETofQA.hst.root
hadd StETofQA.hst.root st_physics_*.etofQA.root

root4star -b -q -l 'pl_HitQa.C(18,3,1)'
mv pl_HitQA_18_3_1.pdf pl_HitQA_18_3_1_Iter2.pdf
cp StETofCalib.hst.root StETofCalib_Iter2.hst.root


#3rd iteration
root4star -b -q -l 'bfc.C( 100000, "in, Event, tpcDB, etofCalib, etofHit, etofQa, EvOut, cMuDst", "in/st_physics_19156044_raw_5500001.event.root" )'

rm StETofQA.hst.root
hadd StETofQA.hst.root st_physics_*.etofQA.root

root4star -b -q -l 'pl_HitQa.C(18,3,1)'
mv pl_HitQA_18_3_1.pdf pl_HitQA_18_3_1_Iter3.pdf
cp StETofCalib.hst.root StETofCalib_Iter3.hst.root


#4th iteration
root4star -b -q -l 'bfc.C( 100000, "in, Event, tpcDB, etofCalib, etofHit, etofQa, EvOut, cMuDst", "in/st_physics_19156044_raw_5500001.event.root" )'

rm StETofQA.hst.root
hadd StETofQA.hst.root st_physics_*.etofQA.root

root4star -b -q -l 'pl_HitQa.C(18,3,1)'
mv pl_HitQA_18_3_1.pdf pl_HitQA_18_3_1_Iter4.pdf
cp StETofCalib.hst.root StETofCalib_Iter4.hst.root


#5th iteration
root4star -b -q -l 'bfc.C( 100000, "in, Event, tpcDB, etofCalib, etofHit, etofQa, EvOut, cMuDst", "in/st_physics_19156044_raw_5500001.event.root" )'

rm StETofQA.hst.root
hadd StETofQA.hst.root st_physics_*.etofQA.root

root4star -b -q -l 'pl_HitQa.C(18,3,1)'
mv pl_HitQA_18_3_1.pdf pl_HitQA_18_3_1_Iter5.pdf
cp StETofCalib.hst.root StETofCalib_Iter5.hst.root


#6th iteration
root4star -b -q -l 'bfc.C( 100000, "in, Event, tpcDB, etofCalib, etofHit, etofQa, EvOut, cMuDst", "in/st_physics_19156044_raw_5500001.event.root" )'

rm StETofQA.hst.root
hadd StETofQA.hst.root st_physics_*.etofQA.root

root4star -b -q -l 'pl_HitQa.C(18,3,1)'
mv pl_HitQA_18_3_1.pdf pl_HitQA_18_3_1_Iter6.pdf
cp StETofCalib.hst.root StETofCalib_Iter6.hst.root


#7th iteration
root4star -b -q -l 'bfc.C( 100000, "in, Event, tpcDB, etofCalib, etofHit, etofQa, EvOut, cMuDst", "in/st_physics_19156044_raw_5500001.event.root" )'

rm StETofQA.hst.root
hadd StETofQA.hst.root st_physics_*.etofQA.root

root4star -b -q -l 'pl_HitQa.C(18,3,1)'
mv pl_HitQA_18_3_1.pdf pl_HitQA_18_3_1_Iter7.pdf
cp StETofCalib.hst.root StETofCalib_Iter7.hst.root



root4star -b -q -l 'pl_HitQa.C(18,1,1)'
root4star -b -q -l 'pl_HitQa.C(18,1,2)'
root4star -b -q -l 'pl_HitQa.C(18,1,3)'
root4star -b -q -l 'pl_HitQa.C(18,2,1)'
root4star -b -q -l 'pl_HitQa.C(18,2,2)'
root4star -b -q -l 'pl_HitQa.C(18,2,3)'
root4star -b -q -l 'pl_HitQa.C(18,3,1)'
root4star -b -q -l 'pl_HitQa.C(18,3,2)'
root4star -b -q -l 'pl_HitQa.C(18,3,3)'

