#include <cmath>

#include "StETofUtil/etofHelperFunctions.h"

#include "SystemOfUnits.h"
#include "PhysicalConstants.h"
#include "phys_constants.h"

#include "StMessMgr.h"
#include "StBTofHeader.h"

#include "StEvent.h"
#include "StEventTypes.h"
#include "Stypes.h"
#include "StThreeVectorD.hh"
#include "StThreeVectorF.hh"
#include "StHelix.hh"
#include "StEventUtilities/StuRefMult.hh"
#include "StPhysicalHelixD.hh"

#include "StETofCollection.h"
#include "StETofHeader.h"
#include "StETofDigi.h"
#include "StETofHit.h"
#include "StETofPidTraits.h"
#include "StBTofCollection.h"
#include "StBTofHit.h"

#include "StTrackNode.h"
#include "StTrackGeometry.h"
#include "StGlobalTrack.h"
#include "StPrimaryTrack.h"
#include "StTrackPidTraits.h"
#include "StPrimaryVertex.h"

#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuDstMaker.h"
#include "StMuDSTMaker/COMMON/StMuTrack.h"
#include "StMuDSTMaker/COMMON/StMuETofHeader.h"
#include "StMuDSTMaker/COMMON/StMuETofDigi.h"
#include "StMuDSTMaker/COMMON/StMuETofHit.h"
#include "StMuDSTMaker/COMMON/StMuETofPidTraits.h"
#include "StMuDSTMaker/COMMON/StMuBTofHit.h"
#include "StMuDSTMaker/COMMON/StMuPrimaryVertex.h"
#include "StVpdCalibMaker/StVpdCalibMaker.h"

#include "StETofUtil/StETofConstants.h"

#include "tables/St_etofSignalVelocity_Table.h"



// *******************************************************************************************************
//
double
calculateVpdTstart( const StBTofHeader* btofHeader, const double& pVtxZ, const double& bTofClockRange )
{
    if( !btofHeader ) {
        LOG_WARN << "calculateVpdTstart():  -- no bTof header. Skip start time calculation." << endm;
        return 0.;
    }


    const int nVpd = 19; // number of VPD tubes on each side of STAR


    int nWest = btofHeader->numberOfVpdHits( west );
    int nEast = btofHeader->numberOfVpdHits( east );

    double vpdLeTime[ 2 * nVpd ];

    // check if bTof header is filled with useful information
    if( fabs( btofHeader->vpdVz() ) > 200. ) {
        LOG_DEBUG << "calculateVpdTstart(): no valid Vpd data in the bTOF header " << endm;
        return 0.;
    }
    else {
        LOG_DEBUG << "calculateVpdTstart(): Vpd vertex is at: " << btofHeader->vpdVz() << endm;
    }


    double tMean   = 0.;
    int nTubes     = 0;
    int nWestTubes = 0;
    int nEastTubes = 0;

    // west side
    for( int i=0; i< nVpd; i++ ) {
        vpdLeTime[ i ] = btofHeader->vpdTime( west, i+1 );
        if( vpdLeTime[ i ] > 0. ) {
            tMean = updateCyclicRunningMean( tMean, vpdLeTime[ i ], nTubes, bTofClockRange );
            nWestTubes++;
            LOG_DEBUG << "calculateVpdTstart(): loading VPD west tubeId = " << i+1 << " time " << vpdLeTime[ i ] << endm;
        }
    }

    // east side
    for( int i=0; i< nVpd; i++ ) {
        vpdLeTime[ i + nVpd ] = btofHeader->vpdTime( east, i+1 );
        if( vpdLeTime[ i + nVpd ] > 0. ) {
            tMean = updateCyclicRunningMean( tMean, vpdLeTime[ i + nVpd ], nTubes, bTofClockRange );
            nEastTubes++;
            LOG_DEBUG << "calculateVpdTstart(): loading VPD east tubeId = " << i+1 << " time " << vpdLeTime[ i + nVpd ] << endm;
        }
    }


    if( nWest != nWestTubes || nEast != nEastTubes ) {
        LOG_INFO << "calculateVpdTstart(): number of fired Vpd tubes not correct ... " << endm;
        //return 0.;
    }

    
    double tstart = 0.;
    if( nWestTubes + nEastTubes ) {
        tstart = tMean;

        if( fabs( pVtxZ ) < 200. ) {
            LOG_DEBUG << "calculateVpdTstart():  --  Vpd start time before pVtxZ ( " << pVtxZ << " ) correction: " << tstart << endm;            

            tstart -= ( ( nEastTubes - nWestTubes ) * pVtxZ / ( c_light * nanosecond ) ) / ( nWestTubes + nEastTubes );
        }

    }


    if( tstart > bTofClockRange ) {
        tstart -= bTofClockRange;
    }
    else if( tstart < 0. ) {
        tstart += bTofClockRange;
    }

    LOG_INFO << "calculateVpdTstart(): -- sum: " << tMean << " nWest: " << nWest << " nEast: " << nEast;
    LOG_INFO << "  --  Vpd start time: " << tstart << endm;

    return tstart;
}    
// *******************************************************************************************************

double
updateCyclicRunningMean( const double& mean, const double& value, int& count, const double& bTofClockRange )
{
    double valIn = value;
    if( mean - value < -0.9 * bTofClockRange ) {
        valIn -= bTofClockRange;  
    } 
    else if( mean - value > 0.9 * bTofClockRange ) {
        valIn += bTofClockRange;
    }


    double scaling = 1. / ( count + 1. );

    double valOut  = valIn * scaling + mean * ( 1. - scaling );
    count++;

    //LOG_INFO << "old mean: " << mean << "  scaling: " << scaling << " value in: " << valIn << "  new mean: " << valOut << endm;
    
    return valOut;
}



// *******************************************************************************************************
// calculate pathlength of a helix between two points 
double tofPathLength(const StThreeVectorD*  beginPoint, const StThreeVectorF* endPoint, const double curvature){
  double x,y,z;
  x = (double)beginPoint->x();
  y = (double)beginPoint->y();
  z = (double)beginPoint->z();
  StThreeVector<double> bp (x,y,z);
  x = (double)endPoint->x();
  y = (double)endPoint->y();
  z = (double)endPoint->z();
  StThreeVector<double> ep(x,y,z);
  return tofPathLength(&bp,&ep,curvature);
}


double tofPathLength(const StThreeVectorF* beginPoint, const StThreeVectorD* endPoint, const double curvature){
  double x,y,z;
  x = (double)beginPoint->x();
  y = (double)beginPoint->y();
  z = (double)beginPoint->z();
  StThreeVector<double> bp (x,y,z);
  x = (double)endPoint->x();
  y = (double)endPoint->y();
  z = (double)endPoint->z();
  StThreeVector<double> ep(x,y,z);
  return tofPathLength(&bp,&ep,curvature);
}

double tofPathLength(const StThreeVectorF* beginPoint, const StThreeVectorF* endPoint, const double curvature){
  double x,y,z;
  x = (double)beginPoint->x();
  y = (double)beginPoint->y();
  z = (double)beginPoint->z();
  StThreeVector<double> bp (x,y,z);
  x = (double)endPoint->x();
  y = (double)endPoint->y();
  z = (double)endPoint->z();
  StThreeVector<double> ep(x,y,z);
  return tofPathLength(&bp,&ep,curvature);
}

// ----------------------------------------------------------------------------------------------
double tofPathLength(const StThreeVector<double>*  beginPoint, const StThreeVector<double>* endPoint, const double curvature){

  double xdif =  endPoint->x() - beginPoint->x();
  double ydif =  endPoint->y() - beginPoint->y();
  
  double C = ::sqrt(xdif*xdif + ydif*ydif);
  double s_perp = C;
  if (curvature){
    double R = 1/curvature;
    s_perp = 2*R * asin(C/(2*R));
  }

  double s_z = fabs(endPoint->z() - beginPoint->z());
  double value = ::sqrt(s_perp*s_perp + s_z*s_z);

  return(value);
}


// *******************************************************************************************************
// calculate expected time of flight of particle hypothesis
double
expectedTimeOfFlight( const double& pathLength, const double& momentum, const double& mass )
{
    double inverseBeta = sqrt( 1 + mass * mass / ( momentum * momentum ) );

    return pathLength * centimeter * ( 1. / c_light ) * inverseBeta / nanosecond;
}
// *******************************************************************************************************



double 
calculateNoVpdTstart(const StETofCollection *etofColl, const StPrimaryVertex *pVtx)
{
    double tstart = -9999.;
    if(!etofColl) return 0;
    
    const StSPtrVecETofHit &etofHits = etofColl->etofHits();
    Int_t nCan = 0;
    Double_t tSum = 0.;
    Double_t t0[5000];
    memset(t0, 0., sizeof(t0));
    for(size_t i=0;i<etofHits.size();i++) {
        StETofHit *aHit = dynamic_cast<StETofHit*>(etofHits[i]);
        if(!aHit) continue;
       
            StGlobalTrack *gTrack = dynamic_cast<StGlobalTrack*>(aHit->associatedTrack());
            if(!gTrack) continue;
            StPrimaryTrack *pTrack = dynamic_cast<StPrimaryTrack*>(gTrack->node()->track(primary));
            if(!pTrack) continue;
            if(pTrack->vertex() != pVtx) continue;
            StThreeVectorF mom = pTrack->geometry()->momentum();
            double ptot = mom.mag();
            
            // use lose cut for low energies to improve the efficiency - resolution is not a big issue
            if(ptot<0.2 || ptot>0.6) continue;
            
            static StTpcDedxPidAlgorithm PidAlgorithm;
            static StPionPlus* Pion = StPionPlus::instance();
            const StParticleDefinition* pd = pTrack->pidTraits(PidAlgorithm);
            double nSigPi = -999.;
            if(pd) {
                nSigPi = PidAlgorithm.numberOfSigma(Pion);
            }
            
            if( fabs(nSigPi)>2.0 ) continue;
            
            const StPtrVecTrackPidTraits& theTofPidTraits = pTrack->pidTraits(kTofId);
            if(!theTofPidTraits.size()) continue;
            
            StTrackPidTraits *theSelectedTrait = theTofPidTraits[theTofPidTraits.size()-1];
            if(!theSelectedTrait) continue;
            
            StETofPidTraits *pidTof = dynamic_cast<StETofPidTraits *>(theSelectedTrait);
            if(!pidTof) continue;
            
            double tot = aHit->totalTot(); // ns
            double tof = aHit->time();
            
        
            StThreeVectorF primPos = pVtx->position();
            StPhysicalHelixD helix = pTrack->geometry()->helix();
            double L = tofPathLength(&primPos, &pidTof->position(), helix.curvature());
            double tofPi = L*sqrt(M_PION_PLUS*M_PION_PLUS+ptot*ptot)/(ptot*(C_C_LIGHT/1.e9));
            
            tSum += tof - tofPi;
            t0[nCan] = tof - tofPi;
            nCan++;
            
        
        
    }
    
    if(nCan<=0) {
        tstart = -9999.;
        return tstart;
    }
    
    Int_t nTzero = nCan;
    if(nCan>1) { // remove hits too far from others
        for(int i=0;i<nCan;i++) {
            double tdiff = t0[i] - (tSum-t0[i])/(nTzero-1);
            if(fabs(tdiff)>5.0) {
                tSum -= t0[i];
                nTzero--;
            }
        }
    }
    
    
    tstart = nTzero>0 ? tSum / nTzero : -9999.;
    
    return tstart;

}



double calculateNoVpdTstart(const StMuDst *muDst, const StMuPrimaryVertex *pVtx){

    double tstart = -9999.;
    if(!muDst) return 0;
    
    Int_t nETofHits = muDst->numberOfETofHit();
    Int_t nCan = 0;
    Double_t tSum = 0.;
    Double_t t0[5000];
    memset(t0, 0., sizeof(t0));
    for(int i=0;i<nETofHits;i++) {
        StMuETofHit *aHit = (StMuETofHit*)muDst->etofHit(i);
        if(!aHit) {
            continue;
        }
     
            StMuTrack *gTrack = aHit->globalTrack();
            if(!gTrack) continue;
            StMuTrack *pTrack = aHit->primaryTrack();
            if(!pTrack) continue;
            StMuPrimaryVertex *aVtx = muDst->primaryVertex(pTrack->vertexIndex());
            if(aVtx != pVtx) continue;
            StThreeVectorF mom = pTrack->momentum();
            double ptot = mom.mag();
            
            // For low energies, lose cut to improve the efficiency in peripheral collisions - resolution should be not a big issue
            if(ptot<0.2 || ptot>0.6) continue;
            double nSigPi = pTrack->nSigmaPion();

            if( fabs(nSigPi)>2. ) continue;

            
            StMuBTofPidTraits pidTof = pTrack->btofPidTraits();
            
            
            double tot = aHit->totalTot(); // ns
            double tof = aHit->time();
            
            StThreeVectorF primPos = pVtx->position();
            StPhysicalHelixD helix = pTrack->helix();
            double L = tofPathLength(&primPos, &pidTof.position(), helix.curvature());
            double tofPi = L*sqrt(M_PION_PLUS*M_PION_PLUS+ptot*ptot)/(ptot*(C_C_LIGHT/1.e9));
            
            tSum += tof - tofPi;
            t0[nCan] = tof - tofPi;
            nCan++;
            
      
        
    }
    
    if(nCan<=0) {
        tstart = -9999.;
        return 0;
    }
    
    Int_t nTzero = nCan;
    if(nCan>1) { // remove hits too far from others
        for(int i=0;i<nCan;i++) {
            double tdiff = t0[i] - (tSum-t0[i])/(nTzero-1);
            if(fabs(tdiff)>5.0) {
                tSum -= t0[i];
                nTzero--;
            }
        }
    }
    
    
    tstart = nTzero>0 ? tSum / nTzero : -9999.;
    
    return tstart;


}

// *******************************************************************************************************
/*
double 
calculateBTofTstart( const StEvent* event, const double& bTofClockRange = 51200. )
{
    LOG_INFO << "calculateBTofTstart():  -- loading bTof data from bTof collection" << endm;

    const StBTofCollection* btofCollection = nullptr;
    const StPrimaryVertex*  pVtx           = nullptr;

    if ( event ) {
        btofCollection = ( StBTofCollection* ) event->btofCollection();
        pVtx           = ( StPrimaryVertex* )  event->primaryVertex();
    }
    else {
        LOG_WARN << "No StEvent found by calculateBTofTstart" << endm;
        return 0.;
    }

    StBTofHeader* btofHeader = nullptr; 

    if ( btofCollection ) {
        btofHeader = btofCollection->tofHeader();
    }
    else {
        LOG_WARN << "No StBTofCollection found by calculateBTofTstart" << endm;
        return 0.;
    }
    
    if ( !pVtx ) {
        LOG_WARN << "No StPrimaryVertex found by calculateBTofTstart" << endm;
        return 0.;
    }

    double tstart = -9999.;


    const StSPtrVecBTofHit &btofHits = btofCollection->tofHits();
    
    int nCand = 0;

    double tSum = 0.;

    double t0[ 5000 ];

    memset( t0, 0., sizeof( t0 ) );

    int nTray = 120;

    for( size_t i=0; i<btofHits.size(); i++ ) {
        StBTofHit *aHit = dynamic_cast< StBTofHit* > ( tofHits[ i ] );
        if( !aHit ) continue;

        int trayId = aHit->tray();

        if( trayId > 0 && trayId <= nTray ) {
            StGlobalTrack *gTrack = dynamic_cast< StGlobalTrack* >( aHit->associatedTrack() );
            if( !gTrack ) continue;
            
            StPrimaryTrack *pTrack = dynamic_cast< StPrimaryTrack* >( gTrack->node()->track( primary ) );
            if( !pTrack ) continue;

            if( pTrack->vertex() != pVtx ) continue;
            StThreeVectorF mom = pTrack->geometry()->momentum();
            double ptot = mom.mag();
            

            if( ptot<0.2 ) continue;
            
            static StTpcDedxPidAlgorithm PidAlgorithm;
            static StPionPlus*   Pion   = StPionPlus::instance();
            static StProtonPlus* Proton = StProtonPlus::instance();

            const StParticleDefinition* pd = pTrack->pidTraits( PidAlgorithm );
            double nSigPi = -999.;
            double nSigP  = -999.;

            if( pd ) {
                nSigPi = PidAlgorithm.numberOfSigma( Pion   );
                nSigP  = PidAlgorithm.numberOfSigma( Proton );
            }
            

            bool isPion     = false;
            bool isProton   = false;
            if ( ptot < 0.6 && fabs( nSigPi ) < 2.0 ) isPion   = true;
            if ( ptot < 0.9 && fabs( nSigP  ) < 2.0 ) isProton = true;
            if ( !isPion && !isProton ) continue;

            
            const StPtrVecTrackPidTraits& theTofPidTraits = pTrack->pidTraits( kTofId );
            if( !theTofPidTraits.size() ) continue;
            
            StTrackPidTraits *theSelectedTrait = theTofPidTraits[ theTofPidTraits.size() - 1 ];
            if( !theSelectedTrait ) continue;
            
            StBTofPidTraits *pidTof = dynamic_cast< StBTofPidTraits* > ( theSelectedTrait );
            if( !pidTof ) continue;
            
            double tot = aHit->tot(); // ns
            double tof = aHit->leadingEdgeTime();
            double zhit = pidTof->zLocal();
            
            int moduleChan = ( aHit->module() - 1 ) * 6 + ( aHit->cell() - 1 );
            
            double tofcorr = tofAllCorr( tof, tot, zhit, trayId, moduleChan );  // --> get the for slewing and local z corrected time
            if( tofcorr < 0. ) continue;
            
            StThreeVectorF primPos = pVtx->position();

            StPhysicalHelixD helix = pTrack->geometry()->helix();

            double L = tofPathLength( &primPos, &pidTof->position(), helix.curvature() );

            double tofPi = L * sqrt( M_PION_PLUS   * M_PION_PLUS   + ptot * ptot ) / ( ptot * ( C_C_LIGHT / 1.e9 ) );
            double tofP  = L * sqrt( M_PROTON_PLUS * M_PROTON_PLUS + ptot * ptot ) / ( ptot * ( C_C_LIGHT / 1.e9 ) );
            
            double tofUse = tofPi;
            if ( isProton ) tofUse = tofP;

            tSum       += tofcorr - tofUse;
            t0[ nCan ]  = tofcorr - tofUse;

            
            nCan++;    
        }
        
    }
    
    if( nCan <= 0 ) {
        tstart = -9999.;
        return 0.;
    }
    
    int nTzero = nCan;
    if( nCan > 1 ) { // remove hits too far from others
        for( int i=0; i<nCan; i++ ) {
            double tdiff = t0[ i ] - ( tSum - t0[ i ] ) / ( nTzero - 1 );
            if( fabs( tdiff ) > 5.0 ) {
                tSum -= t0[ i ];
                nTzero--;
            }
        }
    }
    
    tstart = nTzero>0 ? tSum / nTzero : -9999.;
    
    return tstart;
}
*/
