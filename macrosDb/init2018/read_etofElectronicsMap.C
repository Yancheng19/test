void read_etofElectronicsMap() {
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
    DB = dbMk->GetDataBase( "Geometry/etof/etofElectronicsMap" );
    if( !DB ) {
        std::cout << "ERROR: no table found in db, or malformed local db config" << std::endl;
        return;
    }

    St_etofElectronicsMap* dataset = 0;
    dataset = ( St_etofElectronicsMap* ) DB->Find( "etofElectronicsMap" );

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

        etofElectronicsMap_st* table = dataset->GetTable();
        for( int i=0; i<rows; i++ ) {
            // Test if all values are correct
            std::cout << "nAfcks: "    << (int) table[ i ].nAfcks << endl;
            std::cout << "nChannels: " << table[ i ].nChannels    << endl;

            std::cout << std::endl << std::endl;

            for( int j=0; j<12; j++ ) {
                std::cout << "AFCK address (" << j << "): 0x" << std::hex << table[ i ].afckAddress[ j ] << std::dec << " --> sector: " << (int) table[ i ].sector[ j ] << std::endl; 
            }

            std::cout << std::endl << std::endl;

            for( int j=0; j<576; j++ ) {
                std::cout << "channelNumber (" << j << "): " << table[ i ].channelNumber[ j ] << " --> geometryId: " << table[ i ].geometryId[ j ] << std::endl; 
            }
        }
    }
    else {
        std::cout << "ERROR: dataset does not contain requested table" << std::endl;
    }
}