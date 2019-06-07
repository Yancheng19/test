class StMaker;

void bfcRunnerCalib( int nevt, char* chain_opts, char* filename ){

    gROOT->LoadMacro("./bfc.C");
    bfc( -1, chain_opts, filename );
    
    StETofCalibMaker* etofCalib = chain->GetMaker( "etofCalib" );
    etofCalib->setFileNameCalibHistograms( "StETofCalib.hst.root" );

    StMaker::lsMakers( chain );
    chain->PrintInfo();
    chain->Init();
    chain->EventLoop( nevt );
    chain->Finish();

    delete chain;
}
