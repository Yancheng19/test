void write_etofResetTimeCorr() {
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
    StDbTable*      dbtable   = node->addDbTable( "etofResetTimeCorr" );
    TString         storeTime = "2018-01-01 00:00:00"; // beginTime timestamp in MySQL format: "YYYY-MM-DD HH:mm:ss"
    mgr->setStoreTime( storeTime.Data() );

    // Create your c-struct
    etofResetTimeCorr_st table;
    
    // Fill structure with data 
    table.resetTimeOffset = 0;

    std::cout << "resetTime correction: " << table.resetTimeOffset << std::endl;

    // Store data to the StDbTable
    dbtable->SetTable( ( char* ) &table, 1 );

    // uncomment next line to set "sim" flavor. "ofl" flavor is set by default, no need to set it.
    // dbtable->setFlavor("sim");

    // Store table to database
//    mgr->storeDbTable( dbtable );
}