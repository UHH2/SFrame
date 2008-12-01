// $Id: SCycleBaseBase.cxx,v 1.2.2.1 2008-12-01 14:52:56 krasznaa Exp $
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
#include "../include/SCycleBaseBase.h"

/*
#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseBase );
#endif // DOXYGEN_IGNORE
*/

/**
 * The constructor is not doing much. It just initialises the m_logger
 * member.
 */
SCycleBaseBase::SCycleBaseBase()
   : m_logger( "NameNotSet" ) {

   m_logger << VERBOSE << "SCycleBaseBase constructed" << SLogger::endmsg;

}

/**
 * Another one of the "I don't do anything" destructors.
 */
SCycleBaseBase::~SCycleBaseBase() {

   m_logger << VERBOSE << "SCycleBaseBase destructed" << SLogger::endmsg;

}

void SCycleBaseBase::SetLogName( const char* name ) {

   m_logger.SetSource( name );
   return;

}
