void write_etofDetResolution() {
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

    gSystem->Setenv( "DB_ACCESS_MODE", "write" );

    // Initialize db manager
    StDbManager* mgr = StDbManager::Instance();
    StDbConfigNode* node = mgr->initConfig( "Calibrations_etof" );
    StDbTable* dbtable = node->addDbTable( "etofDetResolution" );
    TString storeTime = "2018-12-10 00:00:00"; // beginTime timestamp in MySQL format: "YYYY-MM-DD HH:mm:ss"
    mgr->setStoreTime( storeTime.Data() );

    // Create your c-struct
    etofDetResolution_st table;
    
    // Fill structure with data 
    std::cout << " read calibration parameters from local file" << std::endl;
    
    std::ifstream inData;

    string fileName = "detResolution2019.txt";
    
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

    float detectorResX[ 108 ];
    float detectorResY[ 108 ];
    float detectorResT[ 108 ];

    for( int i=0; i<108; i++ ) {
        inData >> detectorResX[ i ];
    }
    for( int i=0; i<108; i++ ) {
        inData >> detectorResY[ i ];
    }
    for( int i=0; i<108; i++ ) {
        inData >> detectorResT[ i ];
    }
    inData.close();

    for( int i=0; i<108; i++ ) {
        /* detector resolution in local X direction per counter */
        table.detectorResX[ i ] = detectorResX[ i ];

        /* detector resolution in local Y direction per counter */
        table.detectorResY[ i ] = detectorResY[ i ];

        /* detector resolution in time direction per counter */
        table.detectorResT[ i ] = detectorResT[ i ];
    }


    for( int i=0; i<108; i++ ) {
        unsigned int sector   = (   i       / 9 ) + 13;
        unsigned int module   = ( ( i % 9 ) / 3 ) +  1;
        unsigned int counter  = ( ( i % 3 )     ) +  1;

        std::cout << "(" << i << ") --> sector, module, counter: " <<  sector << "  " << module << "  " << counter;
        std::cout << "    --->  detector resolution x,y,t: " << table.detectorResX[ i ] << "  " << table.detectorResY[ i ] << "  " << table.detectorResT[ i ] << std::endl;
    }


    // Store data to the StDbTable
    dbtable->SetTable( ( char* ) &table, 1 );

    // uncomment next line to set "sim" flavor. "ofl" flavor is set by default, no need to set it.
    // dbtable->setFlavor( "sim" );

    // Store table to database
//    mgr->storeDbTable( dbtable );
}