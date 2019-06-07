class StMaker;

void bfcRunnerMatch( int nevt, char* chain_opts, char* filename ){

    gROOT->LoadMacro("./bfc.C");
    bfc( -1, chain_opts, filename );
    
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
