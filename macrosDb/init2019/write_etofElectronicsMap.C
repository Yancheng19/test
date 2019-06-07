void write_etofElectronicsMap() {
    // if you want to use root.exe instead of root4star, uncomment block below:

//    gSystem->AddDynamicPath("/usr/lib/mysql");
//    gSystem->AddDynamicPath("/usr/lib64/mysql");
//    gSystem->AddDynamicPath("$OPTSTAR/lib/mysql/");
//    gSystem->Load("libmysqlclient");

    // Load all required libraries
    gROOT->Macro("LoadLogger.C");
    gSystem->Load("St_base.so");
    gSystem->Load("libStDb_Tables.so");
    gSystem->Load("StDbLib.so");

    gSystem->Setenv("DB_ACCESS_MODE","write");

    // Initialize db manager
    StDbManager*    mgr       = StDbManager::Instance();
    StDbConfigNode* node      = mgr->initConfig( "Geometry_etof" );
    StDbTable*      dbtable   = node->addDbTable( "etofElectronicsMap" );
    TString         storeTime = "2018-12-10 00:00:00"; // beginTime timestamp in MySQL format: "YYYY-MM-DD HH:mm:ss"
    mgr->setStoreTime( storeTime.Data() );

    // Create your c-struct
    etofElectronicsMap_st table;
    
    // Fill structure with data from local file
    std::cout << " read calibration parameters from local file" << std::endl;
    
    std::ifstream inData;

    string fileName = "electronicsMap2019.txt";
    
    //load parameters from local file
    if( fileName.length() == 0 ) {
        std::cout << "local file path parameters is missing" << std::endl;
        return;
    }
    
    std::cout << "local file for electronics to hardware mapping: " << fileName << std::endl;

    inData.open( fileName.c_str() );

    if( !inData.is_open() ) {
        std::cout << "unable to get the parameters from local file" << std::endl;
        return;
    }

    int nAfcks;
    int nChannels;
    int afckAddress[ 12 ];
    int sector[ 12 ];
    int channelNumber[ 576 ];
    int geometryId[ 576 ];

    inData >> nAfcks;
    inData >> nChannels;

    for( int i=0; i<12; i++ ) {
        inData >> std::hex >> afckAddress[ i ];
    }
    for( int i=0; i<12; i++ ) {
        inData >> std::dec >> sector[ i ];
    }
    for( int i=0; i<576; i++ ) {
        inData >> channelNumber[ i ];
    }
    for( int i=0; i<576; i++ ) {
        inData >> geometryId[ i ];
    }

    inData.close();



    /* number of AFCK boards in the system */
    table.nAfcks = 5;

    /* number of channels connected to the AFCKs */
    table.nChannels = 576;

    for( int i=0; i<12; i++ ) {
        /* MAC address of AFCK board */
        table.afckAddress[ i ] = afckAddress[ i ];

        /* eTOF sector linked to the AFCK */
        table.sector[ i ] = sector[ i ];
    }
    for( int i=0; i<576; i++ ) {
        /* channel number ( up to 9 RPCs with 64 channels each are connected to one AFCK ) = chipId * 10 + channelId
           in 2018: = afckId * 1000 + chipId * 10 + channelId */
        table.channelNumber[ i ] = channelNumber[ i ];

        /* eTOF plane, counter, strip and side corresponding to each channel
           geometry id = zPlane * 10000 + counter * 1000 + strip * 10 + side */
        table.geometryId[ i ] = geometryId[ i ];
    }



    // Test if all values are correct
    /* 
    std::cout << "nAfcks: "    << (int) table.nAfcks << endl;
    std::cout << "nChannels: " << table.nChannels    << endl;

    std::cout << std::endl << std::endl;

    for( int i=0; i<12; i++ ) {
        std::cout << "AFCK address (" << i << "): " << std::hex << table.afckAddress[ i ] << std::dec << " --> sector: " << (int) table.sector[ i ] << std::endl; 
    }

    std::cout << std::endl << std::endl;

    for( int i=0; i<576; i++ ) {
        std::cout << "channelNumber (" << i << "): " << table.channelNumber[ i ] << " --> geometryId: " << table.geometryId[ i ] << std::endl; 
    }
    */


    // Store data to the StDbTable
    dbtable->SetTable( (char*) &table, 1 );

    // uncomment next line to set "sim" flavor. "ofl" flavor is set by default, no need to set it.
    // dbtable->setFlavor("sim");

    // Store table to database
//    mgr->storeDbTable( dbtable );
}
