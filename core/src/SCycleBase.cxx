// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
 *
 ***************************************************************************/

// Local include(s):
#include "../include/SCycleBase.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBase )
#endif // DOXYGEN_IGNORE

/**
 * The constructor only silently creates the base objects.
 */
SCycleBase::SCycleBase() {

   REPORT_VERBOSE( "SCycleBase constructed" );
}
