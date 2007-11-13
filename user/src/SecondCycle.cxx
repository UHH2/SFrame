// $Id: SecondCycle.cxx,v 1.1.1.1 2007-11-13 12:41:09 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: User
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author Andreas Hoecker  <Andreas.Hocker@cern.ch>       - CERN
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

// ROOT include(s):
#include "TH1F.h"

// Local include(s):
#include "../include/SecondCycle.h"

ClassImp( SecondCycle );

SecondCycle::SecondCycle() {

   DeclareProperty( "FirstCycleTreeString", m_FirstCycleTreeName );

}

SecondCycle::~SecondCycle() {

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

void SecondCycle::BeginInputFile( const SInputData& inputData )  throw( SError ) {

   //
   // Connect the input variables:
   //
   ConnectVariable( m_FirstCycleTreeName.c_str(), "example_variable", m_example_variable );
   ConnectVariable( m_FirstCycleTreeName.c_str(), "El_p_T", m_El_p_T );
   ConnectVariable( m_FirstCycleTreeName.c_str(), "El", m_El );

   return;
}

void SecondCycle::ExecuteEvent( const SInputData&, Double_t weight ) throw( SError ) {

   // Loop over the simple vector:
   for( std::vector< double >::const_iterator it = m_El_p_T->begin();
        it != m_El_p_T->end(); ++it ) {
      Book( TH1F( "electron_pt", "Electron p_{T}", 50, 0.0, 100000.0 ) )->Fill( *it, weight );
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
