void write_etofDigiTimeCorr() {
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
    StDbManager*    mgr       = StDbManager::Instance();
    StDbConfigNode* node      = mgr->initConfig( "Calibrations_etof" );
    StDbTable*      dbtable   = node->addDbTable( "etofDigiTimeCorr" );
    TString         storeTime = "2018-01-01 00:00:00"; // beginTime timestamp in MySQL format: "YYYY-MM-DD HH:mm:ss"
    mgr->setStoreTime( storeTime.Data() );

    // Create your c-struct
    etofDigiTimeCorr_st table;
    
    // Fill structure with data 
    for( unsigned int i = 0; i < 6912; i++ ) {
        unsigned int sector   = (   i         / 576 ) + 13;
        unsigned int module   = ( ( i % 576 ) / 192 ) +  1;
        unsigned int counter  = ( ( i % 192 ) /  64 ) +  1;
        unsigned int strip    = ( ( i %  64 ) /   2 ) +  1;
        unsigned int side     =   ( i %   2 )         +  1;

        // set additive time correction of all channels to 0 --> no correction
        table.timeCorr[ i ] = 0.;

        // Test if all values are correct
        //std::cout << "channel " << i << ": " << sector << "  " << module << "  " << counter << "  " << strip << "  " << side << " --> time correction = " << table.timeCorr[ i ] << std::endl;
    }


    // Store data to the StDbTable
    dbtable->SetTable( ( char* ) &table, 1 );

    // uncomment next line to set "sim" flavor. "ofl" flavor is set by default, no need to set it.
    // dbtable->setFlavor("sim");

    // Store table to database
//    mgr->storeDbTable( dbtable );
}