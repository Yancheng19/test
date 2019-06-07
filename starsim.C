// macro to instantiate the Geant3 from within
// STAR  C++  framework and get the starsim prompt
// To use it do
//  root4star starsim.C

TF1 *fPt  = 0;
TF1 *fY   = 0;
TF1 *fPhi = 0;

class St_geant_Maker;
St_geant_Maker *geant_maker = 0;

class StarGenEvent;
StarGenEvent   *event       = 0;

class StarPrimaryMaker;
StarPrimaryMaker *primary = 0;

class StarKinematics;
StarKinematics *kinematics = 0;

TH1 * hpt = 0;
TH1 * heta = 0;
TH1 * hphi = 0;

// ----------------------------------------------------------------------------
void geometry( TString tag, Bool_t agml=true )
{
  TString cmd = "DETP GEOM "; cmd += tag;
  if ( !geant_maker ) geant_maker = (St_geant_Maker *)chain->GetMaker("geant");
  geant_maker -> LoadGeometry(cmd);
}
// ----------------------------------------------------------------------------
void command( TString cmd )
{
  if ( !geant_maker ) geant_maker = (St_geant_Maker *)chain->GetMaker("geant");
  geant_maker -> Do( cmd );
}
// ----------------------------------------------------------------------------
void trig( Int_t n=1 )
{
  for ( Int_t i=0; i<n; i++ ) {
    chain->Clear(); 
   
    // make sure to hit the VPD!
    //kinematics->Kine( 10, "mu-", 3., 5., -5.2, -4.1 );
    //kinematics->Kine( 10, "mu+", 3., 5., -5.2, -4.1 );
    
    //kinematics->Kine( 10, "mu-", 3., 5.,  4.1,  5.2 );
    //kinematics->Kine( 10, "mu+", 3., 5.,  4.1,  5.2 );
 
    // mid-rapidity particles for vertex reconstruction
    kinematics->Kine( 5, "mu-", 0.5, 5.0, -0.7, 0.7 );
    kinematics->Kine( 5, "mu+", 0.5, 5.0, -0.7, 0.7 );

    // particles in eTof acceptance
    //kinematics->Kine( 5, "mu+", 1.0, 3.0, -1.7, -1.0 );
    //kinematics->Kine( 5, "mu-", 1.0, 3.0, -1.7, -1.0 );
    
    kinematics->Kine( 2, "e+",         0.16, 3.0, -1.7, -1.0 );
    kinematics->Kine( 2, "e-",         0.16, 3.0, -1.7, -1.0 );
    kinematics->Kine( 2, "pi+",        0.16, 3.0, -1.7, -1.0 );
    kinematics->Kine( 2, "pi-",        0.16, 3.0, -1.7, -1.0 );
    kinematics->Kine( 2, "K+",         0.16, 3.0, -1.7, -1.0 );
    kinematics->Kine( 2, "K-",         0.16, 3.0, -1.7, -1.0 );
    kinematics->Kine( 2, "proton",     0.16, 3.0, -1.7, -1.0 );
    kinematics->Kine( 2, "antiproton", 0.16, 3.0, -1.7, -1.0 );
  
    // kinematics->Dist( 12, "pi+", hpt, heta, hphi );
    // kinematics->Dist( 12, "pi-", hpt, heta, hphi );

    chain->Make();
  }
}

// ----------------------------------------------------------------------------
void Pythia6( Int_t tune=320 )
{
  
  gSystem->Load( "libPythia6_4_28.so");


  StarPythia6 *pythia6 = new StarPythia6("pythia6");

  pythia6->SetFrame("CMS", 200.0 );
  pythia6->SetBlue("proton");
  pythia6->SetYell("proton");
  if ( tune ) pythia6->PyTune( tune );

  PySubs_t &pysubs = pythia6->pysubs();
  pysubs._ckin[2]=50;

  primary->AddGenerator(pythia6);
}

// ----------------------------------------------------------------------------
void starsim( Int_t nevents=50, Int_t rngSeed=1234 )
{ 

  gROOT->ProcessLine(".L bfc.C");
  {
    //TString simple = "y2018a geant gstar agml usexgeom";
    TString simple = "y2019 geant big gstar agml usexgeom";
    bfc(0, simple );
  }

  gSystem->Load( "libVMC.so");

  gSystem->Load( "StarGeneratorUtil.so" );
  gSystem->Load( "StarGeneratorEvent.so" );
  gSystem->Load( "StarGeneratorBase.so" );

  gSystem->Load( "libMathMore.so"   );  
  gSystem->Load( "xgeometry.so"     );

  // Setup RNG seed and map all ROOT TRandom here
  StarRandom::seed( rngSeed );
  StarRandom::capture();
  
  //
  // Create the primary event generator and insert it
  // before the geant maker
  //
  //  StarPrimaryMaker *
  primary = new StarPrimaryMaker();
  primary->SetFileName( "generator.starsim.root");
  chain->AddBefore( "geant", primary );

  //
  // Setup an event generator
  //
  // Pythia6( );

  gSystem->Load( "libKinematics.so");
  kinematics = new StarKinematics();

  primary->AddGenerator(kinematics);

  //TFile *fdist = new TFile( "run15_mtd_matched_Pt_PDF.root" );
  //hpt = (TH1*)fdist->Get( "hpt" );
  //heta = (TH1*)fdist->Get( "heta" );
  //hphi = (TH1*)fdist->Get( "hphi" );

  //
  // Setup cuts on which particles get passed to geant for
  //   simulation.  
  //
  // If ptmax < ptmin indicates an infinite ptmax.
  // ptmin will always be the low pT cutoff.
  //
  //                    ptmin  ptmax
  primary->SetPtRange(0.0,  -1.0);         // GeV
  //
  // If etamax < etamin, there is no cut in eta.
  // otherwise, particles outside of the specified range are cut.
  //
  //                    etamin etamax
   primary->SetEtaRange( -6., +6. );
  //
  //  phirange will be mapped into 0 to 2 pi internally.
  //
  //                    phimin phimax
  primary->SetPhiRange( 0., TMath::TwoPi() );
  
  
  // 
  // Setup a realistic z-vertex distribution:
  //   x = 0 gauss width = 1mm
  //   y = 0 gauss width = 1mm
  //   z = 0 gauss width = 30cm
  // 
  primary->SetVertex( 0.0, 0.0, 0.0 );
  primary->SetSigma( 0.1, 0.1, 0.1 );

  //
  // Initialize primary event generator and all sub makers
  //
  primary -> Init();

  //
  // Setup geometry and set starsim to use agusread for input
  //
  //geometry("y2018a field=-5.0");
  geometry("y2019 field=-5.0");
  command("gkine -4 0");
  command("gfile o generator.starsim.fzd");
  

  // fPt = new TF1( "fPt", "exp(-[0] * 2)" );
  // fPt->SetRange( 0.95, 5.0 ) 

  //
  // Trigger on nevents
  //
  trig( nevents );

  command("call agexit");  // Make sure that STARSIM exits properly
}
// ----------------------------------------------------------------------------

