void plotHitPositions() {
    int run = 20059017;

    TString infile_name = Form( "etofHit_%d.root", run );


    TFile* infile = new TFile( infile_name, "READ" );

    TH2F* hitPos[ 108 ];

    for( size_t i=0; i<108; i++ ) {
        int sector  = ( i / 9 ) + 13;
        int module  = ( i % 9 ) / 3 + 1;
        int counter = ( i % 3 ) + 1;

        hitPos[ i ] = ( TH2F* ) infile->Get( Form( "clusteredHit_pos_s%d_m%d_c%d", sector, module, counter ) );
    }

    //hitPos[ 5 ]->Draw( "colz" );

    //gErrorIgnoreLevel = 3000;

    gStyle->SetOptStat( 110010 );
    // Set stat options
    gStyle->SetStatY( 0.95 );
    // Set y-position (fraction of pad size)
    gStyle->SetStatX( 0.95 );                
    // Set x-position (fraction of pad size)
    gStyle->SetStatW( 0.5 );                
    // Set width of stat-box (fraction of pad size)
    gStyle->SetStatH( 0.1 );                
    // Set height of stat-box (fraction of pad size)


    TCanvas* can[ 12 ];

    for( int i=0; i<12; i++ ) {
        can[ i ] = new TCanvas( Form( "can%d", i ), "hitPos", 900, 900 );
        can[ i ]->Divide( 3, 3 );   
        
        for( int j=0; j<9; j++ ) { 
            can[ i ]->cd( j + 1 );

            if( hitPos[ i * 9 + j ] != nullptr ) {
                hitPos[ i * 9 + j ]->Draw( "colz" );
            }

            gPad->Update();
        }

        TString file_name = Form( "hitPosition_run%d.pdf", run );
        TString title     = Form( "Title: hit positions in sector %d", 13 + i );

        if( i==0 ) file_name += "(";
        if( i==11 ) file_name += ")";
    
        can[ i ]->Print( file_name, title );
    }

}