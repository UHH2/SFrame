// $Id: FirstCycle.cxx,v 1.3.2.1 2008-12-01 14:52:57 krasznaa Exp $
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
   : m_El_p_T( 0 ), m_El_eta( 0 ), m_El_phi( 0 ), m_El_E( 0 ) {

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
   DeclareProperty( "RecoTreeString", m_RecoTreeName );

}

FirstCycle::~FirstCycle() {

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

   return;
}

void FirstCycle::EndCycle() throw( SError ) {

   return;
}

void FirstCycle::BeginInputFile( const SInputData& inputData )  throw( SError ) {

   //
   // Connect the input variables:
   //
   ConnectVariable( m_RecoTreeName.c_str(), "El_N", m_El_N );
   ConnectVariable( m_RecoTreeName.c_str(), "El_p_T", m_El_p_T );
   ConnectVariable( m_RecoTreeName.c_str(), "El_eta", m_El_eta );
   ConnectVariable( m_RecoTreeName.c_str(), "El_phi", m_El_phi );
   ConnectVariable( m_RecoTreeName.c_str(), "El_E", m_El_E );

   return;
}

void FirstCycle::BeginInputData( const SInputData& ) throw( SError ) {

   //
   // Declare the output variables:
   //
   DeclareVariable( m_o_example_variable, "example_variable" );
   DeclareVariable( m_o_El_p_T, "El_p_T" );
   DeclareVariable( m_o_El, "El" );

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

void FirstCycle::ExecuteEvent( const SInputData&, Double_t /*weight*/ ) throw( SError ) {

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
      m_o_El_p_T.push_back( (*m_El_p_T) [i] );

      Book( TH1F( "El_p_T_hist", "Electron p_{T}", 100, 0.0,
                  150000.0 ) )->Fill( (*m_El_p_T) [i] );

      // Fill a vector of objects:
      m_o_El.push_back( SParticle( ( * m_El_p_T )[ i ],
                                   ( * m_El_eta )[ i ],
                                   ( * m_El_phi )[ i ],
                                   ( * m_El_E )[ i ] ) );

   }

   // Perform event selection. If you don't want to write out
   // an event, you have to throw an exception anywhere in the ExecuteEvent
   // method (or in a method called by ExecuteEvent) like this:
   if( ! m_El_N ) throw SError( SError::SkipEvent );

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
      Book( TH1F( suffix + "example_variable", "example_variable",
                  10, 0.0, 10.0 ) )->Fill( m_o_example_variable );

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
