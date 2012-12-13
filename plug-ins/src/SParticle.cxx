// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Plug-ins
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
 *
 ***************************************************************************/

// Local include(s):
#include "../include/SParticle.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SParticle )
#endif // DOXYGEN_IGNORE

/**
 * This constructor creates a "null-vector". Default constructors are
 * needed to be able to read/write objects with ROOT.
 */
SParticle::SParticle()
   : ROOT::Math::LorentzVector< ROOT::Math::PtEtaPhiE4D< Double32_t > >(),
     TObject() {

}

/**
 * This is the cmost useful constructor I guess. It receives the usual
 * 4-momentum parameters of a reconstructed or truth particle.
 *
 * @param pt  p<sub>T</sub> of the particle
 * @param eta pseudo-rapidity of the particle
 * @param phi azimuthal angle of the particle
 * @param e   energy of the particle
 */
SParticle::SParticle( Double32_t pt, Double32_t eta, Double32_t phi,
                      Double32_t e )
   : ROOT::Math::LorentzVector< ROOT::Math::PtEtaPhiE4D< Double32_t > >( pt, eta,
                                                                         phi, e ),
     TObject() {

}

/**
 * This copy constructor clones another SParticle object.
 *
 * @param parent The SParticle that we're copying
 */
SParticle::SParticle( const SParticle& parent )
   : ROOT::Math::LorentzVector< ROOT::Math::PtEtaPhiE4D< Double32_t > >( parent ),
     TObject() {

}
