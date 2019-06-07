 /***************************************************************************
 *
 * $Id: $
 *
 * Author: Florian Seck, November 2018
 ***************************************************************************
 *
 * Description: collection of helper functions needed for start time,
 *              expected time of flight and path length calculation  
 *
 *
 ***************************************************************************
 *
 **************************************************/
#ifndef ETOFHELPERFUNCTIONS_H
#define ETOFHELPERFUNCTIONS_H

class StEvent;
class StMuDst;
class StBTofHeader;

class StPrimaryVertex;
class StETofGeometry;
class StETofCollection;
class StETofHeader;
class StETofPidTraits;
class StMuDst;
class StMuPrimaryVertex;
class StMuETofPidTraits;
#include "StThreeVectorD.hh"
#include "StThreeVector.hh"
#include "StThreeVectorF.hh"



// *******************************************************************************************************
// get tstart from the bTof Header if calculated
double readbackTstart( const StEvent* event );
double readbackTstart( const StEvent* event, const double& bTofClockRange );

// calculate vpd start time manually
double calculateVpdTstart( const StEvent* event );
double calculateVpdTstart( const StMuDst* muDst );

double calculateVpdTstart( const StEvent* event, const double& bTofClockRange );
double calculateVpdTstart( const StMuDst* muDst, const double& bTofClockRange );
double calculateVpdTstart( const StBTofHeader* btofHeader, const double& pVtxZ, const double& bTofClockRange );
double updateCyclicRunningMean( const double& mean, const double& value, int& count, const double& bTofClockRange );



// calculate bTof start time manually
//double calculateBTofTstart( const StEvent* event, const double& bTofClockRange );
//
// *******************************************************************************************************

// *******************************************************************************************************
// calculate helix path length between two points

double tofPathLength(const StThreeVector<double>*, const StThreeVector<double>*, const double);
//double tofPathLength(const StThreeVectorD*, const StThreeVectorD*, const double);
double tofPathLength(const StThreeVectorD*, const StThreeVectorF*, const double);
double tofPathLength(const StThreeVectorF*, const StThreeVectorD*, const double);
double tofPathLength(const StThreeVectorF*, const StThreeVectorF*, const double);
//
// calculate expected time of flight
double expectedTimeOfFlight( const double& pathLength, const double& momentum, const double& mass );
// *******************************************************************************************************


double calculateNoVpdTstart(const StETofCollection *etofColl, const StPrimaryVertex *pVtx);

double calculateNoVpdTstart(const StMuDst *muDst, const StMuPrimaryVertex *pVtx);

#endif
