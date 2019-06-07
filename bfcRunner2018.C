class StMaker;

void bfcRunner2018( int nevt, char* chain_opts, char* filename ){

    gROOT->LoadMacro("./bfc.C");
    bfc( -1, chain_opts, filename );
    
    StETofCalibMaker* etofCalib = chain->GetMaker( "etofCalib" );
    etofCalib->setFileNameCalibHistograms( "StETofCalib.hst2018.root" );

    StETofHitMaker* etofHit = chain->GetMaker( "etofHit" );
    etofHit->setDoQA( true );
    etofHit->setDebug( true );
    
    StETofMatchMaker* etofMatch = chain->GetMaker( "etofMatch" );
    etofMatch->setDoQA( true );
    etofMatch->setDebug( true );

    StMaker::lsMakers( chain );
    chain->PrintInfo();
    chain->Init();
    chain->EventLoop( nevt );
    chain->Finish();

    delete chain;
}
