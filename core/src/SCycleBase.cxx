// $Id: SCycleBase.cxx,v 1.3 2008-01-25 14:33:53 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

// Local include(s):
#include "../include/SCycleBase.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBase );
#endif // DOXYGEN_IGNORE

/**
 * The constructor only silently creates the base objects.
 */
SCycleBase::SCycleBase() {

   m_logger << VERBOSE << "SCycleBase constructed" << SLogger::endmsg;

}


/**
 * Another one of the "I don't do anything" destructors.
 */
SCycleBase::~SCycleBase() {

   m_logger << VERBOSE << "SCycleBase destructed" << SLogger::endmsg;

}
