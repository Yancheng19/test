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
#ifndef STETOFQAMAKER_H
#define STETOFQAMAKER_H

#include <string>

#include "StMaker.h"

class TH1;
class TH2;

class StEvent;
class StMuDst;

class StETofHeader;
class StETofDigi;
class StETofHit;
class StBTofCollection;
class StMuETofHeader;
class StMuETofDigi;
class StMuETofHit;


class StETofQAMaker: public StMaker {
public:
    /// Default constructor
    StETofQAMaker( const char* name = "etofQa" );

    ~StETofQAMaker();

    Int_t  Init();
    Int_t  InitRun( Int_t );
    Int_t  FinishRun( Int_t );
    Int_t  Finish();
    Int_t  Make();

    void processStEvent();
    void processMuDst();

    void setHistFileName( std::string );
    
    inline void setFileNameSignalVelocity(  const char* fileName )     { mFileNameSignalVelocity  = fileName; }

private:
    // internal subfunctions ------------------------------------------------------------
    void bookHistograms();
    void fillHitHistograms(  StETofHit*  hit,  StETofHeader* header );
    void fillDigiHistograms( StETofDigi* digi, StETofHeader* header );
    void fillBTofHistograms( StBTofCollection* btofColl );

    void fillHitHistograms(   StMuETofHit*  hit,  StMuETofHeader* header );
    void fillDigiHistograms(  StMuETofDigi* digi, StMuETofHeader* header );
    void fillBTofHistograms();

    void initCalib();
    void updateCalib();
    void writeHistograms();

    // internals ------------------------------------------------------------------------
    StEvent*                    mEvent;
    StMuDst*                    mMuDst;

    Bool_t                      mIsStEvent;

    Double_t                    mTimeOffset;
    Double_t                    mTstart;

    UInt_t                      mNCalibDigis;
    UInt_t                      mNHitsInTrgWindow;

    Double_t                    mAverageETofTime;
    
    std::string   mFileNameSignalVelocity;  // name of parameter file for signal velocity
    std::map< UInt_t, Float_t >  mSignalVelocity;  // signal velocities in each detector

    std::map< UInt_t, UInt_t >  mMapRpcCluMult;

    // histograms -----------------------------------------------------------------------
    std::string                 mQAHistFileName;
    std::map< string, TH1* >    mHistograms1d;
    std::map< string, TH2* >    mHistograms2d;

    TFile*                      mCalibInFile;
    TFile*                      mCalibOutFile; // output file has to be run specific
    std::string                 mCalibInHistFileName;
    std::string                 mCalibOutHistFileName;
    UInt_t                      mCalibStep;

    virtual const Char_t *GetCVS() const { static const char cvs[]="Tag $Name:  $Id: built " __DATE__ " " __TIME__ ; return cvs; }
    ClassDef( StETofQAMaker, 1 )
};

#endif
