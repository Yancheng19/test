
class StSPtrVecETofDigi;

void AnalyzeDigis( StSPtrVecETofDigi digis, double& trgGdpbFullTime, TH1F* h_trgWindow ); //, TString outputFileName);

TString chainOpts =  "in event"; // tpcDB etofDat";







void AnalyzeEvents( char* inputDirectory, char* inputFileName )
{
  
  TString _infile = inputDirectory;
  _infile += "/";
  _infile += inputFileName;
/*
  TString outfile = inputFileName;
  outfile.ReplaceAll(".daq","_Levels.txt");
*/

  gROOT->Macro("Load.C");
  //gSystem->Load("StEpdUtil");

  gROOT->LoadMacro("bfc.C");

  bfc( 0, chainOpts, _infile );

  StSPtrVecETofDigi digis;

  TH1F* h_trgWindow = new TH1F( "h_trgWindow", "", 600, -3, 3 );
  
  const int N = 200; // process N events
  for ( int i=0; i<N; i++ ) {

    chain->Clear(); 
    chain->Make(); // in principle should check return status...

    StEvent* event = ( StEvent* ) chain->GetInputDS( "StEvent" );

    if( !event ) {
	 cout << "no event available -> end of file?" << endl;
	 break;
    }

    cout << "\n\n\n\n\n --------- here in the macro -----------\n\n\n i= " << i << "\n\n";
    cout << "Looking at ETof digis in StEvent....\n\n";
    if ( event->etofCollection() == 0 ) {
      cout << "No StETofCollection!\n";
    }
    else {
      StETofCollection* etofCollection = event->etofCollection();
      cout << "StETofCollection pointer: " << etofCollection << endl; 
     
      StETofHeader* header = etofCollection->etofHeader();
      
      double trgGpdbfullTime = 0;

      if( header ) {
	  trgGdpbFullTime = header->trgGdpbFullTime();
	  printf( "gdpb trg time: %18.6f \n", trgGdpbFullTime );
      }
      else {
	 cout << "*** NO ETOF HEADER :( ***" << endl;
      }

  
      digis = etofCollection->etofDigis();
      //      StPtrVecETofDigiIterator it = digis->begin(); I'd rather avoid iterators...
      cout << "StETofCollection size is " << digis.size() << endl;
    }
    cout << "\n\n\n\n\n\n ----------------------------------\n\n\n";
    
    AnalyzeDigis( digis, trgGdpbFullTime, h_trgWindow ); //, outfile );

    cout << "\n\n\n";
  }
 
  h_trgWindow->Draw(); 

  //  new TBrowser();

};


void AnalyzeDigis( StSPtrVecETofDigi digis, double& trgGdpbFullTime, TH1F* h_trgWindow ) { //, TString outputFileName){
    
  for ( unsigned int i=0; i<digis.size(); i++ ){
    StETofDigi* theDigi = digis[ i ];

    printf("Digi (%03d) : rocId = %#06x, Get4ChipId=%02d, ChannelId=%d, ", i, theDigi->rocId(), theDigi->get4Id(), theDigi->elChan());   
    printf("rawTime (ns) = %18.4f,   rawTot (ns) = %6.2f \n", theDigi->rawTime(), theDigi->rawTot());    

    printf("Digi (%03d) : sector=%02d, zplane=%d, counter=%d, strip=%02d, side=%d", i, theDigi->sector(), theDigi->zPlane(), theDigi->counter(), theDigi->strip(), theDigi->side());   
    printf("calibTime (ns) = %18.4f,   calibTot (ns) = %6.2f \n", theDigi->calibTime(), theDigi->calibTot());    
  
    h_trgWindow->Fill( ( theDigi->rawTime() - trgGdpbFullTime ) / 1000. );
  }

}
