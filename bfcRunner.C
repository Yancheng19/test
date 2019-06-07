class StMaker;

void bfcRunner( int nevt, char* chain_opts, char* filename ){
//void bfcRunner( int start, int stop, char* chain_opts, char* filename ){

    gROOT->LoadMacro("./bfc.C");
    bfc( -1, chain_opts, filename );
    
    StETofMatchMaker* etofMatch = chain->GetMaker( "etofMatch" );
    etofMatch->setIsSim( true );

    StMaker::lsMakers(chain);
    chain->PrintInfo();
    chain->Init();
    chain->EventLoop(nevt);
    //chain->EventLoop(start, stop);
    chain->Finish();

    delete chain;
}
