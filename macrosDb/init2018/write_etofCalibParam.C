void write_etofCalibParam() {
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
    StDbManager*    mgr = StDbManager::Instance();
    StDbConfigNode* node = mgr->initConfig( "Calibrations_etof" );
    StDbTable*      dbtable = node->addDbTable( "etofCalibParam" );
    TString         storeTime = "2018-01-01 00:00:00"; // beginTime timestamp in MySQL format: "YYYY-MM-DD HH:mm:ss"
    mgr->setStoreTime( storeTime.Data() );

    // Create your c-struct
    etofCalibParam_st table;
    
    // Fill structure with data 
    table.get4TotBinWidthNs = 0.4;
    table.minDigisInSlewBin = 25;

    // Test if all values are correct
    std::cout << "get4TotBinWidthNs: " << table.get4TotBinWidthNs << "   minDigisInSlewBin: " << (int) table.minDigisInSlewBin << std::endl;


    // Store data to the StDbTable
    dbtable->SetTable( ( char* ) &table, 1 );

    // uncomment next line to set "sim" flavor. "ofl" flavor is set by default, no need to set it.
    // dbtable->setFlavor("sim");

    // Store table to database
//    mgr->storeDbTable( dbtable );
}
