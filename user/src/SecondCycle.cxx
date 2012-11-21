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
#include "../include/SecondCycle.h"

ClassImp( SecondCycle );

SecondCycle::SecondCycle()
   : SCycleBase() {

   SetLogName( GetName() );

   DeclareProperty( "FirstCycleTreeString",
                    m_FirstCycleTreeName = "FirstCycleTree" );
}

void SecondCycle::BeginCycle() throw( SError ) {

   return;
}

void SecondCycle::EndCycle() throw( SError ) {

   return;
}

void SecondCycle::BeginInputData( const SInputData& ) throw( SError ) {

   return;
}

void SecondCycle::EndInputData( const SInputData& ) throw( SError ) {

   return;
}

void SecondCycle::BeginInputFile( const SInputData& ) throw( SError ) {

   //
   // Connect the input variables:
   //
   ConnectVariable( m_FirstCycleTreeName.c_str(), "example_variable",
                    m_example_variable );
   ConnectVariable( m_FirstCycleTreeName.c_str(), "El_p_T", m_El_p_T );
   ConnectVariable( m_FirstCycleTreeName.c_str(), "El", m_El );

   //
   // Retrieve some objects from the input file:
   //
   TH1* hist = Retrieve< TH1 >( "El_p_T_hist" );
   if( ! hist ) {
      throw SError( "Coulnd't find histogram with name El_p_T_hist in the "
                    "input", SError::SkipCycle );
   }
   hist->Print(); // Show that we succeeded
   TGraph* graph = Retrieve< TGraph >( "MyGraph", "graph_dir" );
   if( ! graph ) {
      throw SError( "Couldn't find graph with name \"graph_dir/MyGraph\" in the"
                    " input", SError::SkipCycle );
   }
   graph->Print(); // Show that we succeeded

   return;
}

void SecondCycle::ExecuteEvent( const SInputData&,
                                Double_t weight ) throw( SError ) {

   // Loop over the simple vector:
   for( std::vector< double >::const_iterator it = m_El_p_T->begin();
        it != m_El_p_T->end(); ++it ) {
      // Ask the histogram to be merged in file. This will produce a warning,
      // as there's no output file used in the merging. This is meant as
      // a double-check that things are working correctly in this case as
      // well.
      Book( TH1F( "electron_pt", "Electron p_{T}", 50, 0.0, 100000.0 ),
            0, kTRUE )->Fill( *it, weight );
   }

   // Loop over the electron objects:
   for( std::vector< SParticle >::const_iterator el = m_El->begin();
        el != m_El->end(); ++el ) {
      Book( TH1F( "El_p_T", "Electron p_{T}", 100, 0.0, 150000.0 ),
            "obj_test" )->Fill( el->Pt(), weight );
      Book( TH1F( "El_eta", "Electron #eta", 100, -3.5, 3.5 ),
            "obj_test" )->Fill( el->Eta(), weight );
      Book( TH1F( "El_phi", "Electron #phi", 100, -3.141592, 3.141592 ),
            "obj_test" )->Fill( el->Phi(), weight );
   }

   return;
}
