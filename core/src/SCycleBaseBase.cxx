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
#include "../include/SCycleBaseBase.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseBase )
#endif // DOXYGEN_IGNORE

/**
 * The constructor is not doing much. It just initialises the m_logger
 * member.
 */
SCycleBaseBase::SCycleBaseBase()
   : m_logger( "NameNotSet" ) {

   REPORT_VERBOSE( "SCycleBaseBase constructed" );
}

/**
 * Since this base class doesn't inherit from TObject anymore, it can't know
 * the name of the user cycle anymore unfortunately. So all user cycles
 * have to put a line like this in their constructor:
 *
 * <code>
 *   SetLogName( this->GetName() );
 * </code>
 *
 * @param name Name of the current cycle
 */
void SCycleBaseBase::SetLogName( const char* name ) {

   m_logger.SetSource( name );
   return;
}
