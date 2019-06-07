# if there is no vertex -> picoDsts seem to be empty ...

# use newly created event/MuDst files with digis inside, produced with the calibration production chain options
#root4star -b -q -l 'bfc.C( 500, "in, etofCalib, etofHit, cMuDst, picoDst, picoVtxDefault, picoWrite", "/star/data06/ETOF/reco-files/fastoffline/st_physics_19156044_raw_5500001.event.root" )'

# workes without cMuDst when in the PicoDstMaker the pointer to the muDst dataset is fetched via 'GetInputDs("MuDst")'
root4star -b -q -l 'bfc.C( 2000, "in, AgML, ry2018a, etofMatch, picoDst, picoVtxDefault, picoWrite", "/star/data06/ETOF/reco-files/testing/st_physics_19156044_raw_track-etofhit.MuDst.root" )'
