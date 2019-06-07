void read_etofDigiSlewCorr() {
    // if you want to use root.exe instead of root4star, uncomment block below:

//    gSystem->AddDynamicPath("/usr/lib/mysql");
//    gSystem->AddDynamicPath("/usr/lib64/mysql");
//    gSystem->AddDynamicPath("$OPTSTAR/lib/mysql/");
//    gSystem->Load("libmysqlclient");

    // base libraries
    gSystem->Load( "St_base" );
    gSystem->Load( "StChain" );
    gSystem->Load( "StUtilities" );
    gSystem->Load( "StIOMaker" );
    gSystem->Load( "StarClassLibrary" );

    // db-related libraries
    gSystem->Load( "St_Tables" );
    gSystem->Load( "StDbLib" );
    gSystem->Load( "StDbBroker" );
    gSystem->Load( "St_db_Maker" );

    St_db_Maker* dbMk = new St_db_Maker( "db", "MySQL:StarDb", "$STAR/StarDb" );
    dbMk->SetDebug();
    dbMk->SetDateTime( 20180101, 0 ); // event or run start time, set to your liking
    dbMk->SetFlavor( "ofl" );

    dbMk->Init();
    dbMk->Make();

    TDataSet* DB = 0;
    DB = dbMk->GetDataBase( "Calibrations/etof/etofDigiSlewCorr" );
    if( !DB ) {
        std::cout << "ERROR: no table found in db, or malformed local db config" << std::endl;
        return;
    }

    St_etofDigiSlewCorr* dataset = 0;
    dataset = ( St_etofDigiSlewCorr* ) DB->Find( "etofDigiSlewCorr" );

    if( dataset ) {
        int rows = dataset->GetNRows();
        if( rows > 1 ) {
            std::cout << "INFO: found INDEXED table with " << rows << " rows" << std::endl;
        }

        TDatime val[ 2 ];
        dbMk->GetValidity( ( TTable* ) dataset, val );
        std::cout << "Dataset validity range: [ " << val[ 0 ].GetDate() << "." << val[ 0 ].GetTime() << " - "
        << val[ 1 ].GetDate() << "." << val[ 1 ].GetTime() << " ] "
        << std::endl;

        etofDigiSlewCorr_st* table = dataset->GetTable();
        for( int i=0; i<rows; i++ ) {

            int nChannels = 6912;
            int nTotBins  = 30;

            // conversion factor from float to short used for storing the values in the database more efficiently
            int conversionFactor = 100;


            for( unsigned int j = 0; j < nChannels * nTotBins; j++ ) {

                int totBin = j % nTotBins;
                int chan   = j / nTotBins;

                unsigned int sector   = (   chan         / 576 ) + 13;
                unsigned int module   = ( ( chan % 576 ) / 192 ) +  1;
                unsigned int counter  = ( ( chan % 192 ) /  64 ) +  1;
                unsigned int strip    = ( ( chan %  64 ) /   2 ) +  1;
                unsigned int side     =   ( chan %   2 )         +  1;


                // Test if all values are correct
                std::cout << "table entry: " << j << "  channel " << table[ i ].channelId[ j ] << ": " << sector << "  " << module << "  " << counter << "  " << strip << "  " << side;
                std::cout << " --> slewing correction for upperTotEdge: " << table[ i ].upperTotEdge[ j ] << " --> "  << table[ i ].corr[ j ] << std::endl;
            }
        }
    }
    else {
        std::cout << "ERROR: dataset does not contain requested table" << std::endl;
    }
}