/***************************************************************************
 *
 * $Id: StETofHitMaker.h,v 1.2 2019/03/08 19:03:35 fseck Exp $
 *
 * Author: Philipp Weidenkaff & Florian Seck, April 2018
 ***************************************************************************
 *
 * Description: StETofHitMaker - class to read the eTofCollection from
 * StEvent and combine digis on both sides of each read-out strip into a hit.
 * The hits on each strip are further merger into clusters.
 * The eTOF collection is filled with hits and written to StEvent.
 *
 ***************************************************************************
 *
 * $Log: StETofHitMaker.h,v $
 * Revision 1.2  2019/03/08 19:03:35  fseck
 * moved QA histograms for clustered hits into separate function
 *
 * Revision 1.1  2019/02/19 19:52:28  jeromel
 * Reviewed code provided by F.Seck
 *
 *
 ***************************************************************************/
#ifndef STETOFHITMAKER_H
#define STETOFHITMAKER_H

#include <string>
#include <vector>
#include <map>

#include "StMaker.h"

class TH1;

class StEvent;
class StMuDst;

class StETofDigi;
class StMuETofDigi;

class StETofHit;
class StMuETofHit;


class StETofHitMaker: public StMaker {
public:
    StETofHitMaker( const char* name = "etofHit" );     // default constructor

    ~StETofHitMaker();


    Int_t  Init();
    Int_t  InitRun(   Int_t );
    Int_t  FinishRun( Int_t );
    Int_t  Finish();
    Int_t  Make();


    void processStEvent();
    void processMuDst();

    /// read hit building parameters from file
    void setFileNameHitParam( const char* fileName );
    void setFileNameSignalVelocity( const char* fileName );


    void setDoQA( const bool doQA );
    void setDebug( const bool debug );



private:
    // internal subfunctions ----------------------------------------------------------------------
    void bookHistograms();
    void setHistFileName();
    void writeHistograms();

    void clearHits( const bool isMuDst ); // clear hits from StEvent / MuDst in afterburner mode
    void clearStorage();

    bool fillStorage( StETofDigi*   aDigi, unsigned int index ); // sorts digis into vectors by detector
    bool fillStorage( StMuETofDigi* aDigi, unsigned int index ); // sorts digis into vectors by detector

    void matchSides();                               // matches digis on opposing sides to form single strip hits

    void mergeClusters( const bool isMuDst );        // merges hits to clusters across multiple strips

    void assignAssociatedHits( const bool isMuDst ); // write associated hits into digis

    double startTime();

    void fillUnclusteredHitQA( const double& tstart );

    void fillHitQA( const bool isMuDst, const double& tstart );

    unsigned int detectorToKey( const unsigned int detectorId );

    // internal containers ------------------------------------------------------------------------
    StEvent*                mEvent;
    StMuDst*                mMuDst;

    std::string   mFileNameHitParam;        // name of parameter file for hit parameters
    std::string   mFileNameSignalVelocity;  // name of parameter file for signal velocity

    // store digis ordered by detectorstrip for side-matching 
    std::map< UInt_t, std::vector< StETofDigi* > > mStoreDigi;  // key: strip index, value: vector of digis

    // link between mStorStDigi and StETofCollection->digis()
    std::map< StETofDigi*, UInt_t > mMapDigiIndex; 

    // store hit ordered by detector for merging.
    std::map< UInt_t, std::vector< StETofHit* > > mStoreHit; // key: detector index, value: vector of hits

    // link between hits and constituent digis
    std::map< StETofHit*, std::vector< UInt_t > > mMapHitDigiIndices;
    
    // map between hit index in MuDsts and constituent digis
    std::map< UInt_t, std::vector< UInt_t > > mMapHitIndexDigiIndices;

    // parameters ---------------------------------------------------------------------------------
    Double_t mMaxYPos;               // maximum absolute Y Pos for side matching
    Double_t mMergingRadius;         // maximum XYT radius[ns] for cluster merging

    std::map< UInt_t, Double_t > mSigVel; // signal velocities in each detector


    // histograms for QA --------------------------------------------------------
    Bool_t                    mDoQA;
    Bool_t                    mDebug;
    std::string               mHistFileName;
    map< std::string, TH1* >  mHistograms;


    virtual const Char_t *GetCVS() const { static const char cvs[]="Tag $Name:  $Id: built " __DATE__ " " __TIME__ ; return cvs; }

    ClassDef( StETofHitMaker, 0 )
};


inline void StETofHitMaker::setFileNameHitParam(       const char* fileName )     { mFileNameHitParam       = fileName; }
inline void StETofHitMaker::setFileNameSignalVelocity( const char* fileName )     { mFileNameSignalVelocity = fileName; }

inline void StETofHitMaker::setDoQA(  const bool doQA )  { mDoQA  = doQA;  }
inline void StETofHitMaker::setDebug( const bool debug ) { mDebug = debug; }

#endif