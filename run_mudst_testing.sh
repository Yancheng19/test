#---------------------- testing CalibMaker --------------------
# create the digis inside StEvent & MuDsts
#root4star -b -q -l 'bfc.C( 100, "in, event, tpcDb, etofDat, evOut, cMuDst", "/star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500001.daq" )'

#mv st_physics_19156044_raw_5500001.event.root /star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofdat.event.root 
#mv st_physics_19156044_raw_5500001.MuDst.root /star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofdat.MuDst.root 

# run calib on digis in StEvent format as cross check
#root4star -b -q -l 'bfcRunnerCalib.C( 100, "in, etofCalib, evOut, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofdat.event.root" )'

# run calib on digis in MuDst format
#root4star -b -q -l 'bfc.C( 100, "in, etofCalib", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofdat.MuDst.root" )'

# -------------------------------------------------------------------------------------------------

#---------------------- testing HitMaker --------------------
# create the calibrated digis inside StEvent & MuDsts
#root4star -b -q -l 'bfc.C( 500, "in, event, tpcDb, btofDat, vpdCalib, etofDat, etofCalib, evOut, cMuDst", "/star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500001.daq" )'

#mv st_physics_19156044_raw_5500001.event.root /star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofcalib.event.root
#mv st_physics_19156044_raw_5500001.MuDst.root /star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofcalib.MuDst.root
 
# run hit maker in StEvent format as cross check (directly)
#root4star -b -q -l 'bfc.C( 50, "in, etofHit, evOut, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofcalib.event.root" )'
# run hit maker in StEvent format as cross check (reproduction)
#root4star -b -q -l 'bfc.C( 20, "in, etofHit, evOut, cMuDst", "in/st_physics_19156044_raw_etofcalib.event.root" )'

# run hit maker in MuDst format (directly)
#root4star -b -q -l 'bfc.C( 200, "in, etofHit, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofcalib.MuDst.root" )'
#root4star -b -q -l 'bfcRunnerHit.C( 200, "in, etofHit, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofcalib.MuDst.root" )'
# run hit maker in MuDst format (reproduction)
#root4star -b -q -l 'bfc.C( 20, "in, etofHit, cMuDst", "in/st_physics_19156044_raw_etofcalib.MuDst.root" )'
#--------------------------------------------------------------------------------------------------

#---------------------- testing MatchMaker --------------------
# using newly created event/mudst files with digis inside (produced with the calibration production chain options)
# run chain to create hits
#root4star -b -q -l 'bfc.C( 2000, "in, etofCalib, etofHit, evOut, cMuDst", "/star/data06/ETOF/reco-files/fastoffline/st_physics_19156044_raw_5500001.event.root" )'

#mv st_physics_19156044_raw_5500001.event.root /star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_track-etofhit.event.root
#mv st_physics_19156044_raw_5500001.MuDst.root /star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_track-etofhit.MuDst.root

# run match maker in StEvent format as cross check (directly)
#root4star -b -q -l 'bfc.C( 200, "in, AgML, ry2018a, etofMatch, evOut, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_track-etofhit.event.root" )'
# run match maker in StEvent format as cross check (reproduction)
#root4star -b -q -l 'bfc.C( 200, "in, AgML, ry2018a, etofMatch, evOut, cMuDst", "in/st_physics_19156044_raw_track-etofhit.event.root" )'

# run match maker in MuDst format (directly)
#root4star -b -q -l 'bfcRunnerMatch.C( 2000, "in, AgML, ry2018a, etofMatch", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_track-etofhit.MuDst.root" )'
# run match maker in MuDst format (reproduction)
#root4star -b -q -l 'bfcRunnerMatch.C( 2000, "in, AgML, ry2018a, etofMatch", "in/st_physics_19156044_raw_track-etofhit.MuDst.root" )'


#root4star -b -q -l 'bfc.C( 200, "in, etofHit, evOut, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_track-etofhit.event.root" )'
#root4star -b -q -l 'bfc.C( 200, "in, etofHit, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_track-etofhit.MuDst.root" )'


#root4star -b -q -l 'bfc.C( 2000, "in, ry2018a, AgML, etofCalib, etofHit, etofMatch, etofQA", "/star/data06/ETOF/muDstAna/st_mtd_19166008_raw_5500003.MuDst.root" )'
#root4star -b -q -l 'bfcRunnerCalib.C( 2000, "in, etofCalib", "/star/data06/ETOF/muDstAna/st_mtd_19166008_raw_5500003.MuDst.root" )'
#root4star -b -q -l 'bfcRunnerHit.C( 2000, "in, etofHit", "/star/data06/ETOF/muDstAna/st_mtd_19166008_raw_5500003.MuDst.root" )'
#root4star -b -q -l 'bfcRunnerMatch.C( 2000, "in, AgML, ry2018a, etofMatch", "/star/data06/ETOF/muDstAna/st_mtd_19166008_raw_5500003.MuDst.root" )'
root4star -b -q -l 'bfcRunner2018.C( 2000, "in, AgML, ry2018a, etofCalib, etofHit, etofMatch", "/star/data06/ETOF/muDstAna/st_mtd_19166008_raw_5500003.MuDst.root" )'

#--------------------------------------------------------------------------------------------------

#---------------------- testing QAMaker --------------------
# create the calibrated digis and hits inside StEvent & MuDsts
#root4star -b -q -l 'bfc.C( 2000, "in, AgML, y2018a, etofCalib, etofHit, etofMatch, etofQA ,evOut, cMuDst", "/star/data06/ETOF/reco-files/fastoffline/st_physics_19156044_raw_5500001.event.root" )'

#mv st_physics_19156044_raw_5500001.event.root /star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofmatch.event.root
#mv st_physics_19156044_raw_5500001.MuDst.root /star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofmatch.MuDst.root

# run QA maker in StEvent format as cross check
#root4star -b -q -l 'bfc.C( 2000, "in, etofQa, evOut, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofmatch.event.root" )'

# run QA maker in MuDst format
#root4star -b -q -l 'bfc.C( 2000, "in, etofQa", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofmatch.MuDst.root" )'

#root4star -b -q -l 'bfc.C( 1000, "in, etofQa", "/star/data06/ETOF/muDstAna/st_mtd_19156042_raw_4000003.MuDst.root" )'

#--------------------------------------------------------------------------------------------------


#root4star -b -q -l 'bfcRunnerCalib.C( 100, "in, etofCalib, etofHit, evOut, cMuDst", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_etofdat.event.root" )'


#root4star -b -q -l 'bfc.C( 100, "in, event, tpcDb, etofDat, evOut, cMuDst", "/star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500001.daq" )'
#root4star -b -q -l 'bfc.C( 100, "in, AgML, ry2018a, event, tpcDb, etofDat, etofCalib, etofHit, etofMatch, evOut, cMuDst", "/star/data06/ETOF/daq/2018/156/19156044/st_physics_19156044_raw_5500001.daq" )'
