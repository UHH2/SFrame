// $Id: SParticle.cxx,v 1.1.1.1 2007-11-13 12:41:09 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Plug-ins
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author Andreas Hoecker  <Andreas.Hocker@cern.ch>       - CERN
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

// Local include(s):
#include "../include/SParticle.h"

ClassImp( SParticle );

SParticle::SParticle()
   : ROOT::Math::LorentzVector< ROOT::Math::PtEtaPhiE4D< Double32_t > >(),
     TObject() {

}

SParticle::SParticle( Double32_t pt, Double32_t eta, Double32_t phi,
                      Double32_t e )
   : ROOT::Math::LorentzVector< ROOT::Math::PtEtaPhiE4D< Double32_t > >( pt, eta,
                                                                         phi, e ),
     TObject() {

}

SParticle::SParticle( const SParticle& parent )
   : ROOT::Math::LorentzVector< ROOT::Math::PtEtaPhiE4D< Double32_t > >( parent ),
     TObject() {

}

SParticle::~SParticle() {

}
