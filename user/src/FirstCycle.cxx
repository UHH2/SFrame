// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: User
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

// ROOT include(s):
#include "TH1F.h"
#include "TGraph.h"

// Local include(s):
#include "../include/FirstCycle.h"

ClassImp( FirstCycle );

FirstCycle::FirstCycle()
   : m_El_p_T( 0 ), m_El_eta( 0 ), m_El_phi( 0 ), m_El_E( 0 ),
     m_allEvents( "allEvents", this ), m_passedEvents( "passedEvents", this ),
     m_test( "test", this ) {

   // To have the correct name in the log:
   SetLogName( this->GetName() );

   //
   // Declare the properties of the cycle:
   //
   DeclareProperty( "TestString", m_stringVariable );
   DeclareProperty( "TestInt", m_intVariable );
   DeclareProperty( "TestDouble", m_doubleVariable );
   DeclareProperty( "TestBool", m_boolVariable );
   DeclareProperty( "TestIntVector", m_intVecVariable );
   DeclareProperty( "TestDoubleVector", m_doubleVecVariable );
   DeclareProperty( "TestStringVector", m_stringVecVariable );
   DeclareProperty( "TestBoolVector", m_boolVecVariable );

   DeclareProperty( "RecoTreeString", m_recoTreeName );
   DeclareProperty( "MetaTreeName", m_metaTreeName );
}

void FirstCycle::BeginCycle() throw( SError ) {

   //
   // Print the properties specified in the XML configuration:
   //
   m_logger << INFO << "Value of string property: " << m_stringVariable
            << SLogger::endmsg;
   m_logger << INFO << "Value of int property   : " << m_intVariable
            << SLogger::endmsg;
   m_logger << INFO << "Value of double property: " << m_doubleVariable
            << SLogger::endmsg;
   m_logger << INFO << "Value of bool property  : " << ( m_boolVariable ? "true" : "false" )
            << SLogger::endmsg;
   m_logger << INFO << "Values of int vector property   : ";
   for( std::vector< int >::const_iterator it = m_intVecVariable.begin();
        it != m_intVecVariable.end(); ++it ) {
      m_logger << *it << "; ";
   }
   m_logger << SLogger::endmsg;
   m_logger << INFO << "Values of double vector property: ";
   for( std::vector< double >::const_iterator it = m_doubleVecVariable.begin();
        it != m_doubleVecVariable.end(); ++it ) {
      m_logger << *it << "; ";
   }
   m_logger << SLogger::endmsg;
   m_logger << INFO << "Values of string vector property: ";
   for( std::vector< std::string >::const_iterator it = m_stringVecVariable.begin();
        it != m_stringVecVariable.end(); ++it ) {
      m_logger << *it << "; ";
   }
   m_logger << SLogger::endmsg;
   m_logger << INFO << "Values of bool vector property: ";
   for( std::vector< bool >::const_iterator it = m_boolVecVariable.begin();
        it != m_boolVecVariable.end(); ++it ) {
      m_logger << ( *it ? "true" : "false" ) << "; ";
   }
   m_logger << SLogger::endmsg;

   //
   // Test how various printed lines look like:
   //
   m_logger << VERBOSE << "This is a VERBOSE line" << SLogger::endmsg;
   m_logger << DEBUG << "This is a DEBUG line" << SLogger::endmsg;
   m_logger << INFO << "This is an INFO line" << SLogger::endmsg;
   m_logger << WARNING << "This is a WARNING line" << SLogger::endmsg;
   m_logger << ERROR << "This is an ERROR line" << SLogger::endmsg;
   m_logger << FATAL << "This is a FATAL line" << SLogger::endmsg;
   m_logger << ALWAYS << "This is an ALWAYS line" << SLogger::endmsg;

   return;
}

void FirstCycle::EndCycle() throw( SError ) {

   return;
}

void FirstCycle::BeginInputFile( const SInputData& )  throw( SError ) {

   //
   // Connect the input variables:
   //
   ConnectVariable( m_recoTreeName.c_str(), "El_N", m_El_N );
   ConnectVariable( m_recoTreeName.c_str(), "El_p_T", m_El_p_T );
   ConnectVariable( m_recoTreeName.c_str(), "El_eta", m_El_eta );
   ConnectVariable( m_recoTreeName.c_str(), "El_phi", m_El_phi );
   ConnectVariable( m_recoTreeName.c_str(), "El_E", m_El_E );

   return;
}

void FirstCycle::BeginInputData( const SInputData& ) throw( SError ) {

   //
   // Declare the output variables:
   //
   DeclareVariable( m_o_example_variable, "example_variable" );
   DeclareVariable( m_o_El_p_T,           "El_p_T" );
   DeclareVariable( m_o_El,               "El" );

   //
   // Declare the metadata variables:
   //
   DeclareVariable( m_meta_El_p_T, "p_T", m_metaTreeName.c_str() );
   DeclareVariable( m_meta_El_eta, "eta", m_metaTreeName.c_str() );
   DeclareVariable( m_meta_El_phi, "phi", m_metaTreeName.c_str() );
   DeclareVariable( m_meta_El_E,   "E",   m_metaTreeName.c_str() );

   // Access the metadata tree to be able to control how entries
   // are written to it:
   m_electronTree = GetMetadataTree( m_metaTreeName.c_str() );

   //
   // Declare the output histograms:
   //
   Book( TH1F( "El_p_T_hist_file", "Electron p_{T}, merged 'in file'",
               100, 0.0, 150000.0 ), 0, kTRUE );
   Book( TH1F( "El_p_T_hist", "Electron p_{T}, merged 'in memory'", 100, 0.0,
               150000.0 ) );

   // Reserve two entries in the vector:
   m_test->resize( 2, 0 );

   return;
}

void FirstCycle::EndInputData( const SInputData& ) throw( SError ) {

   static const Int_t n = 5;
   Float_t x_array[ n ] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
   Float_t y_array[ n ] = { 0.0, 2.0, 4.0, 6.0, 8.0 };
   TGraph mygraph( n, x_array, y_array );
   mygraph.SetName( "MyGraph" );
   WriteObj( mygraph, "graph_dir" );

   return;
}

void FirstCycle::BeginMasterInputData( const SInputData& ) throw( SError ) {

   return;
}

void FirstCycle::EndMasterInputData( const SInputData& ) throw( SError ) {

   m_logger << INFO << "Number of all processed events: "
            << *m_allEvents << " " << ( m_test->size() > 0 ? ( *m_test )[ 0 ] : 0 )
            << SLogger::endmsg;
   m_logger << INFO << "Number of events passing selection: "
            << *m_passedEvents << " " << ( m_test->size() > 1 ? ( *m_test )[ 1 ] : 0 )
            << SLogger::endmsg;

   return;
}

void FirstCycle::ExecuteEvent( const SInputData&, Double_t weight ) throw( SError ) {

   //
   // If you have vectors (or any other type of containers) in the output,
   // you have to clear them by hand for each event. Otherwise they would
   // just grow and grow indefinitely.
   //
   m_o_El_p_T.clear();
   m_o_El.clear();

   // Fill the most simple output variable:
   m_o_example_variable = 1;

   // Loop over all electrons in the input:
   for( Int_t i = 0; i < m_El_N; ++i ) {

      // Fill a simple vector:
      m_o_El_p_T.push_back( ( *m_El_p_T )[ i ] );

      // Fill the example histogram(s):
      Hist( "El_p_T_hist" )->Fill( ( *m_El_p_T )[ i ], weight );
      Hist( "El_p_T_hist_file" )->Fill( ( *m_El_p_T )[ i ], weight );

      // Fill a vector of objects:
      m_o_El.push_back( SParticle( ( * m_El_p_T )[ i ],
                                   ( * m_El_eta )[ i ],
                                   ( * m_El_phi )[ i ],
                                   ( * m_El_E )[ i ] ) );

      // Fill the metadata tree. The user has to call TTree::Fill() by hand.
      m_meta_El_p_T = ( * m_El_p_T )[ i ];
      m_meta_El_eta = ( * m_El_eta )[ i ];
      m_meta_El_phi = ( * m_El_phi )[ i ];
      m_meta_El_E   = ( * m_El_E )[ i ];
      int nbytes = m_electronTree->Fill();
      if( nbytes < 0 ) {
         m_logger << ERROR << "Write error occured in metadata tree \""
                  << m_electronTree->GetName() << "\"" << SLogger::endmsg;
      } else if( nbytes == 0 ) {
         m_logger << WARNING << "No data written to tree \""
                  << m_electronTree->GetName() << "\"" << SLogger::endmsg;
      }
   }

   // Count the total number of processed events:
   ++m_allEvents;
   ( *m_test )[ 0 ]++;

   // Perform event selection. If you don't want to write out
   // an event, you have to throw an exception anywhere in the ExecuteEvent
   // method (or in a method called by ExecuteEvent) like this:
   if( ! m_El_N ) throw SError( SError::SkipEvent );

   // Count the number of events that passed the selection:
   ++m_passedEvents;
   ( *m_test )[ 1 ]++;

   // Fill validation histograms.
   // For adding more ValHistsTypes, edit the header File for this Cycle.
   // Change the line " enum ValHistsType { GENERAL, ELECTRON, MUON, JETS };"
   // by adding more Types
   TString StatusString = "Before_Cuts_";
   FillValidationHists( GENERAL, StatusString );
   FillValidationHists( ELECTRON, StatusString );
   FillValidationHists( MUON, StatusString );
   FillValidationHists( JETS, StatusString );

   return;
}

void FirstCycle::FillValidationHists( ValHistsType ht, const TString& status ) {  

   if( ht == GENERAL ){

      // book and fill general histograms here
      TString suffix = status + "General_";
      // The formalism for adding a new histogram to the output is the
      // following:
      //      Book( TH1F( suffix + "example_variable", "example_variable",
      //                  10, 0.0, 10.0 ) )->Fill( m_o_example_variable );

   } else if( ht == ELECTRON ){

      // book and fill Electron histograms here
      TString suffix = status + "Electron_";

   } else if( ht == MUON ){

      // book and fill muon histograms here
      TString suffix = status + "Muon_";

   } else if( ht == JETS ){

      // book and fill jet histograms here
      TString suffix = status + "Jets_";

   } else {

      SError error( SError::StopExecution );
      error << GetName() << ": Validation histogram type"  << ht << " not supported";
      throw error;
   }

   return;
}
