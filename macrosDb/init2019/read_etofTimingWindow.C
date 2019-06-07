void read_etofTimingWindow() {
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
    dbMk->SetDateTime( 20181210, 0 ); // event or run start time, set to your liking
    dbMk->SetFlavor( "ofl" );

    dbMk->Init();
    dbMk->Make();

    TDataSet *DB = 0;
    DB = dbMk->GetDataBase( "Calibrations/etof/etofTimingWindow" );
    if( !DB ) {
        std::cout << "ERROR: no table found in db, or malformed local db config" << std::endl;
        return;
    }

    St_etofTimingWindow *dataset = 0;
    dataset = ( St_etofTimingWindow* ) DB->Find( "etofTimingWindow" );

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

        etofTimingWindow_st *table = dataset->GetTable();
        for( int i = 0; i < rows; i++ ) {

            for( int j=0; j<12; j++ ) {
                std::cout << "AFCK address (" << j << "): 0x" << std::hex << table[ i ].afckAddress[ j ] << std::dec << std::endl;
                std::cout << "    ---> timingWindow: [ " << table[ i ].timingMin[ j ] << " , " << table[ i ].timingMax[ j ] << "] with peak at: " << table[ i ].timingPeak[ j ] << std::endl;
                std::cout << "    ---> pulserWindow: [ " << table[ i ].pulserMin[ j ] << " , " << table[ i ].pulserMax[ j ] << "] with peak at: " << table[ i ].pulserPeak[ j ] << std::endl;
                std::cout << std::endl;
            }
        }
    }
    else {
        std::cout << "ERROR: dataset does not contain requested table" << std::endl;
    }

}