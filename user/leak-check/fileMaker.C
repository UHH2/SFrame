
// System include(s):
#include <iostream>
#include <iomanip>
#include <sstream>

// STL include(s):
#include <vector>

// ROOT include(s):
#include <TFile.h>
#include <TTree.h>
#include <TRandom.h>

/**
 * Simple macro that creates N different small ROOT files to be used
 * in subsequent tests.
 */
void fileMaker() {

   for( Int_t filenum = 0; filenum < 500; ++filenum ) {

      std::ostringstream filename;
      filename << "SFrameTest_" << std::setw( 4 ) << std::setfill( '0' )
               << filenum << ".root";

      TFile* ofile = new TFile( filename.str().c_str(), "RECREATE" );
      ofile->cd();

      if( ( ! ofile ) || ofile->IsZombie() ) {
         std::cerr << "Can't open file: " << filename.str() << std::endl;
         return;
      } else {
         std::cout << "Opened: " << filename.str() << std::endl;
      }

      TTree* otree = new TTree( "CollectionTree", "Format: User, data type: SFrameTest" );
      otree->SetAutoSave( 10000000 );
      otree->SetDirectory( ofile );
      TTree::SetBranchStyle( 1 );

      Int_t oint;
      std::vector< double >* ovec = new std::vector< double >();

      otree->Branch( "TestInt", &oint, "TestInt/I" );
      otree->Bronch( "TestVec", "std::vector<double>", &ovec );

      for( int i = 0; i < 500; ++i ) {

         ovec->clear();

         oint = gRandom->Integer( 5 );
         for( Int_t j = 0; j < oint; ++j ) {
            ovec->push_back( gRandom->Gaus( 0.0, 10.0 ) );
         }

         otree->Fill();

      }

      ofile->Write();
      ofile->Close();
      delete ofile;

      std::cout << "Written 500 events to " << filename.str() << std::endl;

   }

   return;

}
