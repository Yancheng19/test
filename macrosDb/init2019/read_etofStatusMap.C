void read_etofStatusMap() {
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

    TDataSet* DB = 0;
    DB = dbMk->GetDataBase( "Calibrations/etof/etofStatusMap" );
    if( !DB ) {
        std::cout << "ERROR: no table found in db, or malformed local db config" << std::endl;
        return;
    }

    St_etofStatusMap* dataset = 0;
    dataset = ( St_etofStatusMap* ) DB->Find( "etofStatusMap" );

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

        etofStatusMap_st* table = dataset->GetTable();
        for( int i=0; i<rows; i++ ) {
            // sample output of first member variable
            //std::cout << i << "th row : " << (int) table[ i ].status[ 0 ] << std::endl;

            for( int j=0; j<6912; j++ ) {      
                unsigned int sector   = (   j         / 576 ) + 13;
                unsigned int module   = ( ( j % 576 ) / 192 ) +  1;
                unsigned int counter  = ( ( j % 192 ) /  64 ) +  1;
                unsigned int strip    = ( ( j %  64 ) /   2 ) +  1;
                unsigned int side     =   ( j %   2 )         +  1;

                std::cout << "channel " << j << ": " << sector << "  " << module << "  " << counter << "  " << strip << "  " << side << " --> status = " << (int) table[ i ].status[ j ] << std::endl;
            }
        }
    } else {
        std::cout << "ERROR: dataset does not contain requested table" << std::endl;
    }

}