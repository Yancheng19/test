#!/bin/tcsh

#shell-script to submit condor jobs and combine output. Needed since condor jobs cannot resumbit themselves.

set iStep=1
set iFinalStep=22
set WORKDIR="/star/u/weidenkaff/StEtofHitMaker"
#set OUTDIR="/star/u/weidenkaff/StEtofHitMaker"
set OUTDIR="/star/data01/pwg/weidenkaff/"

#Clean up old files
if( $iStep == 1 ) then
	mv ${WORKDIR}/StETofCalib.hst.root ${WORKDIR}/StETofCalib.hst_old.root
	rm ${WORKDIR}/StETofCalib_Iter*.hst.root
	rm ${WORKDIR}/StETofQA_Iter*.hst.root
 	rm ${WORKDIR}/StETofQA.hst.root
else
	mv ${WORKDIR}/StETofCalib.hst.root ${WORKDIR}/StETofCalib.hst_old.root
	mv ${WORKDIR}/StETofCalib.hst_Iter_${iStep}.root ${WORKDIR}/StETofCalib.hst.root
endif

	rm ${WORKDIR}/CalibStatus.txt
	echo "Starting Calibration " `date` > ${WORKDIR}/CalibStatus.txt

	mkdir ${OUTDIR}/out/
	mkdir ${OUTDIR}/log/
	mkdir ${OUTDIR}/gen/
	rm -r ${OUTDIR}/gen/* ${OUTDIR}/out/* ${OUTDIR}/log/* ${OUTDIR}/sched* ${OUTDIR}/*.session.xml

	echo "Starting Calibration step $iStep " `date` >> ${WORKDIR}/CalibStatus.txt
	echo "Starting Calibration step $iStep"

	star-submit-template -p bnl_condor_dag -template ${WORKDIR}/ETOF-Calib_paralell.xml -entities iStep=${iStep},WORKDIR=$WORKDIR,OUTDIR=$OUTDIR

while ( $iFinalStep>$iStep )

	if( { grep -q "Finished calibration step ${iStep}" ${WORKDIR}/CalibStatus.txt } )then
			

			rm -r ${OUTDIR}/gen/* ${OUTDIR}/log/* ${OUTDIR}/sched* ${OUTDIR}/*.session.xml #keep disc clean.
		   root4star -b -q -l "${WORKDIR}/pl_HitQa.C(18,3,3)"
			mv ${WORKDIR}/pl_HitQA_18_3_3.pdf ${WORKDIR}/pl_HitQA_18_3_3_Iter_${iStep}.pdf			
			
			@ iStep += 1
			echo "Starting Calibration Step $iStep " `date` >> ${WORKDIR}/CalibStatus.txt
		 	star-submit-template -p bnl_condor_dag -template ${WORKDIR}/ETOF-Calib_paralell.xml -entities iStep=${iStep},WORKDIR=$WORKDIR,OUTDIR=$OUTDIR

	endif

  sleep 600

end

root4star -b -q -l "${WORKDIR}/pl_HitQa.C(18,3,3)"
mv ${WORKDIR}/pl_HitQA_18_3_3.pdf ${WORKDIR}/pl_HitQA_18_3_3_Iter_${iStep}.pdf	

echo "Calibration run finished after $iStep " `date` >> ${WORKDIR}/CalibStatus.txt
