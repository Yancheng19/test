class Geometry;
Geometry *build = 0;

void loadStarGeometry( const Char_t *mytag="y2009a", Bool_t agml = true )
{
  gEnv->SetValue("Logger.Colors","YES");

  TString tag = mytag;
  gSystem->AddIncludePath(" -IStRoot -Igeom -IStarVMC -IStarVMC/Geometry/macros ");

  // gROOT   -> LoadMacro("Load.C");
  //$$$  Load("libSt_g2t, libStarMagField.so, St_geant_Maker");
  //  Load(".$STAR_HOST_SYS/lib/StarAgmlLib.so");
  gSystem->Load("libGeom.so");
  gSystem->Load("StarRoot.so");
  gSystem->Load("St_base.so");
  gSystem->Load("StUtilities.so");
  gSystem->Load("libPhysics.so");
  gSystem->Load("StarAgmlUtil.so");
  gSystem->Load("StarAgmlLib.so");
  gSystem->Load("libStarGeometry.so");
  gSystem->Load("libGeometry.so");

  if (!mytag) return;

  if (!gMessMgr) gMessMgr = new StMessMgr();

  gErrorIgnoreLevel=9999;

  // ROOT TGeo stacker
  StarTGeoStacker *stacker = new StarTGeoStacker();
  //  stacker -> Debug("TPAD", "position");
  //  stacker -> Debug("TPCM", "position");
  //  stacker -> Debug("TSWH", "position"); 
  //  stacker -> Debug("TPGV", "position");
  //  stacker -> Debug("TSAS", "position");
  //  stacker -> Debug("TWAS", "position");
  //  stacker -> Debug("TSGT", "position");
  AgBlock::SetStacker( stacker );

  // Load the master geometry libraries
  //  gROOT->ProcessLine(".L libGeometry.so");
  //  gROOT->ProcessLine(".L libStarGeometry.so");


  // Instantiate the geometry builder
  build = new Geometry(); 

  // Once the geometry is created we can load in the DB
  // gROOT->ProcessLine(".L StarGeometryDb.C");
  // gROOT->ProcessLine("StarGeometryDb();");

  //
  // Setup a geometry control structure for testing purposes
  //  
  gROOT->ProcessLine(Form(".!mkdir %s",mytag));
  if ( agml )
    {
      if ( !gGeoManager ) new TGeoManager(tag,tag+" | dyson");	
      build -> ConstructGeometry ( tag );
    }
  else
    {
      loadStarsimGeometry( mytag );
      // gROOT->Macro(Form("$STAR/StarDb/VmcGeometry/%s.h",mytag));
      // gROOT->Macro(Form("scripts/%s.h",mytag));
    }

  //  gGeoManager->CloseGeometry();
  gGeoManager->SetVisLevel(99);        

  return;

}