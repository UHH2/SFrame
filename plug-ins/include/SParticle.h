// Dear emacs, this is -*- c++ -*-
// $Id: SParticle.h,v 1.1.1.1 2007-11-13 12:41:09 krasznaa Exp $
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

#ifndef SFRAME_PLUGINS_SParticle_H
#define SFRAME_PLUGINS_SParticle_H

// STL include(s):
#include <vector> // This include is only here to be able to generate a dictionary
                  // for std::vector< SParticle >.

// ROOT include(s):
#include "Rtypes.h"
#include "Math/LorentzVector.h"
#include "Math/PtEtaPhiE4D.h"
#include "TObject.h"

/**
 * 4-momentum object that can be used in analyses.
 */
class SParticle :
   public ROOT::Math::LorentzVector< ROOT::Math::PtEtaPhiE4D< Double32_t > >,
   public TObject {

public:
   //
   // Various constructors
   //
   SParticle();
   SParticle( Double32_t pt, Double32_t eta, Double32_t phi,
              Double32_t e );
   SParticle( const SParticle& parent );
   template< class Geom > SParticle( const ROOT::Math::LorentzVector< Geom >& parent );

   //
   // Destructor
   //
   virtual ~SParticle();

   //
   // Copy operator(s)
   //
   template< class Geom > SParticle& operator= ( const ROOT::Math::LorentzVector< Geom >& rh );

   ClassDef( SParticle, 1 );

}; // class SParticle

//
// Include template implementation:
//
#ifndef __CINT__
#include "SParticle.icc"
#endif // __CINT__

#endif // SFRAME_PLUGINS_SParticle_H
