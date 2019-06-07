#include "ETofAnalysisMaker.h"

#include <iostream>

#include "StMuDSTMaker/COMMON/StMuDstMaker.h"
#include "StMuDSTMaker/COMMON/StMuTrack.h"
#include "StMuDSTMaker/COMMON/StMuEvent.h"
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuETofHit.h"
#include "StMuDSTMaker/COMMON/StMuETofPidTraits.h"

#include "StETofUtil/StETofGeometry.h"

#include "StEvent/StETofHit.h"

#include "TH1.h"
#include "TH2.h"
#include "TFile.h"
#include "TObjArray.h"
#include "TList.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TMath.h"
#include "TProfile.h"
#include "TF1.h"

#include <algorithm>
#include <vector>
#include <cmath>

const Int_t uNbSectors      = 12;
const Int_t uNbZPlanes      =  3;
const Int_t uNbDetPerModule =  3;
const Int_t uNbStripsPerDet = 32;

// safety margins in cm in local x and y direction for track extrapolation to etof modules
//const double safetyMargins[ 2 ] = { 0., 0. };
const double safetyMargins[ 2 ] = { 2., 2. };

ETofAnalysisMaker::ETofAnalysisMaker( StMuDstMaker* maker )
 : StMaker( "ETofAnalysisMaker" )
{ // Initialize and/or zero all public/private data members here.
  mMuDstMaker      = maker;                // Pass MuDst pointer to DstAnlysisMaker Class member functions
  histogram_output = nullptr;              // Zero the Pointer to histogram output file
  mEventsProcessed = 0;                    // Zero the Number of Events processed by the maker 
  mHistogramOutputFileName = "";           // Histogram Output File Name will be set inside the "analysis".C macro
  mTrackCount = 0;
  mETofGeom = 0;
  hist = nullptr;
}


ETofAnalysisMaker::~ETofAnalysisMaker() 

{ // Destroy and/or zero out all public/private data members here.

}


Int_t ETofAnalysisMaker::Init( )

{ // Do once at the start of the analysis

  //Setting up storage
  mOverlapStripHit.clear();
  mOverlapStripTrack.clear();
  mOverlapStripHit.resize( uNbSectors ); //number of Sectors
  mOverlapStripTrack.resize( uNbSectors );

  for( Int_t iSector = 0; iSector < uNbSectors; iSector++ ) { 
        mOverlapStripHit[iSector].resize( uNbZPlanes ); //number of ZPlanes
	mOverlapStripTrack[iSector].resize( uNbZPlanes );
        
        for( Int_t iPlane = 0; iPlane < uNbZPlanes; iPlane++ ) {
            mOverlapStripHit[iSector][iPlane].resize( uNbDetPerModule ); //number of Detectors
        }
  }

  // Create Histogram output file
  histogram_output = new TFile( mHistogramOutputFileName, "RECREATE" ) ;  // Name was set in "analysis".C macro

  // Create Histograms
  hist = new TH1F( "Vertex", "Event Vertex Z Position", 100, -25.0, 25.0 ) ; 

  refPositionZplane1 = new TH2F( "refPositionZplane1", "Overlapped hit in the reference counter in ZPlane 1", 34, -16, 18, 100, -100, 100 ) ; 
  dutPositionZplane1 = new TH2F( "dutPositionZplane1", "Same overlapped hit in the dut counter in ZPlane 1", 34, -16, 18, 100, -100, 100 ) ; 
  refPositionZplane2 = new TH2F( "refPositionZplane2", "Overlapped hit in the reference counter in ZPlane 2", 34, -16, 18, 100, -100, 100 ) ; 
  dutPositionZplane2 = new TH2F( "dutPositionZplane2", "Same overlapped hit in the dut counter in ZPlane 2", 34, -16, 18, 100, -100, 100 ) ;
  refPositionZplane3 = new TH2F( "refPositionZplane3", "Overlapped hit in the reference counter in ZPlane 3", 34, -16, 18, 100, -100, 100 ) ; 
  dutPositionZplane3 = new TH2F( "dutPositionZplane3", "Same overlapped hit in the dut counter in ZPlane 3", 34, -16, 18, 100, -100, 100 ) ;

  timeDiff1 = new TH1F( "timeDiff1", "Raw Time Difference in ZPlane 1", 100, -3, 3 ) ;
  timeDiff2 = new TH1F( "timeDiff2", "Raw Time Difference in ZPlane 2", 100, -3, 3 ) ;
  timeDiff3 = new TH1F( "timeDiff3", "Raw Time Difference in ZPlane 3", 100, -3, 3 ) ;

  xDiff1 = new TH1F( "xDiff1", "Raw X Difference in ZPlane 1", 50, -50, 0 ) ;
  xDiff2 = new TH1F( "xDiff2", "Raw X Difference in ZPlane 2", 50, -50, 0 ) ;
  xDiff3 = new TH1F( "xDiff3", "Raw X Difference in ZPlane 3", 50, -50, 0 ) ;

  yDiff1 = new TH1F( "yDiff1", "Raw Y Difference in ZPlane 1", 100, -50, 50 ) ;
  yDiff2 = new TH1F( "yDiff2", "Raw Y Difference in ZPlane 2", 100, -50, 50 ) ;
  yDiff3 = new TH1F( "yDiff3", "Raw Y Difference in ZPlane 3", 100, -50, 50 ) ;

  posDiff1 = new TH2F( "posDiff1", "dXdY in ZPlane 1", 8, -32, -24, 100, -50, 50 ) ;
  posDiff2 = new TH2F( "posDiff2", "dXdY in ZPlane 2", 8, -32, -24, 100, -50, 50 ) ;
  posDiff3 = new TH2F( "posDiff3", "RdXdY in ZPlane 3", 8, -32, -24, 100, -50, 50 ) ;

  chi2Distribution1 = new TH1F( "chi2Distribution1", "Chi2 distribution in ZPlane 1", 10, 0, 10 ) ;
  chi2Distribution2 = new TH1F( "chi2Distribution2", "Chi2 distribution in ZPlane 2", 10, 0, 10 ) ;
  chi2Distribution3 = new TH1F( "chi2Distribution3", "Chi2 distribution in ZPlane 3", 10, 0, 10 ) ;

  timeDiffMathced1 = new TH1F( "timeDiffMathced1", "Time Difference with Chi2 cut in ZPlane 1", 100, -3, 3 ) ;
  timeDiffMathced2 = new TH1F( "timeDiffMathced2", "Time Difference with Chi2 cut in ZPlane 2", 100, -3, 3 ) ;
  timeDiffMathced3 = new TH1F( "timeDiffMathced3", "Time Difference with Chi2 cut in ZPlane 3", 100, -3, 3 ) ;

  yDiffFit1 = new TF1( "yDiffFit1", "gaus");
  yDiffFit2 = new TF1( "yDiffFit2", "gaus");
  yDiffFit3 = new TF1( "yDiffFit3", "gaus");

  timeRawDiffFit1 = new TF1( "timeRawDiffFit1", "gaus");
  timeRawDiffFit2 = new TF1( "timeRawDiffFit2", "gaus");
  timeRawDiffFit3 = new TF1( "timeRawDiffFit3", "gaus");

  timeDiffFit1 = new TF1( "timeDiffFit1", "gaus");
  timeDiffFit2 = new TF1( "timeDiffFit2", "gaus");
  timeDiffFit3 = new TF1( "timeDiffFit3", "gaus");

  trackPosDut1 = new TH2F( "trackPosDut1", "Track positon in the dut counter in ZPlane 1", 36, -17, 19, 100, -100, 100 ) ;
  trackPosRef1 = new TH2F( "trackPosRef1", "Track positon in the ref counter in ZPlane 1", 36, -17, 19, 100, -100, 100 ) ;
  trackPosDut2 = new TH2F( "trackPosDut1", "Track positon in the dut counter in ZPlane 2", 36, -17, 19, 100, -100, 100 ) ;
  trackPosRef2 = new TH2F( "trackPosRef1", "Track positon in the ref counter in ZPlane 2", 36, -17, 19, 100, -100, 100 ) ;
  trackPosDut3 = new TH2F( "trackPosDut1", "Track positon in the dut counter in ZPlane 3", 36, -17, 19, 100, -100, 100 ) ;
  trackPosRef3 = new TH2F( "trackPosRef1", "Track positon in the ref counter in ZPlane 3", 36, -17, 19, 100, -100, 100 ) ;

  trackCounter = new TH1F( "trackCounter", "trackCounter", 5, 0, 5 ) ;
  trackZPlane = new TH1F( "trackZPlane", "trackZPlane", 5, 0, 5 ) ;
  trackX = new TH1F( "trackX", "trackX", 70, -35, 35 ) ;
  trackY = new TH1F( "trackY", "trackY", 70, -35, 35 ) ;

  return kStOK ;

}

//---------------------------------------------------------------------------
Int_t ETofAnalysisMaker::InitRun( Int_t runnumber )
{
    LOG_INFO << "ETofAnalysisMaker::InitRun()" << endm;

    if( !mETofGeom ) {
        LOG_INFO << " creating a new eTOF geometry . . . " << endm;
        mETofGeom = new StETofGeometry( "etofGeometry", "etofGeometry in MatchMaker" );
    }

    if( mETofGeom && !mETofGeom->isInitDone() ) {
        LOG_INFO << " eTOF geometry initialization ... " << endm;

        if( !gGeoManager ) GetDataBase( "VmcGeometry" );

        if ( !gGeoManager ) {
            LOG_ERROR << "Cannot get GeoManager" << endm;
            return kStErr;
        }

        //LOG_INFO << " gGeoManager: " << gGeoManager << endm;

        mETofGeom->init( gGeoManager, safetyMargins );
    }

    return kStOK;
}


//---------------------------------------------------------------------------
Int_t ETofAnalysisMaker::FinishRun( Int_t runnumber )
{
    LOG_INFO << "StETofAnalysisMaker::FinishRun() -- cleaning up the geometry" << endm;
    if( mETofGeom ) mETofGeom->reset();

    return kStOK;
}





Int_t ETofAnalysisMaker::Make( )
 
{ // Do each event

  // Get 'event' data 
  StMuEvent* muEvent = mMuDstMaker->muDst()->event();

  // Do 'event' analysis based on event data 
  hist->Fill( muEvent->primaryVertexPosition().z() ); // Make histogram of the vertex Z distribution


  // Get 'track' data
  //TObjArray* tracks = mMuDstMaker->muDst()->primaryTracks();     // Create a TObject array containing the primary tracks
  //TObjArrayIter GetTracks(tracks);                               // Create an iterator to step through the tracks

  //StMuTrack* track;                                              // Pointer to a track
  //while ( ( track = (StMuTrack*)GetTracks.Next() ) ) {           // Main loop for Iterating over tracks
	/// do your analysis on tracks
	//LOG_INFO << "Hit number on the track:" << track->nHits() << endm;
        
  //}

  size_t nNodes = mMuDst->numberOfGlobalTracks();
  //LOG_INFO << "nNodes: " << nNodes << endm;
  for( size_t iNode=0; iNode<nNodes; iNode++ ) {
    	StMuTrack* track = mMuDst->globalTracks( iNode );
      	if( !track ) continue;
       	if( track->index2ETofHit() < 0 ) continue;

	StMuETofPidTraits etofTraits = track->etofPidTraits();
	if( etofTraits.matchFlag() == 2 ){

		//To get the track information
		trackCounter->Fill( mMuDst->etofHit( track->index2ETofHit() )->counter());
		trackZPlane->Fill( mMuDst->etofHit( track->index2ETofHit() )->zPlane());
		trackX->Fill( mMuDst->etofHit( track->index2ETofHit() )->localX());
		trackY->Fill( mMuDst->etofHit( track->index2ETofHit() )->localY());
		
		LOG_INFO << "*******************************************matchFlag: " << etofTraits.matchFlag() << endm;
		
		if( (mMuDst->etofHit( track->index2ETofHit() )->zPlane()==1) && (mMuDst->etofHit( track->index2ETofHit() )->counter()==2) ){
			trackPosDut1->Fill( mMuDst->etofHit( track->index2ETofHit() )->localX(),mMuDst->etofHit( track->index2ETofHit() )->localY());
		}
		if( (mMuDst->etofHit( track->index2ETofHit() )->zPlane()==1) && (mMuDst->etofHit( track->index2ETofHit() )->counter()==3) ){
			trackPosRef1->Fill( mMuDst->etofHit( track->index2ETofHit() )->localX(),mMuDst->etofHit( track->index2ETofHit() )->localY());
		}
		if( (mMuDst->etofHit( track->index2ETofHit() )->zPlane()==2) && (mMuDst->etofHit( track->index2ETofHit() )->counter()==2) ){
			trackPosDut2->Fill( mMuDst->etofHit( track->index2ETofHit() )->localX(),mMuDst->etofHit( track->index2ETofHit() )->localY());
		}
		if( (mMuDst->etofHit( track->index2ETofHit() )->zPlane()==2) && (mMuDst->etofHit( track->index2ETofHit() )->counter()==3) ){
			trackPosRef2->Fill( mMuDst->etofHit( track->index2ETofHit() )->localX(),mMuDst->etofHit( track->index2ETofHit() )->localY());
		}
		if( (mMuDst->etofHit( track->index2ETofHit() )->zPlane()==3) && (mMuDst->etofHit( track->index2ETofHit() )->counter()==2) ){
			trackPosDut3->Fill( mMuDst->etofHit( track->index2ETofHit() )->localX(),mMuDst->etofHit( track->index2ETofHit() )->localY());
		}
		if( (mMuDst->etofHit( track->index2ETofHit() )->zPlane()==3) && (mMuDst->etofHit( track->index2ETofHit() )->counter()==3) ){
			trackPosRef3->Fill( mMuDst->etofHit( track->index2ETofHit() )->localX(),mMuDst->etofHit( track->index2ETofHit() )->localY());
		}

		mTrackCount ++;
		LOG_INFO << "___________________________trackCount: "<< mTrackCount << endm;

		StPhysicalHelixD theHelix = track->helix();

		int moduleId  = mETofGeom->calcModuleIndex( 18,  1 );
		int counterId  = 2;

		double pathlength, theta;
		StThreeVectorD cross;

		StETofGeomCounter* detUnderTest = (StETofGeomCounter*) mETofGeom->findETofNode( moduleId, counterId );

		bool helixCrossedCounter = detUnderTest->helixCross( theHelix, pathlength, cross, theta );

		if( helixCrossedCounter ) {
  			double global[ 3 ];
  			double local [ 3 ];

  			global[ 0 ] = cross.x();
  			global[ 1 ] = cross.y();
  			global[ 2 ] = cross.z();

  			detUnderTest->master2Local( global, local );

			LOG_INFO << "___________________________X: "<< local[0] << "Y: " << local[1] << endm;
		}


	}

  }
  




  /*

  // Get 'ETOF' data
  TObjArray* etofHits = mMuDstMaker->muDst()->etofArray( muETofHit ); 
  TObjArrayIter GetETofHits( etofHits );

  StMuETofHit* hit;




  while( ( hit = (StMuETofHit*)GetETofHits.Next() ) ) {
	//LOG_INFO << "ETOF HIT: sector=" << hit->sector() << " zplane=" << hit->zPlane() << " counter=" << hit->counter() << " strip=" << hit->localX() + 16 << " time=" << hit->time() << "  local ( x , y )=( " << hit->localX() << " , " << hit->localY() << " )" << endm;
  	//if( (hit->zPlane()==1) && (hit->counter()==2) && (hit->localX()>=-15) && (hit->localX()<-13) ){
	if( (hit->zPlane()==1) && (hit->counter()==2) && (hit->localX()>=-16) && (hit->localX()<-14) && (hit->associatedTrackId()==2) ){
  		//LOG_INFO << "ETOF HIT: sector=" << hit->sector() << " zplane=" << hit->zPlane() << " counter=" << hit->counter() << " strip=" << hit->localX() + 16 << " time=" << hit->time() << "  local ( x , y )=( " << hit->localX() << " , " << hit->localY() << " )" << endm;
		LOG_INFO << "Matched track ID =" << hit->associatedTrackId() << endm;
		mOverlapStripHit[5][0][1].push_back( hit );
	}
	if( (hit->zPlane()==1) && (hit->counter()==1) && (hit->localX()>11) && (hit->localX()<=17) && (hit->associatedTrackId()==0) ){
  		//LOG_INFO << "ETOF HIT: sector=" << hit->sector() << " zplane=" << hit->zPlane() << " counter=" << hit->counter() << " strip=" << hit->localX() + 16 << " time=" << hit->time() << "  local ( x , y )=( " << hit->localX() << " , " << hit->localY() << " )" << endm;
		LOG_INFO << "Matched track ID =" << hit->associatedTrackId() << endm;
                LOG_INFO << "Hits matched on the same track in module 1" << endm;
		mOverlapStripHit[5][0][0].push_back( hit );
	}
	if( (hit->zPlane()==2) && (hit->counter()==2) && (hit->localX()>=-15) && (hit->localX()<-13) ){
		//LOG_INFO << "Matched track ID =" << hit->associatedTrackId() << endm;
		mOverlapStripHit[5][1][1].push_back( hit );
	}
	if( (hit->zPlane()==2) && (hit->counter()==1) && (hit->localX()>11) && (hit->localX()<=17) ){
		//LOG_INFO << "Matched track ID =" << hit->associatedTrackId() << endm;
		mOverlapStripHit[5][1][0].push_back( hit );
	}
	if( (hit->zPlane()==3) && (hit->counter()==2) && (hit->localX()>=-15) && (hit->localX()<-13) ){
		//LOG_INFO << "Matched track ID =" << hit->associatedTrackId() << endm;
		mOverlapStripHit[5][2][1].push_back( hit );
	}
	if( (hit->zPlane()==3) && (hit->counter()==1) && (hit->localX()>11) && (hit->localX()<=17) ){
		//LOG_INFO << "Matched track ID =" << hit->associatedTrackId() << endm;
		mOverlapStripHit[5][2][0].push_back( hit );
	}
  }

  Int_t dutHitSize1 = mOverlapStripHit[5][0][0].size();
  Int_t refHitSize1 = mOverlapStripHit[5][0][1].size();
  Int_t dutHitSize2 = mOverlapStripHit[5][1][0].size();
  Int_t refHitSize2 = mOverlapStripHit[5][1][1].size();
  Int_t dutHitSize3 = mOverlapStripHit[5][2][0].size();
  Int_t refHitSize3 = mOverlapStripHit[5][2][1].size();
	  
  Double_t chi2Match1;				//Chi2 cut
  Double_t chi2Match2;
  Double_t chi2Match3;

  if ( refHitSize1 > 0 ){
	for( int i=0; i<refHitSize1; i++ ) {
		refPositionZplane1->Fill(mOverlapStripHit[5][0][1].at(i)->localX(), mOverlapStripHit[5][0][1].at(i)->localY());
	}
  }

  if ( (refHitSize1 > 0) && (dutHitSize1 > 0) ){
	for( int i=0; i<dutHitSize1; i++ ) {
		dutPositionZplane1->Fill(mOverlapStripHit[5][0][0].at(i)->localX(), mOverlapStripHit[5][0][0].at(i)->localY());
		for( int j=0; j<refHitSize1; j++ ) {
			timeDiff1->Fill(mOverlapStripHit[5][0][1].at(j)->time() - mOverlapStripHit[5][0][0].at(i)->time());
			xDiff1->Fill(mOverlapStripHit[5][0][1].at(j)->localX() - mOverlapStripHit[5][0][0].at(i)->localX());
			yDiff1->Fill(mOverlapStripHit[5][0][1].at(j)->localY() - mOverlapStripHit[5][0][0].at(i)->localY());
			posDiff1->Fill(mOverlapStripHit[5][0][1].at(j)->localX() - mOverlapStripHit[5][0][0].at(i)->localX(), mOverlapStripHit[5][0][1].at(j)->localY() - mOverlapStripHit[5][0][0].at(i)->localY());
			chi2Match1 = TMath::Power((mOverlapStripHit[5][0][1].at(j)->localX() - mOverlapStripHit[5][0][0].at(i)->localX() + 28.08)/1, 2)
	                           + TMath::Power((mOverlapStripHit[5][0][1].at(j)->localY() - mOverlapStripHit[5][0][0].at(i)->localY() - 0.6788)/1, 2)
	                           + TMath::Power((mOverlapStripHit[5][0][1].at(j)->time() - mOverlapStripHit[5][0][0].at(i)->time() - 0.3532)/0.2, 2);
			chi2Match1 /= 3;
			//cout<<"The chi2Match1 is: "<<chi2Match1<<endl;
			chi2Distribution1->Fill(chi2Match1);
			if (chi2Match1 < 2){
				timeDiffMathced1->Fill(mOverlapStripHit[5][0][1].at(j)->time() - mOverlapStripHit[5][0][0].at(i)->time());
			}
		}
	}
  }

  if ( refHitSize2 > 0 ){
	for( int i=0; i<refHitSize2; i++ ) {
		refPositionZplane2->Fill(mOverlapStripHit[5][1][1].at(i)->localX(), mOverlapStripHit[5][1][1].at(i)->localY());
	}
  }

  if ( (refHitSize2 > 0) && (dutHitSize2 > 0) ){
	for( int i=0; i<dutHitSize2; i++ ) {
		dutPositionZplane2->Fill(mOverlapStripHit[5][1][0].at(i)->localX(), mOverlapStripHit[5][1][0].at(i)->localY());
		for( int j=0; j<refHitSize2; j++ ) {
			timeDiff2->Fill(mOverlapStripHit[5][1][1].at(j)->time() - mOverlapStripHit[5][1][0].at(i)->time());
			xDiff2->Fill(mOverlapStripHit[5][1][1].at(j)->localX() - mOverlapStripHit[5][1][0].at(i)->localX());
			yDiff2->Fill(mOverlapStripHit[5][1][1].at(j)->localY() - mOverlapStripHit[5][1][0].at(i)->localY());
			posDiff2->Fill(mOverlapStripHit[5][1][1].at(j)->localX() - mOverlapStripHit[5][1][0].at(i)->localX(), mOverlapStripHit[5][1][1].at(j)->localY() - mOverlapStripHit[5][1][0].at(i)->localY());
			chi2Match2 = TMath::Power((mOverlapStripHit[5][1][1].at(j)->localX() - mOverlapStripHit[5][1][0].at(i)->localX() + 28.06)/1, 2)
	                           + TMath::Power((mOverlapStripHit[5][1][1].at(j)->localY() - mOverlapStripHit[5][1][0].at(i)->localY() + 1.885)/1, 2)
	                           + TMath::Power((mOverlapStripHit[5][1][1].at(j)->time() - mOverlapStripHit[5][1][0].at(i)->time() - 0.1576)/0.2, 2);
			chi2Match2 /= 3;
			chi2Distribution2->Fill(chi2Match2);
			if (chi2Match2 < 2){
				timeDiffMathced2->Fill(mOverlapStripHit[5][1][1].at(j)->time() - mOverlapStripHit[5][1][0].at(i)->time());
			}
		}
	}
  }

  if ( refHitSize3 > 0 ){
	for( int i=0; i<refHitSize3; i++ ) {
		refPositionZplane3->Fill(mOverlapStripHit[5][2][1].at(i)->localX(), mOverlapStripHit[5][2][1].at(i)->localY());
	}
  }

  if ( (refHitSize3 > 0) && (dutHitSize3 > 0) ){
	for( int i=0; i<dutHitSize3; i++ ) {
		dutPositionZplane3->Fill(mOverlapStripHit[5][2][0].at(i)->localX(), mOverlapStripHit[5][2][0].at(i)->localY());
		for( int j=0; j<refHitSize3; j++ ) {
			timeDiff3->Fill(mOverlapStripHit[5][2][1].at(j)->time() - mOverlapStripHit[5][2][0].at(i)->time());
			xDiff3->Fill(mOverlapStripHit[5][2][1].at(j)->localX() - mOverlapStripHit[5][2][0].at(i)->localX());
			yDiff3->Fill(mOverlapStripHit[5][2][1].at(j)->localY() - mOverlapStripHit[5][2][0].at(i)->localY());
			posDiff3->Fill(mOverlapStripHit[5][2][1].at(j)->localX() - mOverlapStripHit[5][2][0].at(i)->localX(), mOverlapStripHit[5][2][1].at(j)->localY() - mOverlapStripHit[5][2][0].at(i)->localY());
			chi2Match3 = TMath::Power((mOverlapStripHit[5][2][1].at(j)->localX() - mOverlapStripHit[5][2][0].at(i)->localX() + 28.11)/1, 2)
	                           + TMath::Power((mOverlapStripHit[5][2][1].at(j)->localY() - mOverlapStripHit[5][2][0].at(i)->localY() + 1.309)/1, 2)
	                           + TMath::Power((mOverlapStripHit[5][2][1].at(j)->time() - mOverlapStripHit[5][2][0].at(i)->time() - 0.2854)/0.2, 2);
			chi2Match3 /= 3;
			chi2Distribution3->Fill(chi2Match3);
			if (chi2Match3 < 2){
				timeDiffMathced3->Fill(mOverlapStripHit[5][2][1].at(j)->time() - mOverlapStripHit[5][2][0].at(i)->time());
			}
		}
	}
  }
  */

  //Clear storage
  mOverlapStripHit.clear();
  mOverlapStripHit.resize( uNbSectors ); //number of Sectors

  for( Int_t iSector = 0; iSector < uNbSectors; iSector++ ) { 
        mOverlapStripHit[iSector].resize( uNbZPlanes ); //number of ZPlanes
        
        for( Int_t iPlane = 0; iPlane < uNbZPlanes; iPlane++ ) {
            mOverlapStripHit[iSector][iPlane].resize( uNbDetPerModule ); //number of Detectors
        }
  }

  mEventsProcessed++ ;
  return kStOK ;
  
}


Int_t ETofAnalysisMaker::Finish( )

{ // Do once at the end of the analysis

  // Write histograms to disk, output miscellaneous other information

  histogram_output -> Write() ;   // Write all histograms to disk 

  TCanvas* can1 = new TCanvas( "canvas all" );
  can1->Divide(2,1);
  can1->cd(1);
  refPositionZplane1->Draw("colz");
  can1->cd(2);
  dutPositionZplane1->Draw("colz");
  can1->Print( "eTof_plots_overlapping.pdf(" );
  can1->cd(1);
  refPositionZplane2->Draw("colz");
  can1->cd(2);
  dutPositionZplane2->Draw("colz");
  can1->Print( "eTof_plots_overlapping.pdf" );
  can1->cd(1);
  refPositionZplane3->Draw("colz");
  can1->cd(2);
  dutPositionZplane3->Draw("colz");
  can1->Print( "eTof_plots_overlapping.pdf" );

  TCanvas* can2 = new TCanvas( "canvas all" );
  can2->Divide(2,2);
  
  can2->cd(1);
  xDiff1->Draw();
  can2->cd(2);
  xDiff2->Draw();
  can2->cd(3);
  xDiff3->Draw();
  can2->Print( "eTof_plots_overlapping.pdf" );

  can2->cd(1);
  double fitpardy[3];
  yDiffFit1->SetRange(yDiff1->GetMean() - 1 * yDiff1->GetRMS(), yDiff1->GetMean() + 1 * yDiff1->GetRMS());
  yDiff1->Fit("yDiffFit1","QR");
  yDiffFit1->GetParameters(&fitpardy[0]);
  yDiffFit1->SetRange(fitpardy[1] - 1 * fitpardy[2], fitpardy[1] + 1 * fitpardy[2]);
  yDiff1->Fit("yDiffFit1","QR");
  yDiff1->Draw();
  can2->cd(2);
  yDiffFit2->SetRange(yDiff2->GetMean() - 1 * yDiff2->GetRMS(), yDiff2->GetMean() + 1 * yDiff2->GetRMS());
  yDiff2->Fit("yDiffFit2","QR");
  yDiffFit2->GetParameters(&fitpardy[0]);
  yDiffFit2->SetRange(fitpardy[1] - 1 * fitpardy[2], fitpardy[1] + 1 * fitpardy[2]);
  yDiff2->Fit("yDiffFit2","QR");
  yDiff2->Draw();
  can2->cd(3);
  yDiffFit3->SetRange(yDiff3->GetMean() - 1 * yDiff3->GetRMS(), yDiff3->GetMean() + 1 * yDiff3->GetRMS());
  yDiff3->Fit("yDiffFit3","QR");
  yDiffFit3->GetParameters(&fitpardy[0]);
  yDiffFit3->SetRange(fitpardy[1] - 1 * fitpardy[2], fitpardy[1] + 1 * fitpardy[2]);
  yDiff3->Fit("yDiffFit3","QR");
  yDiff3->Draw();
  can2->Print( "eTof_plots_overlapping.pdf" );

  can2->cd(1);
  double fitpardt[3];
  timeRawDiffFit1->SetRange(timeDiff1->GetMean() - 1 * timeDiff1->GetRMS(), timeDiff1->GetMean() + 1 * timeDiff1->GetRMS());
  timeDiff1->Fit("timeRawDiffFit1","QR");
  timeRawDiffFit1->GetParameters(&fitpardt[0]);
  timeRawDiffFit1->SetRange(fitpardt[1] - 1 * fitpardt[2], fitpardt[1] + 1 * fitpardt[2]);
  timeDiff1->Fit("timeRawDiffFit1","QR");
  timeDiff1->Draw();
  can2->cd(2);
  timeRawDiffFit2->SetRange(timeDiff2->GetMean() - 1 * timeDiff2->GetRMS(), timeDiff2->GetMean() + 1 * timeDiff2->GetRMS());
  timeDiff2->Fit("timeRawDiffFit2","QR");
  timeRawDiffFit2->GetParameters(&fitpardt[0]);
  timeRawDiffFit2->SetRange(fitpardt[1] - 1 * fitpardt[2], fitpardt[1] + 1 * fitpardt[2]);
  timeDiff2->Fit("timeRawDiffFit2","QR");
  timeDiff2->Draw();
  can2->cd(3);
  timeRawDiffFit3->SetRange(timeDiff3->GetMean() - 1 * timeDiff3->GetRMS(), timeDiff3->GetMean() + 1 * timeDiff3->GetRMS());
  timeDiff3->Fit("timeRawDiffFit3","QR");
  timeRawDiffFit3->GetParameters(&fitpardt[0]);
  timeRawDiffFit3->SetRange(fitpardt[1] - 1 * fitpardt[2], fitpardt[1] + 1 * fitpardt[2]);
  timeDiff3->Fit("timeRawDiffFit3","QR");
  timeDiff3->Draw();
  can2->Print( "eTof_plots_overlapping.pdf" );

  can2->cd(1);
  posDiff1->Draw("colz");
  gPad->SetLogz(1);
  can2->cd(2);
  posDiff2->Draw("colz");
  gPad->SetLogz(1);
  can2->cd(3);
  posDiff3->Draw("colz");
  gPad->SetLogz(1);
  can2->Print( "eTof_plots_overlapping.pdf" );

  can2->cd(1);
  chi2Distribution1->Draw();
  can2->cd(2);
  chi2Distribution2->Draw();
  can2->cd(3);
  chi2Distribution3->Draw();
  can2->Print( "eTof_plots_overlapping.pdf" );

  can2->cd(1);
  //gStyle->SetOptFit(111);
  //gStyle->SetOptStat(1111);
  timeDiffFit1->SetRange(timeDiffMathced1->GetMean() - 3 * timeDiffMathced1->GetRMS(), timeDiffMathced1->GetMean() + 3 * timeDiffMathced1->GetRMS());
  timeDiffMathced1->Fit("timeDiffFit1","QR");
  timeDiffMathced1->Draw();
  can2->cd(2);
  timeDiffFit2->SetRange(timeDiffMathced2->GetMean() - 3 * timeDiffMathced2->GetRMS(), timeDiffMathced2->GetMean() + 3 * timeDiffMathced2->GetRMS());
  timeDiffMathced2->Fit("timeDiffFit2","QR");
  timeDiffMathced2->Draw();
  can2->cd(3);
  timeDiffFit3->SetRange(timeDiffMathced3->GetMean() - 3 * timeDiffMathced3->GetRMS(), timeDiffMathced3->GetMean() + 3 * timeDiffMathced3->GetRMS());
  timeDiffMathced3->Fit("timeDiffFit3","QR");
  timeDiffMathced3->Draw();
  can2->Print( "eTof_plots_overlapping.pdf" );

  can2->cd(1);
  trackCounter->Draw();
  can2->cd(2);
  trackZPlane->Draw();
  can2->cd(3);
  trackX->Draw();
  can2->cd(4);
  trackY->Draw();
  can2->Print( "eTof_plots_overlapping.pdf" );



  TCanvas* can3 = new TCanvas( "canvas all" );
  can3->Divide(2,3);

  can3->cd(1);
  trackPosDut1->Draw("colz");
  can3->cd(2);
  trackPosRef1->Draw("colz");
  can3->cd(3);
  trackPosDut2->Draw("colz");
  can3->cd(4);
  trackPosRef2->Draw("colz");
  can3->cd(5);
  trackPosDut3->Draw("colz");
  can3->cd(6);
  trackPosRef3->Draw("colz");
  can3->Print( "eTof_plots_overlapping.pdf)" );

  cout << "Total Events Processed in DstMaker " << mEventsProcessed << endl ;



    if( mETofGeom ) delete mETofGeom;
    mETofGeom = 0;

  return kStOK ;

}









