// $Id: SErrorHandler.cxx,v 1.1.2.3 2008-12-04 17:02:19 krasznaa Exp $
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

// System include(s):
#include <string.h>

// STL include(s):
#include <map>
#include <cstdlib>

// ROOT include(s):
#include <TSystem.h>

// Local include(s):
#include "core/include/SErrorHandler.h"
#include "core/include/SLogger.h"

/// Local map to translate between ROOT and SFrame message levels
static std::map< int, SMsgType > msgLevelMap;

/**
 * This function the "SFrame version" of DefaultErrorHandler defined in the
 * TError.h header. By calling
 *
 * <code>
 * SetErrorHandler( SErrorHandler )
 * </code>
 *
 * somewhere at the beginning of the application, we can channel all ROOT messages
 * through our own message logging facility.
 *
 * @param level ROOT message level
 * @param abort Flag telling that the process should abort execution
 * @param location The source of the message
 * @param message The message itself
 */
void SErrorHandler( int level, Bool_t abort, const char* location,
                    const char* message ) {

   // Create a local logger object:
   SLogger logger( location );

   // Initialise the helper map the first time the function is called:
   if( ! msgLevelMap.size() ) {
      msgLevelMap[ kInfo ]     = INFO;
      msgLevelMap[ kWarning ]  = WARNING;
      msgLevelMap[ kError ]    = ERROR;
      msgLevelMap[ kBreak ]    = ERROR;
      msgLevelMap[ kSysError ] = ERROR;
      msgLevelMap[ kFatal ]    = FATAL;
   }

   // Print the message:
   logger << msgLevelMap[ level ] << message << SLogger::endmsg;

   // Abort the process if necessary:
   if( abort ) {
      logger << ERROR << "Aborting..." << SLogger::endmsg;
      if( gSystem ) {
         gSystem->StackTrace();
         gSystem->Abort();
      } else {
         ::abort();
      }
   }

   return;

}
