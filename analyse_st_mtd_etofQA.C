#include <fstream>
#include <string>
#include <iostream>
#include "TString.h"
#include "TFile.h"
#include "TH1F.h"

using namespace::std;

void analyse_st_mtd_etofQA( char* filename ) {


	std::ifstream file( filename );
    
	std::string str; 
	while( std::getline( file, str ) ) {

		const char* name = str.c_str();

		string str_copy = str;
		
		TString infile_name( name );

		//cout << infile_name << endl;


		size_t lastindex;
	        // get rid of 
        	lastindex = str.find_last_of( "_" );
        	str = str.substr( 0, lastindex );
        	lastindex = str.find_last_of( "_" );
       		str = str.substr( 0, lastindex );

    		lastindex = str.find_last_of( "_" );
        	str = str.substr( lastindex + 1, str.length() );

        	cout << str << "    highest bin in trigger window plot at: ";




		TFile infile( infile_name, "READ" );

		TH1F* h = 0;
		h = ( TH1F* ) infile.Get( "AllDigiRaw_TDiff_ETof_Trg" );
		if( h ) {
			double mean = h->GetMean();
			//cout << mean << endl;

			double max = h->GetBinCenter( h->GetMaximumBin() );
			//cout << max  << " ns   ( mean: " << mean <<  " ) " << endl;
		}

		h = 0;
		infile.Close();



		string in_hit = "Hit";

		str_copy.replace( str_copy.end()-7, str_copy.end()-5, in_hit.begin(), in_hit.end());

		//cout << str_copy << endl;

		const char* name2 = str_copy.c_str();

		TString infile_name2( name2 );



		TFile infile2( infile_name2, "READ" );

		bool h_exists = infile2.GetListOfKeys()->Contains( "unclusteredHit_tof_fullrange" );

		//cout << h_exists << endl; 

		TH1F* h2 = 0;
		if( h_exists ) h2 = ( TH1F* ) infile2.Get( "unclusteredHit_tof_fullrange" );
		if( h2 ) {
			double mean = h2->GetMean();
			//cout << mean << endl;

			double max = h2->GetBinCenter( h2->GetMaximumBin() );
			cout << "maximum in hit time of flight distribution at: " << max  << " ns   ( mean: " << mean <<  " " << "rms: " << h2->GetRMS() << " ) " << endl;
		}

		h2 = 0;
		infile2.Close();

	}	

}
