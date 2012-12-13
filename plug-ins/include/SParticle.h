// Dear emacs, this is -*- c++ -*-
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

#ifndef SFRAME_PLUGINS_SParticle_H
#define SFRAME_PLUGINS_SParticle_H

// STL include(s):
#include <vector> // This include is only here to be able to generate a
                  // dictionary for std::vector< SParticle >.

// ROOT include(s):
#include "Rtypes.h"
#include "Math/LorentzVector.h"
#include "Math/PtEtaPhiE4D.h"
#include "TObject.h"

/**
 *   @short 4-momentum object that can be used in analyses.
 *
 *          This class is basically just here to demonstrate what kind of
 *          "advanced" ROOT objects can be used/saved/read with SFrame.
 *          The class allows all the algebra operations between SParticle
 *          objects that are defined for the LorentzVector class, so
 *          for instance things like
 *
 *          <code>
 *            SParticle p1( ... ), p2( ... );<br/>
 *            SParticle p3 = p1 + p2;
 *          </code>
 *
 *          The package generates dictionaries for the SParticle and
 *          std::vector<SParticle> classes, so they can be written out to
 *          and read back from TTree-s.
 *
 *          For detailed a description of the functions that this class
 *          supports, see:
 *          http://root.cern.ch/root/html518/ROOT__Math__LorentzVector_ROOT__Math__PtEtaPhiE4D_Double32_t___.html
 *
 * @version $Revision$
 */
class SParticle :
   public ROOT::Math::LorentzVector< ROOT::Math::PtEtaPhiE4D< Double32_t > >,
   public TObject {

public:
   /// Default constructor
   SParticle();
   /// Constructor with 4 discrete momentum parameters
   SParticle( Double32_t pt, Double32_t eta, Double32_t phi,
              Double32_t e );
   /// Copy constructor
   SParticle( const SParticle& parent );
   /// Generic copy constructor
   template< class Geom >
   SParticle( const ROOT::Math::LorentzVector< Geom >& parent );

   /// Generic copy operator
   template< class Geom >
   SParticle& operator= ( const ROOT::Math::LorentzVector< Geom >& rh );

#ifndef DOXYGEN_IGNORE
   ClassDef( SParticle, 1 )
#endif // DOXYGEN_IGNORE

}; // class SParticle

//
// Include template implementation:
//
#ifndef __CINT__
#include "SParticle.icc"
#endif // __CINT__

/** @class ROOT::Math::LorentzVector
 *  @short ROOT class for describing a 4-momentum
 *
 *         This is a very useful ROOT class that can be used in HEP
 *         kinematical calculations. For more documentation, go to
 *         http://root.cern.ch/root/html518/ROOT__Math__LorentzVector_ROOT__Math__PtEtaPhiE4D_Double32_t___.html
 */

/** @namespace ROOT::Math
 *  @short Namespace for the ROOT math classes
 *
 *         This ROOT namespace holds all algebra related classes. For
 *         more information, see:
 *         http://root.cern.ch/root/html518/ROOT__Math.html
 */

/** @namespace ROOT
 *  @short General namespace for ROOT objects
 *
 *         The namespace holds quite a few classes. For more information,
 *         see: http://root.cern.ch/root/html518/ROOT.html
 */

#endif // SFRAME_PLUGINS_SParticle_H
