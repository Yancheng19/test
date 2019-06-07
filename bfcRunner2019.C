class StMaker;

void bfcRunner2019( int nevt, char* chain_opts, char* filename ){
//void bfcRunner2019( int start, int stop, char* chain_opts, char* filename ){

    gROOT->LoadMacro("./bfc.C");
    bfc( -1, chain_opts, filename );
    
    StETofCalibMaker* etofCalib = chain->GetMaker( "etofCalib" );
    etofCalib->setFileNameCalibHistograms( "StETofCalib.hst2019.root" );
    
    StETofHitMaker* etofHit = chain->GetMaker( "etofHit" );
    etofHit->setDoQA( true );

    StETofMatchMaker* etofMatch = chain->GetMaker( "etofMatch" );
    etofMatch->setDoQA( true );
    
    StMaker::lsMakers( chain );
    chain->PrintInfo();
    chain->Init();
    chain->EventLoop( nevt );
    //chain->EventLoop( start, stop );
    chain->Finish();

    delete chain;
}
