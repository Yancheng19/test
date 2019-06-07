#define ROUND_2_INT( f ) ( (int) ( f >= 0.0 ? ( f + 0.5 ) : ( f - 0.5 ) ) )

void write_etofDigiSlewCorr() {
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
    StDbTable*      dbtable   = node->addDbTable( "etofDigiSlewCorr" );
    TString         storeTime = "2018-12-10 00:00:00"; // beginTime timestamp in MySQL format: "YYYY-MM-DD HH:mm:ss"
    mgr->setStoreTime( storeTime.Data() );

    // Create your c-struct
    etofDigiSlewCorr_st table;
    
    // Fill structure with data
    int nChannels = 6912;
    int nTotBins  = 30;

    float totEdges[ 30 ] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0,
                             1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0,
                             2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5, 7.0
                           };

    // conversion factor from float to short used for storing the values in the database more efficiently
    int conversionFactor = 100;


    for( unsigned int i = 0; i < nChannels * nTotBins; i++ ) {

        int totBin = i % nTotBins;
        int chan   = i / nTotBins;

        unsigned int sector   = (   chan         / 576 ) + 13;
        unsigned int module   = ( ( chan % 576 ) / 192 ) +  1;
        unsigned int counter  = ( ( chan % 192 ) /  64 ) +  1;
        unsigned int strip    = ( ( chan %  64 ) /   2 ) +  1;
        unsigned int side     =   ( chan %   2 )         +  1;

        table.channelId[ i ]    = chan;
        table.upperTotEdge[ i ] = ROUND_2_INT( totEdges[ totBin ] * conversionFactor );


        // set additive time correction of all channels to 0 --> no correction        
        float correctionValue = 0.;

        table.corr[ i ] = ROUND_2_INT( correctionValue * conversionFactor );


        // Test if all values are correct
        //std::cout << "table entry: " << i << "  channel " << table.channelId[ i ] << ": " << sector << "  " << module << "  " << counter << "  " << strip << "  " << side;
        //std::cout << " --> slewing correction for upperTotEdge: " << table.upperTotEdge[ i ] << " --> "  << table.corr[ i ] << std::endl;
    }


    // Store data to the StDbTable
    dbtable->SetTable( ( char* ) &table, 1 );

    // uncomment next line to set "sim" flavor. "ofl" flavor is set by default, no need to set it.
    // dbtable->setFlavor("sim");

    // Store table to database
//    mgr->storeDbTable( dbtable );
}