// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Cintex
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
 *
 ***************************************************************************/

// ROOT include(s):
#include <Rtypes.h>
#include <Cintex/Cintex.h>

// SFrame include(s):
#include "core/include/SLogger.h"

/// Enable the Cintex library
/**
 * Function used for enabling the Cintex library. Since the Cintex.h header is
 * available in all ROOT releases since at least 5.20, relying on this header
 * shouldn't be a problem.
 *
 * @returns A dummy value, just for technical reasons
 */
Int_t EnableCintex() {

   SLogger logger( "EnableCintex" );

   // Enable the Cintex library:
   ROOT::Cintex::Cintex::Enable();

   // Let the user know what we just did:
   logger << INFO << "Usage of the Cintex library is now enabled"
          << SLogger::endmsg;
   return 0;
}

/// The usual trick for executing some code when the library is loaded
static Int_t dummy = EnableCintex();
