#ifndef ETOFANALYSISMAKER_H
#define ETOFANALYSISMAKER_H

#include "StMaker.h"
#include "TString.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"

class TFile;
class TH1;
class TH2;
class StETofHit;
class StMuDst;
class StMuTrack;
class StMuETofHit;
class StMuDstMaker;
class StETofGeometry;


class ETofAnalysisMaker : public StMaker { 
public:
  ETofAnalysisMaker(StMuDstMaker* maker);       //  Constructor
  virtual          ~ETofAnalysisMaker( );       //  Destructor

  Int_t Init( );                                  //  Initiliaze the analysis tools ... done once
  Int_t InitRun( Int_t );  			// initialize geometry
  Int_t FinishRun( Int_t );  			// clean up geometry
  Int_t Make( );                                  //  The main analysis that is done on each event
  Int_t Finish( );                                //  Finish the analysis, close files, and clean up.

  StETofGeometry*  etofGeometry() const;

  void SetOutputFileName(TString name) {mHistogramOutputFileName = name;} // Make name available to member functions

private:

  StMuDstMaker* mMuDstMaker;                      //  Make MuDst pointer available to member functions
  StMuDst* mMuDst;
  StETofGeometry* mETofGeom;   		// pointer to the ETof geometry utility class

  Int_t mTrackCount;

  TH1F*         hist;                             //  1D Histograms

  TH2F*		refPositionZplane1;		  //  Hit position in the first 2 strips of the ref
  TH2F*		dutPositionZplane1;		  //  With hits in the first 2 strips of the ref, hit position in the last 6 strips of the dut
  TH2F*		refPositionZplane2;
  TH2F*		dutPositionZplane2;
  TH2F*		refPositionZplane3;
  TH2F*		dutPositionZplane3;

  TH1F*		timeDiff1;			  //  time difference between hits both found in ref (2 strips) and dut (6 strips)
  TH1F*		timeDiff2;
  TH1F*		timeDiff3;
 
  TH1F*		xDiff1;				  //  dx between hits both found in ref (2 strips) and dut (6 strips)
  TH1F*		xDiff2;
  TH1F*		xDiff3;

  TH1F*		yDiff1;				  //  dy between hits both found in ref (2 strips) and dut (6 strips)
  TH1F*		yDiff2;
  TH1F*		yDiff3;

  TH2F*		posDiff1;			  //  dxdy between hits both found in ref (2 strips) and dut (6 strips)
  TH2F*		posDiff2;
  TH2F*		posDiff3;

  TH1F*		chi2Distribution1;		  //  chi2 distribution
  TH1F*		chi2Distribution2;
  TH1F*		chi2Distribution3;

  TH1F*		timeDiffMathced1;	 	  //  time difference between hits both found in ref (2 strips) and dut (6 strips) with chi2 cuts
  TH1F*		timeDiffMathced2;
  TH1F*		timeDiffMathced3;

  TF1*          yDiffFit1;		  	  //  gaus fit of the dy difference
  TF1*          yDiffFit2;
  TF1*          yDiffFit3;

  TF1*          timeRawDiffFit1;		  //  gaus fit of the raw time difference
  TF1*          timeRawDiffFit2;
  TF1*          timeRawDiffFit3;

  TF1*          timeDiffFit1;			  //  gaus fit of the time difference
  TF1*          timeDiffFit2;
  TF1*          timeDiffFit3;

  TH2F*   	trackPosDut1;
  TH2F*		trackPosRef1;
  TH2F*   	trackPosDut2;
  TH2F*		trackPosRef2;
  TH2F*   	trackPosDut3;
  TH2F*		trackPosRef3;

  TH1F*		trackCounter;
  TH1F*		trackZPlane;
  TH1F*		trackX;
  TH1F*		trackY;






  TFile*        histogram_output;                 //  Histograms outputfile pointer

  UInt_t        mEventsProcessed;                 //  Number of Events read and processed
  TString       mHistogramOutputFileName;         //  Name of the histogram output file 

  // store hit in the overlapping strip.
  std::vector< std::vector< std::vector< std::vector< StMuETofHit* > > > > mOverlapStripHit; //[mSector][mZPlane][mDet][nDigis]
  std::vector< std::vector< std::vector< StMuTrack* > > > mOverlapStripTrack; //[mSector][mZPlane][nTracks]
  

  ClassDef( ETofAnalysisMaker, 1 )                //  Macro for CINT compatability
    
};

inline StETofGeometry* ETofAnalysisMaker::etofGeometry() const { return mETofGeom; }

#endif















