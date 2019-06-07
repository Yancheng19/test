void write_etofTimingWindow() {
    // if you want to use root.exe instead of root4star, uncomment block below:

//    gSystem->AddDynamicPath("/usr/lib/mysql");
//    gSystem->AddDynamicPath("/usr/lib64/mysql");
//    gSystem->AddDynamicPath("$OPTSTAR/lib/mysql/");
//    gSystem->Load("libmysqlclient");

    // Load all required libraries
    gROOT->Macro( "LoadLogger.C" );
    gSystem->Load( "St_base.so" );
    gSystem->Load( "libStDb_Tables.so" );
    gSystem->Load( "StDbLib.so" );

    gSystem->Setenv("DB_ACCESS_MODE","write");

    // Initialize db manager
    StDbManager*    mgr       = StDbManager::Instance();
    StDbConfigNode* node      = mgr->initConfig( "Calibrations_etof" );
    StDbTable*      dbtable   = node->addDbTable( "etofTimingWindow" );
    TString         storeTime = "2018-01-01 00:00:00"; // beginTime timestamp in MySQL format: "YYYY-MM-DD HH:mm:ss"
    mgr->setStoreTime( storeTime.Data() );

    // Create your c-struct
    etofTimingWindow_st table;
    
    // Fill structure with data 
	std::cout << " read calibration parameters from local file" << std::endl;
    
    std::ifstream inData;

    string fileName = "timingWindow2018.txt";
    
    //load parameters from local file
    if( fileName.length() == 0 ) {
        std::cout << "local file path parameters is missing" << std::endl;
        return;
    }
    
    std::cout << "local file for timing window parameters: " << fileName << std::endl;

    inData.open( fileName.c_str() );

    if( !inData.is_open() ) {
        std::cout << "unable to get the parameters from local file" << std::endl;
        return;
    }

    int afckAddress[ 12 ];

    int timingMin[ 12 ];
    int timingMax[ 12 ];
    int timingPeak[ 12 ];


    int pulserMin[ 12 ];
    int pulserMax[ 12 ];
    int pulserPeak[ 12 ];


    for( int i=0; i<12; i++ ) {
        inData >> std::hex >> afckAddress[ i ] >> std::dec;
        inData >> timingMin[ i ] >> timingMax[ i ] >> timingPeak[ i ];
        inData >> pulserMin[ i ] >> pulserMax[ i ] >> pulserPeak[ i ];
    }
    inData.close();



    for( int i=0; i<12; i++ ) {
        /* MAC address of AFCK board */
        table.afckAddress[ i ] = afckAddress[ i ];

        /* lower edge of timing window */
        table.timingMin[ i ] = timingMin[ i ];
        /* upper egde of timing window */
        table.timingMax[ i ] = timingMax[ i ];
        /* peak value inside timing window */
        table.timingPeak[ i ] = timingPeak[ i ];

        /* lower edge of pulser window */
        table.pulserMin[ i ] = pulserMin[ i ];
        /* upper egde of pulser window */
        table.pulserMax[ i ] = pulserMax[ i ];
        /* peak value inside pulser window */
        table.pulserPeak[ i ] = pulserPeak[ i ];

    }


    for( int i=0; i<12; i++ ) {
        std::cout << "AFCK address (" << i << "): 0x" << std::hex << table.afckAddress[ i ] << std::dec << std::endl;
        std::cout << "    ---> timingWindow: [ " << table.timingMin[ i ] << " , " << table.timingMax[ i ] << "] with peak at: " << table.timingPeak[ i ] << std::endl;
        std::cout << "    ---> pulserWindow: [ " << table.pulserMin[ i ] << " , " << table.pulserMax[ i ] << "] with peak at: " << table.pulserPeak[ i ] << std::endl;
        std::cout << std::endl;
    }


    // Store data to the StDbTable
    dbtable->SetTable( ( char* ) &table, 1 );

    // uncomment next line to set "sim" flavor. "ofl" flavor is set by default, no need to set it.
    // dbtable->setFlavor("sim");

    // Store table to database
//    mgr->storeDbTable( dbtable );
}