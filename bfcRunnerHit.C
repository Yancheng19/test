class StMaker;

void bfcRunnerHit( int nevt, char* chain_opts, char* filename ){

    gROOT->LoadMacro("./bfc.C");
    bfc( -1, chain_opts, filename );
    
    StETofHitMaker* etofHit = chain->GetMaker( "etofHit" );
    etofHit->setDoQA( true );
    etofHit->setDebug( true );

    StMaker::lsMakers( chain );
    chain->PrintInfo();
    chain->Init();
    chain->EventLoop( nevt );
    chain->Finish();

    delete chain;
}
