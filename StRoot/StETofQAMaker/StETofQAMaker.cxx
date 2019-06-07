/***************************************************************************
 *
 * $Id: StETofQAMaker.h,v 1.1 2018/07/25 14:38:06 jeromel Exp $
 *
 * Author: Philipp Weidenkaff & Pengfei Lyu, May 2018
 ***************************************************************************
 *
 * Description: StETofQAMaker - class to read the eTofCollection from
 * StEvent build QA histograms. 
 *
 ***************************************************************************
 *
 * $Log: StETofQAMaker.h,v $
 * Revision 1.1  2018/07/25 14:38:06  jeromel
 * Peer reviewed Raghav+Jerome - code from Florian Seck
 *
 *
 ***************************************************************************/
#include <vector>
#include <algorithm>
#include <cmath>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

//#include "StParticleTypes.hh"
//#include "StParticleDefinition.hh"
#include "SystemOfUnits.h"
#include "PhysicalConstants.h"

#include "StEvent.h"
#include "StETofCollection.h"
#include "StETofHeader.h"
#include "StETofDigi.h"
#include "StETofHit.h"
#include "StETofPidTraits.h"
#include "StBTofCollection.h"
#include "StBTofHeader.h"
#include "StBTofHit.h"

#include "StTrackNode.h"
#include "StTrackGeometry.h"
#include "StGlobalTrack.h"
#include "StPrimaryTrack.h"
#include "StTrackPidTraits.h"
#include "StPrimaryVertex.h"

#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuTrack.h"
#include "StMuDSTMaker/COMMON/StMuETofHeader.h"
#include "StMuDSTMaker/COMMON/StMuETofDigi.h"
#include "StMuDSTMaker/COMMON/StMuETofHit.h"
#include "StMuDSTMaker/COMMON/StMuETofPidTraits.h"
#include "StMuDSTMaker/COMMON/StMuBTofHit.h"
#include "StMuDSTMaker/COMMON/StMuPrimaryVertex.h"

#include "StChain/StChainOpt.h" // for renaming the histogram file

#include "StETofQAMaker.h"

#include "StETofUtil/StETofConstants.h"
#include "StETofUtil/etofHelperFunctions.h"

#include "tables/St_etofSignalVelocity_Table.h"


const int iCalibLoop[ 22 ] = { 1, 1, 3 ,1, 3, 1, 2, 1, 2, 1,
                               2, 1, 2, 1, 2, 1, 2, 1, 2, 1,
                               1, 0
                             };

//const int iCalibLoop[ 1 ] = { 0 };

//_____________________________________________________________
StETofQAMaker::StETofQAMaker( const char* name )
: StMaker( "etofQa", name ),
  mEvent( 0 ),              /// pointer to StEvent
  mMuDst( 0 ),              /// pointer to MuDst
  mIsStEvent( false ),
  mTimeOffset( 0 ),
  mTstart( 0 ),
  mNCalibDigis( 0 ),
  mNHitsInTrgWindow( 0 ),
  mAverageETofTime( 0 ),
  mQAHistFileName( "" ),
  mCalibInFile( 0 ),
  mCalibOutFile( 0 ),
  mCalibInHistFileName( "StETofCalib.hst.root" ),
  mCalibOutHistFileName( "" ),
  mCalibStep( 0 )
{
    LOG_DEBUG << "StETofQAMaker::ctor"  << endm;

    mMapRpcCluMult.clear();

    mHistograms1d.clear();
    mHistograms2d.clear();
}

//_____________________________________________________________
StETofQAMaker::~StETofQAMaker()
{ /* no op */ 

}

//_____________________________________________________________
Int_t
StETofQAMaker::Init()
{
    LOG_INFO << "StETofQAMaker::Init" << endm;

    initCalib(); //execute first to get mCalibStep available in ::BookHistograms

    bookHistograms();

    return kStOk;
}

//_____________________________________________________________
Int_t
StETofQAMaker::InitRun( Int_t runnumber )
{
	
//--------------------------------------------------------------------------------------------
    TDataSet* dbDataSet = nullptr;    
    std::ifstream paramFile;
    
    // signal velocities
    mSignalVelocity.clear();

    if( mFileNameSignalVelocity.empty() ) {
        LOG_INFO << "etofSignalVelocity: no filename provided --> load database table" << endm;

        dbDataSet = GetDataBase( "Calibrations/etof/etofSignalVelocity" );

        St_etofSignalVelocity* etofSignalVelocity = static_cast< St_etofSignalVelocity* > ( dbDataSet->Find( "etofSignalVelocity" ) );
        if( !etofSignalVelocity ) {
            LOG_ERROR << "unable to get the signal velocity from the database" << endm;
            return kStFatal;
        }

        etofSignalVelocity_st* velocityTable = etofSignalVelocity->GetTable();
        
        for( size_t i=0; i<eTofConst::nCountersInSystem; i++ ) {
            unsigned int sector   = (   i       / 9 ) + 13;
            unsigned int zPlane   = ( ( i % 9 ) / 3 ) +  1;
            unsigned int counter  = (   i % 3       ) +  1;


            unsigned int key = sector * 100 + zPlane * 10  + counter;

            if( velocityTable->signalVelocity[ i ] > 0 ) {
                mSignalVelocity[ key ] = velocityTable->signalVelocity[ i ];
            }
        }
    }
    else {
        LOG_INFO << "etofSignalVelocity: filename provided --> use parameter file: " << mFileNameSignalVelocity.c_str() << endm;
        
        paramFile.open( mFileNameSignalVelocity.c_str() );

        if( !paramFile.is_open() ) {
            LOG_ERROR << "unable to get the 'etofSignalVelocity' parameters from file --> file does not exist" << endm;
            return kStFatal;
        }

        std::vector< float > velocity;
        float temp;
        while( paramFile >> temp ) {
            velocity.push_back( temp );
        }

        paramFile.close();

        if( velocity.size() != eTofConst::nCountersInSystem ) {
            LOG_ERROR << "parameter file for 'etofSignalVelocity' has not the right amount of entries: ";
            LOG_ERROR << velocity.size() << " instead of " << eTofConst::nCountersInSystem << " !!!!" << endm;
            return kStFatal;
        }

        for( size_t i=0; i<eTofConst::nCountersInSystem; i++ ) {
            unsigned int sector   = (   i       / 9 ) + 13;
            unsigned int zPlane   = ( ( i % 9 ) / 3 ) +  1;
            unsigned int counter  = (   i % 3       ) +  1;

            unsigned int key = sector * 100 + zPlane * 10  + counter;

            if( velocity.at( i ) > 0 ) {
                mSignalVelocity[ key ] = velocity.at( i );
            }
        }
    }

    for( const auto& kv : mSignalVelocity ) {
        LOG_DEBUG << "counter key: " << kv.first << " --> signal velocity = " << kv.second << " cm / ns" << endm;
    }

    // --------------------------------------------------------------------------------------------
	
	
	
	
    return kStOk;
}

//_____________________________________________________________
Int_t
StETofQAMaker::FinishRun( Int_t runnumber )
{
    return kStOk;
}

//_____________________________________________________________
Int_t
StETofQAMaker::Finish()
{

    updateCalib();

    LOG_INFO << "StETofQAMaker::Finish() - writing *.etofCalib.root ..." << endm;

    setHistFileName( ".etofQA.root" );

    LOG_INFO << "StETofQAMaker::Finish() - writing histograms to: " << mQAHistFileName.c_str() << endm;

    TFile histFile( mQAHistFileName.c_str(), "RECREATE", "etofQA" );
    histFile.cd();

    writeHistograms();

    LOG_INFO << "StETofQAMaker::Finish() - writing *.etofQA.root ..." << endm;

    histFile.Close();

    return kStOk;
}

//_____________________________________________________________
Int_t
StETofQAMaker::Make()
{
    LOG_DEBUG << "StETofQAMaker::Make(): starting ..." << endm;

    mEvent = ( StEvent* ) GetInputDS( "StEvent" );

    if ( mEvent ) {
        LOG_DEBUG << "Make() - running on StEvent" << endm;

        mIsStEvent = true;

        StBTofCollection* btofCollection = ( StBTofCollection* ) mEvent->btofCollection();

        StBTofHeader* btofHeader = nullptr; 

        if ( btofCollection ) {
            btofHeader = btofCollection->tofHeader();
        }

        StETofCollection* etofCollection = mEvent->etofCollection();
	StETofHeader* etofHeader = nullptr; 

        if ( etofCollection ) {
            etofHeader = etofCollection->etofHeader();
        }


        double pVtxZ = 0.;
        const StPrimaryVertex* pVtx = mEvent->primaryVertex();
        if ( pVtx ) {
            pVtxZ = pVtx->position().z();
        }

        //mTstart = calculateVpdTstart( btofHeader, pVtxZ, eTofConst::bTofClockCycle );
	mTstart = calculateNoVpdTstart(etofCollection, pVtx);

        processStEvent();
    }
    else {
        LOG_DEBUG << "Make() - no StEvent found" << endm;

        mMuDst = ( StMuDst* ) GetInputDS( "MuDst" );

        if( mMuDst ) {
            LOG_DEBUG << "Make() - running on MuDsts" << endm;

            mIsStEvent = false;

            StBTofHeader* btofHeader = mMuDst->btofHeader();

            double pVtxZ = 0.;
            StMuPrimaryVertex* pVtx = mMuDst->primaryVertex( 0 );
            if( pVtx ) {
                pVtxZ = pVtx->position().z();
            }

	
            //mTstart = calculateVpdTstart( btofHeader, pVtxZ, eTofConst::bTofClockCycle );
	    mTstart = calculateNoVpdTstart(mMuDst, pVtx);


            processMuDst();
        }
        else {
            LOG_WARN << "Make() - no StMuDst or StEvent" << endm;
        }
    }

    return kStOK;
}

//_____________________________________________________________
void
StETofQAMaker::processStEvent()
{
    StETofCollection* etofCollection = mEvent->etofCollection();

    LOG_DEBUG << "processStEvent() - getting the etof collection " << etofCollection << endm;

    if( !etofCollection ) {
        LOG_WARN << "processStEvent() - no etof collection" << endm;
        return;
    }

    if( !etofCollection->digisPresent() && !etofCollection->hitsPresent() ) {
        LOG_WARN << "processStEvent() - no digis and no hits present" << endm;
        return;
    }

    // --------------------------------------------------------

    StETofHeader* etofHeader = etofCollection->etofHeader();
    
    if( !etofHeader ) {
        LOG_WARN << "processStEvent() - no eTof header available -> skip event." << endm;
        return;
    }

    // skip event if it has no reset time of AFCK 0x18e6
    if( etofHeader->rocStarTs().count( 0x18e6 ) == 0 ) {
        LOG_WARN << "processStEvent() - no reset time for AFCK 0x18e6 available -> skip event." << endm;
        return;
    }

    // --------------------------------------------------------

    mTimeOffset = 0; //reset time offset for each event
    if( mTstart ) {
        mTimeOffset = mTstart; //use VPD start time if available
    }

    // --------------------------------------------------------
    // analyze hits in eTOF
    // --------------------------------------------------------
    const StSPtrVecETofHit& etofHits  = etofCollection->etofHits();

    mAverageETofTime  = 0.;
    mNHitsInTrgWindow = 0;
    mMapRpcCluMult.clear();

    size_t nHits = etofHits.size();
    LOG_INFO << "processStEvent() - # fired eTOF hits : " << nHits << endm;

    for( size_t i = 0; i<nHits; i++ ) {
        StETofHit* aHit = etofHits[ i ];

        if( !aHit ) {
            continue;
        }

        fillHitHistograms( aHit, etofHeader );
    }
    LOG_INFO << "processStEvent() - hit histograms filled with hits" << endm;

    if( mNHitsInTrgWindow > 0 ) mAverageETofTime /= mNHitsInTrgWindow;

    if( fabs( mAverageETofTime ) > 51200 ) {
        LOG_DEBUG << "average ETof time out of range: " << mAverageETofTime << endm;
    }

    for( const auto& kv : mMapRpcCluMult ) {
        LOG_DEBUG << " detector " << kv.first << " multiplicity " << kv.second << endm;

        std::string histName_RpcCluMul = "cl_Sector" + std::to_string( kv.first / 100 ) + "_ZPlane" + std::to_string( ( kv.first % 100 ) / 10 ) + "_Det" + std::to_string( kv.first % 10 ) + "_Mul";

        mHistograms1d.at( histName_RpcCluMul )->Fill( kv.second );
    }


    // --------------------------------------------------------
    // analyze digis in eTOF
    // --------------------------------------------------------
    const StSPtrVecETofDigi& etofDigis  = etofCollection->etofDigis();

    size_t nDigis = etofDigis.size();
    LOG_INFO << "processStEvent() - # fired eTOF digis : " << nDigis << endm;

    mNCalibDigis = 0;

    for( size_t i = 0; i<nDigis; i++ ) {
        StETofDigi* aDigi = etofDigis[ i ];

        if( !aDigi ) {
            continue;
        }

        fillDigiHistograms( aDigi, etofHeader );
    }
    LOG_INFO << "processStEvent() - digi histograms filled with " << mNCalibDigis << " calibrated digis" << endm;


    // --------------------------------------------------------
    // analyze hits in bTOF to get the eTOF-bTOF correlation
    // --------------------------------------------------------

    StBTofCollection* btofCollection = mEvent->btofCollection();
    
    LOG_DEBUG << "processStEvent() - getting the btof collection " << btofCollection << endm;

    if( !btofCollection ) {
        LOG_WARN << "processStEvent() - no btof collection" << endm;
        return;
    }

    if( !btofCollection->hitsPresent() ) {
        LOG_WARN << "processStEvent() - no bTof hits present" << endm;
        return;
    }

    fillBTofHistograms( btofCollection );
    // --------------------------------------------------------
}

//_____________________________________________________________
void
StETofQAMaker::processMuDst()
{
    LOG_INFO << "processMuDst(): starting ..." << endm;

    if( !mMuDst->etofArray( muETofDigi ) ) {
        LOG_WARN << "processMuDst() - no digi array" << endm;
        return;
    }
    if( !mMuDst->etofArray( muETofHit ) ) {
        LOG_WARN << "processMuDst() - no hit array" << endm;
        return;
    }
    if( !mMuDst->etofArray( muETofHeader ) ) {
        LOG_WARN << "processMuDst() - no header" << endm;
        return;
    }

    if( !mMuDst->numberOfETofDigi() && !mMuDst->numberOfETofHit() ) {
        LOG_WARN << "processMuDst() - no digis & no hits present" << endm;
        return;
    }

    // --------------------------------------------------------

    StMuETofHeader* etofHeader = mMuDst->etofHeader();
    
    if( !etofHeader ) {
        LOG_WARN << "processMuDst() - no eTof header available -> skip event." << endm;
        return;
    }

    // skip event if it has no reset time of AFCK 0x18e6
    if( etofHeader->rocStarTs().count( 0x18e6 ) == 0 ) {
        LOG_WARN << "processMuDst() - no reset time for AFCK 0x18e6 available -> skip event." << endm;
        return;
    }

    // --------------------------------------------------------

    mTimeOffset = 0; //reset time offset for each event
    if( mTstart ) {
        mTimeOffset = mTstart; //use VPD start time if available
    }

    // --------------------------------------------------------
    // analyze hits in eTOF
    // --------------------------------------------------------
    mAverageETofTime  = 0.;
    mNHitsInTrgWindow = 0;
    mMapRpcCluMult.clear();

    size_t nHits = mMuDst->numberOfETofHit();
    LOG_INFO << "processMuDst() - # fired eTOF hits : " << nHits << endm;

    for( size_t i = 0; i<nHits; i++ ) {
        StMuETofHit* aHit = mMuDst->etofHit( i );

        if( !aHit ) {
            continue;
        }

        fillHitHistograms( aHit, etofHeader );
    }
    LOG_INFO << "processMuDst() - hit histograms filled with hits" << endm;

    if( mNHitsInTrgWindow > 0 ) mAverageETofTime /= mNHitsInTrgWindow;

    if( fabs( mAverageETofTime ) > 51200 ) {
        LOG_DEBUG << "average ETof time out of range: " << mAverageETofTime << endm;
    }

    for( const auto& kv : mMapRpcCluMult ) {
        LOG_DEBUG << " detector " << kv.first << " multiplicity " << kv.second << endm;

        std::string histName_RpcCluMul = "cl_Sector" + std::to_string( kv.first / 100 ) + "_ZPlane" + std::to_string( ( kv.first % 100 ) / 10 ) + "_Det" + std::to_string( kv.first % 10 ) + "_Mul";

        mHistograms1d.at( histName_RpcCluMul )->Fill( kv.second );
    }


    // --------------------------------------------------------
    // analyze digis in eTOF
    // --------------------------------------------------------
    size_t nDigis = mMuDst->numberOfETofDigi();
    LOG_INFO << "processMuDst() - # fired eTOF digis : " << nDigis << endm;

    mNCalibDigis = 0;

    for( size_t i = 0; i<nDigis; i++ ) {
        StMuETofDigi* aDigi = mMuDst->etofDigi( i );

        if( !aDigi ) {
            continue;
        }

        fillDigiHistograms( aDigi, etofHeader );
    }
    LOG_INFO << "processMuDst() - digi histograms filled with " << mNCalibDigis << " calibrated digis" << endm;


    // --------------------------------------------------------
    // analyze hits in bTOF to get the eTOF-bTOF correlation
    // --------------------------------------------------------
    if( !mMuDst->btofArray( muBTofHit ) ) {
        LOG_WARN << "processMuDst() - no btof hit array" << endm;
        return;
    }
    
    if( !mMuDst->numberOfBTofHit() ) {
        LOG_WARN << "processMuDst() - no btof hits present" << endm;
        return;
    }

    fillBTofHistograms();
    // --------------------------------------------------------
}


//_____________________________________________________________  
void
StETofQAMaker::writeHistograms()
{
    LOG_DEBUG << "writeHistograms() ... " << endm;
    for( const auto& kv : mHistograms1d ) {
        if( kv.second->GetEntries() > 0 ) kv.second->Write();
        //kv.second->Write();
    }
    for( const auto& kv : mHistograms2d ) {
        if( kv.second->GetEntries() > 0 ) kv.second->Write();
        //kv.second->Write();
    }
}
//_____________________________________________________________  



//_____________________________________________________________
void
StETofQAMaker::bookHistograms()
{
    LOG_INFO << "bookHistograms() ... " << endm;

    double tSumMax=30.0; //reduced from 5 to keep chan walk histos small!
//TODO: Move to setter function!
	if (mCalibStep>8) tSumMax=10.0;

	if (mCalibStep>12) tSumMax=5.0;
	
	if (mCalibStep>16) tSumMax=3.0;

    mHistograms1d[ "Av_TDiff_ETof_BTof" ] =  new TH1F( Form( "Av_TDiff_ETof_BTof" ),
            Form( "Difference between average hits times in Events between BTof and ETof; dT [ns]; Events []" ),
            102400, -51200, 51200 ); 

    mHistograms1d[ "Hit_TDiff_ETof_Trg" ] = new TH1F( Form( "Hit_TDiff_ETof_Trg" ),
            Form( "Difference between hits on ETof and the trigger token; dT [ns]; Events []" ),
            10240, -51200, 51200 ); 

    mHistograms1d[ "Digi_TDiff_ETof_Trg" ] = new TH1F( Form( "Digi_TDiff_ETof_Trg" ),
            Form( "Difference between Digis on ETof and the trigger token; dT [ns]; Events []" ),
            10240, -51200, 51200 ); 

    mHistograms1d[ "DigiRaw_TDiff_ETof_Trg" ] = new TH1F( Form( "DigiRaw_TDiff_ETof_Trg" ),
            Form( "Raw time difference between Digis on ETof and the trigger token; dT [ns]; Events []" ),
            10240, -51200, 51200 );

    mHistograms1d[ "AllDigiRaw_TDiff_ETof_Trg" ] = new TH1F( Form( "AllDigiRaw_TDiff_ETof_Trg" ),
            Form( "Raw time difference between Digis on ETof and the trigger token; dT [ns]; Events []" ),
            10240, -51200, 51200 );

    mHistograms1d[ "ETof_Time_of_Flight" ] = new TH1F( "ETof_Time_of_Flight",
            Form( "time of flight for eTof hits; Tof [ns]; # ETof hits" ),
            1000,  -150, 150 );

    mHistograms1d[ "BTof_Time_of_Flight" ] = new TH1F( "BTof_Time_of_Flight",
            Form( "time of flight for bTof hits; Tof [ns]; # BTof hits" ),
            1000, -150, 150 );



    mHistograms2d[ "Mul_ETof_BTof" ] = new TH2F( Form( "Mul_ETof_BTof" ),
            Form( "Multiplicity correlation between ETof and BTof; ETof Multiplicity []; BTof Multiplicity []" ),
            51, -0.5, 50.5, 101, -0.5, 500.5 );

    mHistograms2d[ "Av_TDiff_v_Mul_ETof" ] = new TH2F( Form( "Av_TDiff_v_Mul_ETof" ),
            Form( "Difference between average hits times in Events between BTof and ETof vs ETof multiplicity; dT [ns]; ETof Multiplicity []" ),
            10240, -51200, 51200, 51, -0.5, 50.5 );

    mHistograms2d[ "Av_TDiff_v_Mul_BTof" ] = new TH2F( Form( "Av_TDiff_v_Mul_BTof" ),
            Form( "Difference between average hits times in Events between BTof and ETof vs ETof multiplicity; dT [ns]; BTof Multiplicity []" ),
            10240, -51200, 51200, 101, -0.5, 500.5 );


    for( int sector = eTofConst::sectorStart; sector <= eTofConst::sectorStop; sector++ ) {
        for( int plane = eTofConst::zPlaneStart; plane <= eTofConst::zPlaneStop; plane++ ) {
            for( int counter = eTofConst::counterStart; counter <= eTofConst::counterStop; counter++ ) {

                std::string histName_EtofTimeOfFlight = "ETof_Time_of_Flight_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter );

                mHistograms1d[ histName_EtofTimeOfFlight ] = new TH1F( Form( "ETof_Time_of_Flight_Sector%02d_ZPlane%d_Det%d_", sector, plane, counter ),
                        Form( "time of flight for eTof hits of Det #%02d in ZPlane %d under Sector %02d; Tof [ns]; # ETof hits", counter, plane, sector ),
                        1000,  -150, 150 );

                std::string histName_RpcCluSize = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Size";

                mHistograms2d[ histName_RpcCluSize ] = new TH2F( Form( "cl_Sector%02d_ZPlane%d_Det%d_Size", sector, plane, counter ),
                        Form( "Cluster size of Det #%02d in ZPlane %d under Sector %02d; Strip []; size [strips]", counter, plane, sector ),
                        32, 0, 32, 16, 0.5, 16.5 );


                std::string histName_RpcCluPosition = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Pos";

                mHistograms2d[ histName_RpcCluPosition ] = new TH2F( Form( "cl_Sector%02d_ZPlane%d_Det%d_Pos", sector, plane, counter ),
                        Form( "Cluster position of of Det #%02d in ZPlane %d under Sector %02d; Strip []; ypos [cm]", counter, plane, sector ),
                        32, 0, 32, 100, -50, 50 );


                std::string histName_RpcCluTOff = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_TOff";

                mHistograms2d[ histName_RpcCluTOff ] = new TH2F( Form( "cl_Sector%02d_ZPlane%d_Det%d_TOff", sector, plane, counter ),
                        Form( "Cluster timezero of Det #%02d in ZPlane %d under Sector %02d; Strip []; TOff [ns]", counter, plane, sector ),
                        32, 0, 32, 250, -5, 20); //100 ps binning!


                std::string histName_RpcCluTot = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Tot";

                mHistograms2d[ histName_RpcCluTot ] = new TH2F( Form( "cl_Sector%02d_ZPlane%d_Det%d_Tot", sector, plane, counter ),
                        Form( "Total Cluster Tot of Det #%02d in ZPlane %d under Sector %02d; Strip []; TOT [ns]", counter, plane, sector ),
                        32, 0, 32, 100, 0, 100 );


                std::string histName_RpcDigiTot = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_TotDigi";

                mHistograms2d[ histName_RpcDigiTot ] = new TH2F( Form( "cl_Sector%02d_ZPlane%d_Det%d_TotDigi", sector, plane, counter ),
                        Form( "Total Digi Tot of Det #%02d in ZPlane %d under Sector %02d; Chan []; TOT [ns]", counter, plane, sector ),
                        64, 0, 64, 50, 0, 20 );


                std::string histName_RpcCluAvWalk = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_AvCluWalk";

                mHistograms2d[ histName_RpcCluAvWalk ] = new TH2F( Form( "cl_Sector%02d_ZPlane%d_Det%d_AvCluWalk", sector, plane, counter ),
                        Form( "Intra-Cluster Walk in Det #%02d in ZPlane %d under Sector %02d; digi TOT; T_digi-T_cluster", counter, plane, sector ),
                        15, 0, 6, 150, -0.5 * tSumMax, 0.5 * tSumMax ); 


                std::string histName_RpcDigiAvWalk = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_AvWalk";

                mHistograms2d[ histName_RpcDigiAvWalk ] = new TH2F( Form( "cl_Sector%02d_ZPlane%d_Det%d_AvWalk", sector, plane, counter ),
                        Form( "Walk in Det #%02d in ZPlane %d under Sector %02d; Digi TOT; T-TSel", counter, plane, sector ),
                        25, 0, 10, 300, -2 * tSumMax, 2 * tSumMax );


                std::string histName_RpcCluMul = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Mul";

                mHistograms1d[ histName_RpcCluMul ] = new TH1F( Form( "cl_Sector%02d_ZPlane%d_Det%d_Mul", sector, plane, counter ),
                        Form( "Cluster multiplicity of Det #%02d in ZPlane %d under Sector %02d; Mul []; cnts", counter, plane, sector ),
                        32, 0, 32 );


                std::string histName_HitTrigTimeDet = "Hit_TDiff_Trg_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter );

                mHistograms1d[ histName_HitTrigTimeDet ] = new TH1F( Form( "Hit_TDiff_Trg_Sector%02d_ZPlane%d_Det%d_Mul", sector, plane, counter ),
                        Form( "Difference between hits on Det #%02d in ZPlane %d under Sector %02d and the trigger token; dT [ns]; Events []", counter, plane, sector ),
                        10240, -51200, 51200 );

                // T0 correction histograms
                std::string histName_t0corr_mom  = "matched_t0corr_mom_s" + std::to_string( sector ) + "m" + std::to_string( plane ) + "c" + std::to_string( counter );

                mHistograms2d[ histName_t0corr_mom ] = new TH2F( Form( "G_matched_t0corr_mom_s%dm%dc%d", sector, plane, counter ),  Form( "measured tof - tof_{#pi} vs. momentum in sector %d module %d counter %d;mom (GeV/c);#Delta time (ns)",  sector, plane, counter ), 400,     0.,  10., 1000, -20*tSumMax, 20*tSumMax );

                std::string histName_t0corr_mom_zoom = "matched_t0corr_mom_zoom_s" + std::to_string( sector ) + "m" + std::to_string( plane ) + "c" + std::to_string( counter );

                mHistograms2d[ histName_t0corr_mom_zoom ] = new TH2F( Form( "G_matched_t0corr_mom_zoom_s%dm%dc%d", sector, plane, counter ), Form( "measured tof - tof_{#pi} vs. momentum in sector %d module %d counter %d;mom (GeV/c);#Delta time (ns)", sector, plane, counter ), 200, 0., 3., 1000, -5., 5. );

                // walk histograms
                for( int channel = 0; channel < 2 * eTofConst::nStrips; channel++ ) {

                    std::string histName_RpcDigiChanWalk = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Chan" + std::to_string( channel ) + "_Walk";

                   /* mHistograms2d[ histName_RpcDigiChanWalk ] = new TH2S( Form( "cl_Sector%02d_ZPlane%d_Det%d_Chan%d_Walk", sector, plane, counter,  channel ),
                            Form( "Walk of Chan %d in Det #%02d in ZPlane %d under Sector %02d; cluster TOT; T-TSel", channel, counter, plane, sector ),
                            25, 0, 10, 150, -0.5 * tSumMax, 0.5 * tSumMax ); //FIXME: somehow throws a bad alloc at 2111 ! changed to TH2S to save memory. Still crashes depending on total bins in the walk histogram. 15 is still to high.*/

                    mHistograms2d[ histName_RpcDigiChanWalk ] = new TH2S( Form( "cl_Sector%02d_ZPlane%d_Det%d_Chan%d_Walk", sector, plane, counter,  channel ),
                            Form( "Walk of Chan %d in Det #%02d in ZPlane %d under Sector %02d; cluster TOT; T-TSel", channel, counter, plane, sector ),
                            15, 0, 6, 150, -1 * tSumMax, 1 * tSumMax );
                                                                             //Count coordiniate now overflows at 32.000 entries. Take care! PW
                }

            }
        }
    }

    for( auto& kv : mHistograms1d ) {
        kv.second->SetDirectory( 0 );
    }

    for( auto& kv : mHistograms2d ) {
        kv.second->SetDirectory( 0 );
    }

    LOG_INFO << "bookHistograms() -- successfully created all histograms " << endm;
}//::bookHistograms

//_____________________________________________________________
// setHistFileName uses the string argument from the chain being run to set
// the name of the output histogram file.
void
StETofQAMaker::setHistFileName( std::string extension )
{
    if( GetChainOpt()->GetFileOut() != nullptr ) {
        TString outFile = GetChainOpt()->GetFileOut();

        mQAHistFileName = ( string ) outFile;

        // get rid of .root
        size_t lastindex = mQAHistFileName.find_last_of( "." );
        mQAHistFileName = mQAHistFileName.substr( 0, lastindex );

        // get rid of .MuDst or .event if necessary
        lastindex = mQAHistFileName.find_last_of( "." );
        mQAHistFileName = mQAHistFileName.substr( 0, lastindex );

        // get rid of directories
        lastindex = mQAHistFileName.find_last_of( "/" );
        mQAHistFileName = mQAHistFileName.substr( lastindex + 1, mQAHistFileName.length() );

        mQAHistFileName = mQAHistFileName + extension;
    }
    else {
        LOG_ERROR << "Cannot set the output filename for histograms" << endm;
    }
}

//_____________________________________________________________
void
StETofQAMaker::fillDigiHistograms( StETofDigi* aDigi, StETofHeader* header )
{
    LOG_DEBUG << "fillDigiHistograms() -- filling histograms with digis for calibration and QA" << endm;

    double triggerTime = header->trgGdpbFullTime();

    if( header->rocStarTs().count( aDigi->rocId() ) ) {
        triggerTime = header->rocGdpbTs().at( aDigi->rocId() ) * eTofConst::coarseClockCycle;
    }

    mHistograms1d.at( "AllDigiRaw_TDiff_ETof_Trg" )->Fill( aDigi->rawTime() - triggerTime );


    //using the only working reset time in 2018 for now
    double triggerTimeReduced = triggerTime - ( header->rocStarTs().at( 0x18e6 ) * eTofConst::coarseClockCycle );

    double triggerTimeModulo = fmod( triggerTimeReduced, eTofConst::bTofClockCycle );

    LOG_DEBUG << " 'corrected' trigger time: "<< triggerTimeModulo <<" ns" << endm;



    if ( aDigi->calibTime() == 0 && aDigi->calibTot() == -1 ) {
        LOG_DEBUG << "fillDigiHistograms() -- digi not calibrated, most likely since it is outside the trigger window. Ignore." << endm;
        return;
    }

    if( aDigi->sector() == 0 || aDigi->zPlane() == 0 || aDigi->counter() == 0 || aDigi->strip() == 0 ) {
        LOG_WARN << "StETofQAMaker::fillDigiHistograms() -- sector / zPlane / counter / strip  was not assigned to the digi" << endm;
        return;
    }

    mNCalibDigis++;

    mHistograms1d.at( "DigiRaw_TDiff_ETof_Trg" )->Fill( aDigi->rawTime()   - triggerTime );
    mHistograms1d.at( "Digi_TDiff_ETof_Trg"    )->Fill( aDigi->calibTime() - triggerTimeReduced );


    int sector  = aDigi->sector();
    int plane   = aDigi->zPlane();
    int counter = aDigi->counter();
    int channel = 32 * ( aDigi->side() - 1 ) + aDigi->strip() - 1;

    double reducedTime = fmod( aDigi->calibTime(), eTofConst::bTofClockCycle ) - mTimeOffset; //TODO: FIX T0

    std::string histName_RpcDigiTot      = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_TotDigi";
    std::string histName_RpcDigiAvWalk   = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_AvWalk";
    std::string histName_RpcDigiChanWalk = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Chan" + std::to_string( channel ) + "_Walk";
    std::string histName_RpcCluAvWalk    = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_AvCluWalk";

    mHistograms2d.at( histName_RpcDigiTot      )->Fill( channel,           aDigi->calibTot() );    

    if( mIsStEvent && aDigi->associatedHit() != nullptr ) {
        mHistograms2d.at( histName_RpcCluAvWalk )->Fill( aDigi->calibTot(), fmod( aDigi->calibTime(), eTofConst::bTofClockCycle ) - ( aDigi->associatedHit() )->time() );

        if( ( aDigi->associatedHit() )->associatedTrack() != nullptr ) {
            mHistograms2d.at( histName_RpcDigiChanWalk )->Fill( aDigi->calibTot(), reducedTime ); 

		     for( const auto& traits: ( ( aDigi->associatedHit() )->associatedTrack() )->pidTraits() ) {
		         if( traits->detector() == kETofId ) {
		             StPrimaryTrack* pTrack = dynamic_cast< StPrimaryTrack* > ( ( aDigi->associatedHit() )->associatedTrack()->node()->track( primary ) );
		             if( !pTrack ) continue;

		             float tof        = ( dynamic_cast< StETofPidTraits* > ( traits ) )->timeOfFlight();
		             float pathlength = ( dynamic_cast< StETofPidTraits* > ( traits ) )->pathLength();

		             float mom = pTrack->geometry()->momentum().mag();

		             float tofpi = expectedTimeOfFlight( pathlength , mom, pion_plus_mass_c2 );
								    
								    float walkTime;
								    if (aDigi->side() == 1)
								    {// Replace hit.time in tof by digi.time + Correction for the Y-Position dependence of digi.time. Should be more sensitive to walk of this digi.
								    	walkTime = (tof - tofpi); /*- //hit time of flight deviation
													   aDigi->associatedHit()->time() +
														aDigi->calibTime() + //positive for side 1
														0.5*(aDigi->associatedHit()->localY()/mSignalVelocity[aDigi->sector() * 100 + aDigi->zPlane() * 10  + aDigi->counter()]); //correct out Y-Position dependece of digi time*/

								    }
								    else
								    {// Replace hit.time in tof by digi.time + Correction for the Y-Position dependence of digi.time. Should be more sensitive to walk of this digi.
								    	walkTime = (tof - tofpi); /*- //hit time of flight deviation
													   aDigi->associatedHit()->time() +
														aDigi->calibTime() - //positive for side 2
														0.5*(aDigi->associatedHit()->localY()/mSignalVelocity[aDigi->sector() * 100 + aDigi->zPlane() * 10  + aDigi->counter()]); //correct out Y-Position dependece of digi time*/

								    }								    

										mHistograms2d.at( histName_RpcDigiChanWalk )->Fill( aDigi->calibTot(), walkTime); 
							 			mHistograms2d.at( histName_RpcDigiAvWalk   )->Fill( aDigi->calibTot(), walkTime);
		             break;
		         }
		     }


        }


    } 
}//::fillDigiHistograms

// muDst version:
void
StETofQAMaker::fillDigiHistograms( StMuETofDigi* aDigi, StMuETofHeader* header )
{
    fillDigiHistograms( ( StETofDigi* ) aDigi, ( StETofHeader* ) header  );

    int hitId = aDigi->associatedHitId();
    if( hitId == -1 ) {
        LOG_DEBUG << "fillDigiHistograms() - associated hit id is -1 ... not filling the histogram" << endm;
    }
    else{
        if( hitId < ( int ) mMuDst->numberOfETofHit() ) {
            StMuETofHit* aHit = mMuDst->etofHit( hitId );
        
            if( aHit ) {
                std::string histName_RpcCluAvWalk = "cl_Sector" + std::to_string( aDigi->sector() ) + "_ZPlane" + std::to_string( aDigi->zPlane() ) + "_Det" + std::to_string( aDigi->counter() ) + "_AvCluWalk";

                mHistograms2d.at( histName_RpcCluAvWalk )->Fill( aDigi->calibTot(), fmod( aDigi->calibTime(), eTofConst::bTofClockCycle ) - aHit->time() );

                LOG_DEBUG << "cluster average walk histogram filled ... (associatedHitId=" << hitId << ")" << endm;

                if ( aHit->associatedTrackId() != -1 ) {
                    //maybe not needed in MuDST case?
                   // double reducedTime = fmod( aDigi->calibTime(), eTofConst::bTofClockCycle ) - mTimeOffset; //TODO: FIX T0
    					  std::string histName_RpcDigiAvWalk   = "cl_Sector" + std::to_string(  aDigi->sector() ) + "_ZPlane" + std::to_string(  aDigi->zPlane() ) + "_Det" + std::to_string(  aDigi->counter() ) + "_AvWalk";

                    std::string histName_RpcDigiChanWalk = "cl_Sector" + std::to_string( aDigi->sector() ) + "_ZPlane" + std::to_string( aDigi->zPlane() ) + "_Det" + std::to_string( aDigi->counter() ) + "_Chan" + std::to_string( 32 * ( aDigi->side() - 1 ) + aDigi->strip() - 1 ) + "_Walk";

						StMuTrack* aTrack = aHit->primaryTrack();
						if( aTrack != nullptr ) {

								    float tof        = aTrack->etofPidTraits().timeOfFlight();
								    float pathlength = aTrack->etofPidTraits().pathLength();

								    float mom = aTrack->momentum().mag();

								    float tofpi = expectedTimeOfFlight( pathlength , mom, pion_plus_mass_c2 );
								    
								    float walkTime;
								    if (aDigi->side() == 1)
								    {// Replace hit.time in tof by digi.time + Correction for the Y-Position dependence of digi.time. Should be more sensitive to walk of this digi.
								    	walkTime = (tof - tofpi); /*- //hit time of flight deviation
													   aHit->time() +
														aDigi->calibTime() + //positive for side 1
														0.5*(aHit->localY()/mSignalVelocity[aDigi->sector() * 100 + aDigi->zPlane() * 10  + aDigi->counter()]); //correct out Y-Position dependece of digi time*/

								    }
								    else
								    {// Replace hit.time in tof by digi.time + Correction for the Y-Position dependence of digi.time. Should be more sensitive to walk of this digi.
								    	walkTime = (tof - tofpi);  /*- //hit time of flight deviation
													  aHit->time() +
														aDigi->calibTime() - //positive for side 2
														0.5*(aHit->localY()/mSignalVelocity[aDigi->sector() * 100 + aDigi->zPlane() * 10  + aDigi->counter()]); //correct out Y-Position dependece of digi time*/

								    }

										mHistograms2d.at( histName_RpcDigiChanWalk )->Fill( aDigi->calibTot(), walkTime); 
							 			mHistograms2d.at( histName_RpcDigiAvWalk   )->Fill( aDigi->calibTot(), walkTime);

						}
                }
            }
        }  
    }
}

//_____________________________________________________________
void
StETofQAMaker::fillHitHistograms( StETofHit* aHit, StETofHeader* header )
{
    LOG_DEBUG << "fillDigiHistograms() -- filling histograms with hits for calibration and QA" << endm;

    //using the only working reset time in 2018 for now
    double triggerTimeReduced = header->trgGdpbFullTime() - ( header->rocStarTs().at( 0x18e6 ) * eTofConst::coarseClockCycle );
    double triggerTimeModulo = fmod( triggerTimeReduced, eTofConst::bTofClockCycle );


    if( aHit->sector() == 0 || aHit->zPlane() == 0 || aHit->counter() == 0 ) {
        LOG_ERROR << "StETofQAMaker::fillHitHistograms():  -- sector / zPlane / counter was not assigned to the hit -- something is wrong!" << endm;
        return;
    }
    
    if( mTimeOffset == 0 && mTstart == 0) { // set time offset ot first digi time if no VPD start time is available
        mTimeOffset = aHit->time();
        LOG_INFO << "the time offset is set to " << mTimeOffset << endm;
    }


    mNHitsInTrgWindow++;
    mAverageETofTime += aHit->time();

    int sector  = aHit->sector();
    int plane   = aHit->zPlane();
    int counter = aHit->counter();

    std::string histName_RpcCluSize     = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Size";
    std::string histName_RpcCluPosition = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Pos";
    std::string histName_RpcCluTOff     = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_TOff";
    std::string histName_RpcCluTot      = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Tot";

    // add (eTofConst::nStrips/2.0) to localX in order to recover strip number as localX has its origin at the center of the detector!
    mHistograms2d.at( histName_RpcCluSize     )->Fill( aHit->localX() + ( eTofConst::nStrips / 2.0 ), aHit->clusterSize()        );
    mHistograms2d.at( histName_RpcCluPosition )->Fill( aHit->localX() + ( eTofConst::nStrips / 2.0 ), aHit->localY()             );
    mHistograms2d.at( histName_RpcCluTOff     )->Fill( aHit->localX() + ( eTofConst::nStrips / 2.0 ), aHit->time() - mTimeOffset );
    mHistograms2d.at( histName_RpcCluTot      )->Fill( aHit->localX() + ( eTofConst::nStrips / 2.0 ), aHit->totalTot()           );
    


    std::string histName_HitTrigTimeDet = "Hit_TDiff_Trg_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter );

    mHistograms1d.at( "Hit_TDiff_ETof_Trg"    )->Fill( aHit->time() - triggerTimeModulo );    
    mHistograms1d.at( histName_HitTrigTimeDet )->Fill( aHit->time() - triggerTimeModulo ); //detector specific trigger plot


    if( mTstart ) {
        float timeOfFlight = aHit->time() - mTstart;

        std::string histName_EtofTimeOfFlight = "ETof_Time_of_Flight_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter );
        
        mHistograms1d.at( "ETof_Time_of_Flight"     )->Fill( timeOfFlight );
        mHistograms1d.at( histName_EtofTimeOfFlight )->Fill( timeOfFlight );
    }

    //-------------------------
    if( mIsStEvent && aHit->associatedTrack() != nullptr ) {
        for( const auto& traits: ( aHit->associatedTrack() )->pidTraits() ) {
            if( traits->detector() == kETofId ) {
                StPrimaryTrack* pTrack = dynamic_cast< StPrimaryTrack* > ( aHit->associatedTrack()->node()->track( primary ) ); // not sure if this works as intendet
                if( !pTrack ) continue; // only use primary tracks

                float tof        = ( dynamic_cast< StETofPidTraits* > ( traits ) )->timeOfFlight();
                float pathlength = ( dynamic_cast< StETofPidTraits* > ( traits ) )->pathLength();

                float mom = pTrack->geometry()->momentum().mag();

                float tofpi = expectedTimeOfFlight( pathlength , mom, pion_plus_mass_c2 );

                std::string histName_t0corr_mom  = "matched_t0corr_mom_s" + std::to_string( sector ) + "m" + std::to_string( plane ) + "c" + std::to_string( counter );
                mHistograms2d.at( histName_t0corr_mom )->Fill( mom, tof - tofpi );

                std::string histName_t0corr_mom_zoom = "matched_t0corr_mom_zoom_s" + std::to_string( sector ) + "m" + std::to_string( plane ) + "c" + std::to_string( counter );
                mHistograms2d.at( histName_t0corr_mom_zoom )->Fill( mom, tof - tofpi );

                break;
            }
        }
    }
    //--------------------------

    unsigned int detIndex = sector * 100 + plane * 10 + counter;
    if( mMapRpcCluMult.count( detIndex ) ) {
        mMapRpcCluMult.at( detIndex ) += 1;
    }
    else {
        mMapRpcCluMult[ detIndex ] = 1;
    }

    LOG_DEBUG << "number of hits in detector " << detIndex << " so far: " << mMapRpcCluMult.at( detIndex ) << endm;
}//::fillHitHistograms


// muDst version: 
void
StETofQAMaker::fillHitHistograms( StMuETofHit* aHit, StMuETofHeader* etofHeader )
{
    fillHitHistograms( ( StETofHit* ) aHit, ( StETofHeader* ) etofHeader  );

    StMuETofPidTraits traits;
    StMuTrack* pTrack = aHit->primaryTrack();

    if( pTrack ) {
        traits = pTrack->etofPidTraits();

        if( traits.matchFlag() > 0 ) {
            float tof        = traits.timeOfFlight();
            float pathlength = traits.pathLength();

            float mom = pTrack->momentum().mag();

            float tofpi = pathlength * centimeter * ( 1./c_light ) * sqrt( 1 + pion_plus_mass_c2 * pion_plus_mass_c2 / ( mom * mom ) ) / nanosecond;

            std::string histName_t0corr_mom  = "matched_t0corr_mom_s" + std::to_string( aHit->sector() ) + "m" + std::to_string( aHit->zPlane() ) + "c" + std::to_string( aHit->counter() );
            mHistograms2d.at( histName_t0corr_mom )->Fill( mom, tof - tofpi );

            std::string histName_t0corr_mom_zoom = "matched_t0corr_mom_zoom_s" + std::to_string( aHit->sector() ) + "m" + std::to_string( aHit->zPlane() ) + "c" + std::to_string( aHit->counter() );
            mHistograms2d.at( histName_t0corr_mom_zoom )->Fill( mom, tof - tofpi );
        }
    }
}

    
//_____________________________________________________________
void
StETofQAMaker::fillBTofHistograms( StBTofCollection* coll )
{
    const StSPtrVecBTofHit& bTofHits = coll->tofHits();

    size_t nBtofHits = bTofHits.size();
    if( !nBtofHits ) {
        return;
    }
    LOG_INFO << "processStEvent() - # fired bTOF hits : " << nBtofHits << endm;
    
    double avBtofTime = 0;
    size_t nValidHits = 0;

    for( size_t i = 0; i < nBtofHits; i++ ) {
        StBTofHit* aHit = bTofHits[ i ];
        
        if( !aHit ) {
            continue;
        }

        if( mTstart ) {
            float timeOfFlight = aHit->leadingEdgeTime() - mTstart;
            
            mHistograms1d.at( "BTof_Time_of_Flight" )->Fill( timeOfFlight );
            LOG_DEBUG << "bTof time of flight: " << timeOfFlight << endm;
        }

        avBtofTime += aHit->leadingEdgeTime();
        nValidHits++;
    }

    avBtofTime /= nValidHits;

    mHistograms1d.at( "Av_TDiff_ETof_BTof"  )->Fill( mAverageETofTime - avBtofTime );
    mHistograms2d.at( "Av_TDiff_v_Mul_ETof" )->Fill( mAverageETofTime - avBtofTime, mNHitsInTrgWindow );
    mHistograms2d.at( "Av_TDiff_v_Mul_BTof" )->Fill( mAverageETofTime - avBtofTime, nValidHits        );
    mHistograms2d.at( "Mul_ETof_BTof"       )->Fill( mNHitsInTrgWindow,             nValidHits        );
}//::fillBTofHistograms

// muDst version:
void
StETofQAMaker::fillBTofHistograms()
{
    size_t nBtofHits = mMuDst->numberOfBTofHit();
    if( !nBtofHits ) {
        return;
    }
    LOG_INFO << "processStEvent() - # fired bTOF hits : " << nBtofHits << endm;
    
    double avBtofTime = 0;
    size_t nValidHits = 0;

    for( size_t i = 0; i < nBtofHits; i++ ) {
        StMuBTofHit* aHit = mMuDst->btofHit( i );
        
        if( !aHit ) {
            continue;
        }

        if( mTstart ) {
            float timeOfFlight = aHit->leadingEdgeTime() - mTstart;
            
            mHistograms1d.at( "BTof_Time_of_Flight" )->Fill( timeOfFlight );
            LOG_DEBUG << "bTof time of flight: " << timeOfFlight << endm;
        }

        avBtofTime += aHit->leadingEdgeTime();
        nValidHits++;
    }

    avBtofTime /= nValidHits;

    mHistograms1d.at( "Av_TDiff_ETof_BTof"  )->Fill( mAverageETofTime - avBtofTime );
    mHistograms2d.at( "Av_TDiff_v_Mul_ETof" )->Fill( mAverageETofTime - avBtofTime, mNHitsInTrgWindow );
    mHistograms2d.at( "Av_TDiff_v_Mul_BTof" )->Fill( mAverageETofTime - avBtofTime, nValidHits        );
    mHistograms2d.at( "Mul_ETof_BTof"       )->Fill( mNHitsInTrgWindow,             nValidHits        );
}//::fillBTofHistograms

//_____________________________________________________________
void
StETofQAMaker::initCalib()
{
    LOG_INFO << "initCalib() ... " << endm;

    //create input-specific calibfile name to avoid problems with scheduler
    std::string extension = ".etofCalib.root";

    if( GetChainOpt()->GetFileOut() != nullptr ) {
        TString outFile = GetChainOpt()->GetFileOut();

        mCalibOutHistFileName = ( std::string ) outFile;

        // get rid of .root
        size_t lastindex = mCalibOutHistFileName.find_last_of( "." );
        mCalibOutHistFileName = mCalibOutHistFileName.substr( 0, lastindex );

        // get rid of .MuDst or .event if necessary
        lastindex = mCalibOutHistFileName.find_last_of( "." );
        mCalibOutHistFileName = mCalibOutHistFileName.substr( 0, lastindex );

        // get rid of directories
        lastindex = mCalibOutHistFileName.find_last_of( "/" );
        mCalibOutHistFileName = mCalibOutHistFileName.substr( lastindex + 1, mCalibOutHistFileName.length() );

        mCalibOutHistFileName = mCalibOutHistFileName + extension;
    }
    else {
        LOG_ERROR << "Cannot set the output filename for calibration" << endm;
    }

    mCalibOutFile = new TFile( mCalibOutHistFileName.c_str(), "RECREATE" );

    mCalibInFile = new TFile( mCalibInHistFileName.c_str(), "READ" );

    if( mCalibInFile && !mCalibInFile->IsZombie() ) {
		 if ( mCalibInFile->FindObjectAny( "mCalibStep" ) ) {
		     mCalibStep = ( (TObject*) mCalibInFile->FindObjectAny( "mCalibStep" ) )->GetUniqueID();
		 }
		 else {
			  LOG_INFO << "StETofQAMaker::initCalib() - no calib step found in existing parameter file. Corrupted" << endm;
			  return;
		 }
        LOG_INFO << "StETofQAMaker::initCalib() - calibration paramerter file loaded" << endm;
    }
    else{
        LOG_INFO << "StETofQAMaker::initCalib() - no input calibration file found. creating new calib file. " << endm;
		  mCalibInFile  = new TFile( mCalibInHistFileName.c_str(),  "RECREATE"     );
        mCalibStep = 0;
    }
    
    if( iCalibLoop[ mCalibStep ] ) {
        LOG_INFO << "initCalib() - initialising calibration step " << mCalibStep << " in calibration mode "<< iCalibLoop[ mCalibStep ] << endm;
    }
    else {
        LOG_INFO << "initCalib() - no calibration step to be performed" << endm;
    }

 }//::initCalib

//_____________________________________________________________
void
StETofQAMaker::updateCalib()
{
    if( !iCalibLoop [mCalibStep ] ) return; // calibration finished --> do not update

    int iCalibMode = iCalibLoop[ mCalibStep ];

mCalibOutFile->cd();
    TString hname;

    TObject calibSave;

    TProfile* hOldWalkProfile = nullptr;
    TProfile* hWalkProfile = new TProfile();

    TProfile* hOldPosProfile = nullptr;
    TProfile* hPosProfile = new TProfile();

    TProfile* hOldTotProfile = nullptr;
    TProfile* hTotProfile = new TProfile();

    TProfile* hOldT0Histo = nullptr;
    TProfile* hT0Histo = new TProfile();
mCalibInFile->cd();

    LOG_INFO << "updateCalib() - setting calibration mode " << iCalibLoop[ mCalibStep ] << " for calibration step " << mCalibStep << endm;

    switch( iCalibMode ){
    case 1: // update position and ToT calibration
        LOG_INFO << "updateCalib() - actually running calibration step " << mCalibStep << " in calibration mode " << iCalibLoop[ mCalibStep  ] << endm;

		if(!mCalibInFile){
			LOG_INFO << "no input calibration file found!"<<endm;
			return;
			}
        mCalibInFile->cd();


        for( int sector = eTofConst::sectorStart; sector <= eTofConst::sectorStop; sector++ ) {
            for( int plane = eTofConst::zPlaneStart; plane <= eTofConst::zPlaneStop; plane++ ) {
                for( int counter = eTofConst::counterStart; counter <= eTofConst::counterStop; counter++ ) {
                    // update position offsets ------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_Pos_pfx", sector, plane, counter);
                    hOldPosProfile = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    hPosProfile->Reset();
                    hPosProfile->SetBins( eTofConst::nStrips, 0, eTofConst::nStrips );
                    hPosProfile->SetName( "temp_pos" ); // set temporary name to avoid confusion with old histogram

                    std::string histName_RpcCluPosition = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Pos";

                    if( hOldPosProfile != nullptr ) {
                        mCalibOutFile->cd();
                        LOG_INFO << "updateCalib() - adding histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        for( int iBin = 0 ; iBin<=eTofConst::nStrips; iBin++ ) { // do adding bin by bin since TProfile::Add merges the underlying histograms

                            hPosProfile->Fill( iBin, hOldPosProfile->GetBinContent( iBin+1 ) + ( mHistograms2d.at( histName_RpcCluPosition )->ProfileX()->GetBinContent( iBin+1 ) ),
															( mHistograms2d.at( histName_RpcCluPosition )->ProfileX()->GetEntries()/mHistograms2d.at( histName_RpcCluPosition )->ProfileX()->GetNbinsX() ) );
									 //use weighting based on average histo bin weight so that corrections from previous iterations that lead to empty bins in current iterations are not disregarded in merger PW
                            LOG_DEBUG << " 	updateCalib() -  filling counter "<<sector << plane << counter<<" Bin " << iBin <<endm;
									 LOG_DEBUG << "		with content of previous iteration: " << hOldPosProfile->GetBinContent( iBin+1 ) << endm;
                            LOG_DEBUG << " 	and current correction: " << mHistograms2d.at( histName_RpcCluPosition )->ProfileX()->GetBinContent( iBin+1 ) << endm;
                            LOG_DEBUG << " 	total correction: " << hOldPosProfile->GetBinContent( iBin+1 ) +mHistograms2d.at( histName_RpcCluPosition )->ProfileX()->GetBinContent( iBin+1 ) << endm;
                            LOG_DEBUG << " 	saved as: " << hPosProfile->GetBinContent( iBin+1 )<< endm;
                            LOG_DEBUG << " 	total weight " << mHistograms2d.at( histName_RpcCluPosition )->ProfileX()->GetBinEntries( iBin+1 ) << endm;
                        }


                        hPosProfile->SetName( hname );
                        if( hPosProfile->GetEntries() > 0 ) hPosProfile->Write( hname, kOverwrite );
                        mCalibInFile->cd();
                    }
                    else {
                        mCalibOutFile->cd();
                        LOG_DEBUG << "updateCalib() - creating new calib histograms " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        hPosProfile = mHistograms2d.at( histName_RpcCluPosition )->ProfileX();
                        hPosProfile->SetName( hname ); // important, otherwise naming confusion with local histogram of further iterations
 								mCalibOutFile->cd();
                        if( hPosProfile->GetEntries() > 0 ) hPosProfile->Write( hname, kOverwrite );
                        mCalibInFile->cd();
                    }

                    // update tot factors ------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_TotDigi_pfx", sector, plane, counter );
                    hOldTotProfile=( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    hTotProfile->Reset();
                    hTotProfile->SetBins( 2 * eTofConst::nStrips, 0, 2 * eTofConst::nStrips );
                    hTotProfile->SetName( "temp_tot" ); // set temporary name to avoid confusion with old histogram

                    std::string histName_RpcDigiTot = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_TotDigi";

                    if( hOldTotProfile != nullptr ) {
                        LOG_DEBUG << "updateCalib() - adding histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;

                        for( int iBin = 0 ; iBin <= 2 * eTofConst::nStrips; iBin++ ) { // do adding bin by bin since TProfile::Add merges the underlying histograms.
                            hTotProfile->Fill( iBin, hOldTotProfile->GetBinContent( iBin+1 ) / 2.0 * mHistograms2d.at( histName_RpcDigiTot )->ProfileX()->GetBinContent( iBin+1 ), 
															( mHistograms2d.at( histName_RpcDigiTot )->GetEntries()/mHistograms2d.at( histName_RpcDigiTot )->GetNbinsX() )  );
									 //use weighting based on average histo bin weight so that corrections from previous iterations that lead to empty bins in current iterations are not disregarded in merger PW
								}
                        hTotProfile->SetName( hname );
								mCalibOutFile->cd();
                        if( hTotProfile->GetEntries() > 0 ) hTotProfile->Write( hname, kOverwrite );
                        mCalibInFile->cd();
                    }
                    else {
                        LOG_DEBUG << "updateCalib() - creating new calib histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        mCalibOutFile->cd();
                        hTotProfile = mHistograms2d.at( histName_RpcDigiTot )->ProfileX();
                        hTotProfile->SetName( hname ); // important, otherwise naming confusion with local histogram of further iterations
                        if( hTotProfile->GetEntries() > 0 ) hTotProfile->Write( hname, kOverwrite );
                        mCalibInFile->cd();
                    }

                    //copy and rescale previous walk histograms. Rescale ------------------------------------------------------------------------------------------------------------------------
                    for( int channel = 0; channel <= 2 * eTofConst::nStrips; channel++ ) {
                        hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_Chan%d_Walk_pfx",  sector, plane, counter, channel );
                        hOldWalkProfile = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
                        std::string histName_RpcChanWalk = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Chan" + std::to_string( channel ) + "_Walk";
                        if( hOldWalkProfile != nullptr ) { // produces trash.
                            LOG_INFO << "updateCalib() - adding histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                            //hOldWalkProfile->Scale(1. / mHistograms2d.at( histName_RpcChanWalk )->GetEntries() ); // rescale in order to not mess up weigthing in hadd, since the input histogram is a combination from walk histograms from multiple files!
                            hOldWalkProfile->SetName( hname );
 									 mCalibOutFile->cd(); 
                             if( hOldWalkProfile->GetEntries() > 0 ) hOldWalkProfile->Write( hname, kOverwrite ); // write to new output-file
                            mCalibInFile->cd();
                        }
                    }

                    //copy and rescale previous T0 histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_T0corr",  sector, plane, counter );
                    hOldT0Histo=( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    if( hOldT0Histo != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        hOldT0Histo->SetName( hname );
                        mCalibOutFile->cd(); 
                        if( hOldT0Histo->GetEntries() > 0 ) hOldT0Histo->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }
                }
            }
        }

        mCalibOutFile->cd();
        mCalibStep++;
        calibSave.SetUniqueID( mCalibStep );
        calibSave.Write( "mCalibStep" );

        mCalibOutFile->Close();
        mCalibInFile->Close();
        break;

    case 2: // update channel Walk calibration
        LOG_DEBUG << "StETofQAMaker::updateCalib() - actually running calibration step " << mCalibStep << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
        mCalibInFile->cd();


        for( int sector = eTofConst::sectorStart; sector <= eTofConst::sectorStop; sector++ ) {
            for( int plane = eTofConst::zPlaneStart; plane <= eTofConst::zPlaneStop; plane++ ) {
                for( int counter = eTofConst::counterStart; counter <= eTofConst::counterStop; counter++ ) {
                    for( int channel = 0; channel < 2 * eTofConst::nStrips; channel++ ) {
                        hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_Chan%d_Walk_pfx",  sector, plane, counter, channel );
                        hOldWalkProfile = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
                        hWalkProfile->Reset();
                        hWalkProfile->SetBins( 15, 0, 6 );
                        hWalkProfile->SetName( "temp_walk" ); // set temporary name to avoid confusion with old histogram

                        std::string histName_RpcChanWalk = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Chan" + std::to_string( channel ) + "_Walk";

                        if( hOldWalkProfile != nullptr ) {
                            LOG_INFO << "updateCalib() - adding histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;

                            for( int iBin = 0 ; iBin<16; iBin++ ) { // do adding bin by bin since TProfile::Add merges the underlying histograms.
                                // TODO: Make binning consistent variable

										 if (hWalkProfile->GetBinCenter( iBin ) >= 15)
										 { //only correct if enough entries per bin are available.
                                hWalkProfile->Fill( hWalkProfile->GetBinCenter( iBin ), 
																	 hOldWalkProfile->GetBinContent(iBin) + mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetBinContent( iBin ),
																	( mHistograms2d.at( histName_RpcChanWalk )->GetEntries()/mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetNbinsX() ) );
                              //use weighting based on average histo bin weight so that corrections from previous iterations that lead to empty bins in current iterations are not disregarded in merger PW 
										 }
										 else
										 {
                                hWalkProfile->Fill( hWalkProfile->GetBinCenter( iBin ), 
																	 hOldWalkProfile->GetBinContent(iBin) + mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetMean( 2 ),
																	hOldWalkProfile->GetEntries()/hOldWalkProfile->GetNbinsX() ) ;											 
										 }
                                if( hname == "calib_Sector18_ZPlane3_Det3_Chan3_Walk_pfx" ) {
                                    LOG_DEBUG << "updateCalib() - iteration " << mCalibStep << " correction for bin " << iBin << " consists of old content ";
                                    LOG_DEBUG << hOldWalkProfile->GetBinContent( iBin ) << " and further correction " << mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetBinContent( iBin );
                                    LOG_DEBUG << " bin center: " << mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetBinCenter( iBin ) << endm;
                                }
                            }

                            hWalkProfile->SetName( hname );
                            mCalibOutFile->cd();
                            if( hWalkProfile->GetEntries() > 0 ) hWalkProfile->Write( hname, kOverwrite );
                            mCalibInFile->cd();
                        }
                        else { //TODO: maybe adjust to make sure it is only filled when enough statistics is available in each bin. PW
                            LOG_DEBUG << "StETofQAMaker::updateCalib() - creating new calib histograms " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;

                            for( int iBin = 0 ; iBin<16; iBin++ ) { // do adding bin by bin since TProfile::Add merges the underlying histograms.
                                // TODO: Make binning consistent variable

										 if (hWalkProfile->GetBinCenter( iBin ) >= 15)
										 { //only correct if enough entries per bin are available.
                                hWalkProfile->Fill( hWalkProfile->GetBinCenter( iBin ), 
																	mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetBinContent( iBin ),
																	( mHistograms2d.at( histName_RpcChanWalk )->GetEntries()/mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetNbinsX() ) );
                              //use weighting based on average histo bin weight so that corrections from previous iterations that lead to empty bins in current iterations are not disregarded in merger PW 
										 }
										 else
										 {
                                hWalkProfile->Fill(  mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetMean( 2 ),
																	( mHistograms2d.at( histName_RpcChanWalk )->GetEntries()/mHistograms2d.at( histName_RpcChanWalk )->ProfileX()->GetNbinsX() ) 																	  );											 
										 }
                            }
                            hWalkProfile->SetName( hname );
                            mCalibOutFile->cd();
                            if( hWalkProfile->GetEntries() > 0 ) hWalkProfile->Write( hname, kOverwrite );
                            mCalibInFile->cd();
                      
                        }
                    }

                    //copy and rescale previous position histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_Pos_pfx", sector, plane, counter );
                    hOldPosProfile = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
           	        std::string histName_RpcCluPosition = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Pos";

                    if( hOldPosProfile != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        //hOldPosProfile->Scale( mHistograms2d.at( histName_RpcCluPosition )->GetEntries()); // rescale in order to not mess up weigthing in hadd, since the input histogram is a combination from walk histograms from multiple files!
                        hOldPosProfile->SetName( hname );
                        mCalibOutFile->cd(); 
                        if( hOldPosProfile->GetEntries() > 0 ) hOldPosProfile->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }

                    //copy and rescale previous tot histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_TotDigi_pfx", sector, plane, counter );
                    hOldTotProfile=( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    std::string histName_RpcDigiTot = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_TotDigi";

                    if( hOldPosProfile != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        //hOldTotProfile->Scale( mHistograms2d.at( histName_RpcDigiTot )->GetEntries() ); // rescale in order to not mess up weigthing in hadd, since the input histogram is a combination from walk histograms from multiple files!
                        hOldTotProfile->SetName( hname );
                        mCalibOutFile->cd(); 
                        if( hOldTotProfile->GetEntries() > 0 ) hOldTotProfile->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }

                    //copy and rescale previous T0 histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_T0corr",  sector, plane, counter );
                    hOldT0Histo=( TProfile* ) mCalibInFile->FindObjectAny( hname );

                    if( hOldT0Histo != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        hOldT0Histo->SetName( hname );
                        mCalibOutFile->cd(); 
                        if( hOldT0Histo->GetEntries() > 0 ) hOldT0Histo->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }
                }
            }
        }

        mCalibOutFile->cd();
        mCalibStep++;
        calibSave.SetUniqueID( mCalibStep );
        calibSave.Write( "mCalibStep" );

        mCalibOutFile->Close();
        mCalibInFile->Close();

        break;

    case 3: //Update T0 calibration 
        LOG_DEBUG << "StETofQAMaker::updateCalib() - actually running calibration step " << mCalibStep << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
        mCalibInFile->cd();
//CURRENTLY PROBLEMATIC WITH HADD!!

        for( int sector = eTofConst::sectorStart; sector <= eTofConst::sectorStop; sector++ ) {
            for( int plane = eTofConst::zPlaneStart; plane <= eTofConst::zPlaneStop; plane++ ) {
                for( int counter = eTofConst::counterStart; counter <= eTofConst::counterStop; counter++ ) {
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_T0corr",  sector, plane, counter );

                    hOldT0Histo = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    hT0Histo->Reset();
                    hT0Histo->SetBins( 1, 0, 1 ); // can probably do this counter wise
                    hT0Histo->SetName( "temp_t0" ); // set temporary name to avoid confusion with old histogram

                    std::string histName_t0corr_mom  = "matched_t0corr_mom_s" + std::to_string( sector ) + "m" + std::to_string( plane ) + "c" + std::to_string( counter );

                    TH1D* hT0corrTemp = mHistograms2d.at( histName_t0corr_mom )->ProjectionY("_temp_proj", 20, -1 ); //no gauss fit here. Just look for the maximum
                    float iStepT0Correction = hT0corrTemp->GetBinCenter(hT0corrTemp->GetMaximumBin());

                    if( hOldT0Histo != nullptr ) { // produces trash.
                        LOG_INFO << "updateCalib() - adding histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;

                      	hT0Histo->Fill( hT0Histo->GetBinCenter( 1 ), hOldT0Histo->GetBinContent(1) + iStepT0Correction ,
													( hT0corrTemp->GetEntries()/hT0corrTemp->GetNbinsX() ));
                        //use weighting based on average histo bin weight so that corrections from previous iterations that lead to empty bins in current iterations are not disregarded in merger PW 
                        hT0Histo->SetName( hname );
                        mCalibOutFile->cd();
                        if( hT0Histo->GetEntries() > 0 ) hT0Histo->Write( hname, kOverwrite );
                        mCalibInFile->cd();
                    }
                    else {
                        LOG_DEBUG << "updateCalib() - creating new calib histograms " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        mCalibOutFile->cd();
                      	hT0Histo->Fill( hT0Histo->GetBinCenter( 1 ), iStepT0Correction,  hT0corrTemp->GetEntries() );

                        hT0Histo->SetName( hname ); // important, otherwise naming confusion with local histogram of further iterations
                       if( hT0Histo->GetEntries() > 0 ) hT0Histo->Write( hname, kOverwrite ); // writes
                        mCalibInFile->cd();
                    }

                    //copy and rescale previous position histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_Pos_pfx", sector, plane, counter);
                    hOldPosProfile = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    std::string histName_RpcCluPosition = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Pos";

                    if( hOldPosProfile != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        //hOldPosProfile->Scale( mHistograms2d.at( histName_RpcCluPosition )->GetEntries() ); // rescale in order to not mess up weigthing in hadd, since the input histogram is a combination from walk histograms from multiple files!

                        hOldPosProfile->SetName( hname );
                        mCalibOutFile->cd(); 
                        if( hOldPosProfile->GetEntries() > 0 ) hOldPosProfile->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }

                    // copy and rescale previous tot histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_TotDigi_pfx", sector, plane, counter );
                    hOldTotProfile=( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    std::string histName_RpcDigiTot = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_TotDigi";

                    if( hOldPosProfile != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        //hOldTotProfile->Scale( mHistograms2d.at( histName_RpcDigiTot )->GetEntries() ); // rescale in order to not mess up weigthing in hadd, since the input histogram is a combination from walk histograms from multiple files!
                        hOldTotProfile->SetName( hname );
                        mCalibOutFile->cd(); 
                        if( hOldTotProfile->GetEntries() > 0 ) hOldTotProfile->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }

                    //copy and rescale previous walk histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    for( int channel = 0; channel < 2 * eTofConst::nStrips; channel++ ) {
                        hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_Chan%d_Walk_pfx",  sector, plane, counter, channel );
                        hOldWalkProfile = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
                        std::string histName_RpcChanWalk = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Chan" + std::to_string( channel ) + "_Walk";
                        if( hOldWalkProfile != nullptr ) {//produces trash.
                            LOG_INFO << "updateCalib() - adding histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                            //hOldWalkProfile->Scale( mHistograms2d.at( histName_RpcChanWalk )->GetEntries() ); // rescale in order to not mess up weigthing in hadd, since the input histogram is a combination from walk histograms from multiple files!
                            hOldWalkProfile->SetName( hname );
									 mCalibOutFile->cd(); 
                            if( hOldWalkProfile->GetEntries() > 0 ) hOldWalkProfile->Write( hname, kOverwrite ); // write to new output-file
                            mCalibInFile->cd();
                        }
                    }
                }
            }
        }

        mCalibOutFile->cd();
        mCalibStep++;
        calibSave.SetUniqueID( mCalibStep );
        calibSave.Write( "mCalibStep" );

        mCalibOutFile->Close();
        mCalibInFile->Close();

        break;

    case 4: // update average cluster Walk calibration
        LOG_DEBUG << "StETofQAMaker::updateCalib: Actually running calibration step " <<mCalibStep<<" in calibration mode "<< iCalibLoop[mCalibStep]<<endm;
        mCalibInFile->cd();


        for( int sector = eTofConst::sectorStart; sector <= eTofConst::sectorStop; sector++ ) {
            for( int plane = eTofConst::zPlaneStart; plane <= eTofConst::zPlaneStop; plane++ ) {
                for( int counter = eTofConst::counterStart; counter <= eTofConst::counterStop; counter++ ) {
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_AvCluWalk_pfx",  sector, plane, counter );
                    hOldWalkProfile = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    hWalkProfile->Reset();
                    hWalkProfile->SetBins( 25, 0, 10 );
                    hWalkProfile->SetName( "temp_walk" ); // set temporary name to avoid confusion with old histogram

                    std::string histName_RpcCluAvWalk = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Walk";


                    if( hOldWalkProfile != nullptr ) { // produces trash.
                        LOG_INFO << "updateCalib() - adding histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;

                        for( int iBin = 0 ; iBin<26; iBin++ ) { // do adding bin by bin since TProfile::Add merges the underlying histograms.
                            //TODO: Make binning consistent variable
                            hWalkProfile->Fill( hWalkProfile->GetBinCenter( iBin ), hOldWalkProfile->GetBinContent(iBin) + mHistograms2d.at( histName_RpcCluAvWalk )->ProfileX()->GetBinContent( iBin ), 
																( mHistograms2d.at( histName_RpcCluAvWalk )->GetEntries()/mHistograms2d.at( histName_RpcCluAvWalk )->ProfileX()->GetNbinsX() ));


                            if( hname == "calib_Sector18_ZPlane3_Det3_AvCluWalk_pfx" ) {
                                LOG_DEBUG << "updateCalib() - iteration " << mCalibStep << " correction for bin " << iBin << " consists of old content ";
                                LOG_DEBUG << hOldWalkProfile->GetBinContent( iBin ) << " and further correction " << mHistograms2d.at( histName_RpcCluAvWalk )->ProfileX()->GetBinContent( iBin );
                                LOG_DEBUG << " bin center: " << mHistograms2d.at( histName_RpcCluAvWalk )->ProfileX()->GetBinCenter( iBin ) << endm;
                            }
                        }

                        hWalkProfile->SetName( hname );
                        mCalibOutFile->cd();
                        if( hOldWalkProfile->GetEntries() > 0 ) hWalkProfile->Write( hname, kOverwrite );
                        mCalibInFile->cd();
                    }
                    else { // works
                        LOG_DEBUG << "updateCalib() - creating new calib histograms " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        mCalibOutFile->cd();
                        hWalkProfile = mHistograms2d.at( histName_RpcCluAvWalk )->ProfileX();
                        hWalkProfile->SetName( hname ); // important, otherwise naming confusion with local histogram of further iterations
                        if( hWalkProfile->GetEntries() > 0 ) hWalkProfile->Write( hname, kOverwrite ); // writes
                        mCalibInFile->cd();
                    }

                    //copy and rescale previous position histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_Pos_pfx", sector, plane, counter);
                    hOldPosProfile = ( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    std::string histName_RpcCluPosition = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_Pos";

                    if( hOldPosProfile != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        //hOldPosProfile->Scale(mHistograms2d.at( histName_RpcCluPosition )->GetEntries()); // rescale in order to not mess up weigthing in hadd, since the input histogram is a combination from walk histograms from multiple files!
                        hOldPosProfile->SetName( hname ); 
                        mCalibOutFile->cd();
                        if( hOldPosProfile->GetEntries() > 0 ) hOldPosProfile->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }

                      //copy and rescale previous tot histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_TotDigi_pfx", sector, plane, counter );
                    hOldTotProfile=( TProfile* ) mCalibInFile->FindObjectAny( hname );
                    std::string histName_RpcDigiTot = "cl_Sector" + std::to_string( sector ) + "_ZPlane" + std::to_string( plane ) + "_Det" + std::to_string( counter ) + "_TotDigi";

                    if( hOldPosProfile != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        //hOldTotProfile->Scale(mHistograms2d.at( histName_RpcDigiTot )->GetEntries()); // rescale in order to not mess up weigthing in hadd, since the input histogram is a combination from walk histograms from multiple files!
                        hOldTotProfile->SetName( hname );
                        mCalibOutFile->cd(); 
                        if( hOldTotProfile->GetEntries() > 0 ) hOldTotProfile->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }

                    //copy and rescale previous T0 histograms. Rescale  ------------------------------------------------------------------------------------------------------------------------
                    hname = Form( "calib_Sector%02d_ZPlane%d_Det%d_T0corr",  sector, plane, counter );
                    hOldT0Histo=( TProfile* ) mCalibInFile->FindObjectAny( hname );

                    if( hOldT0Histo != nullptr ) {
                        LOG_INFO << "updateCalib() - keeping old histogram " << hname << " in calibration mode " << iCalibLoop[ mCalibStep ] << endm;
                        hOldT0Histo->SetName( hname );
                        mCalibOutFile->cd(); 
                        if( hOldT0Histo->GetEntries() > 0 ) hOldT0Histo->Write( hname, kOverwrite ); // write to new output-file
                        mCalibInFile->cd();
                    }
                }
            }
        }

        mCalibOutFile->cd();
        mCalibStep++;
        calibSave.SetUniqueID( mCalibStep );
        calibSave.Write( "mCalibStep" );

        mCalibOutFile->Close();
        mCalibInFile->Close();

        break;

    default:
        LOG_INFO << "updateCalib() - something went wrong: calibration mode " << iCalibLoop[ mCalibStep ] << " not implemented for calibration step " << mCalibStep << endm;
        mCalibInFile->Close();

        break;
    } // end switch
}//::updateCalib
//_____________________________________________________________
