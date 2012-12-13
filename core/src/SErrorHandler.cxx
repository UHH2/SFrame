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

// STL include(s):
#include <map>
#include <cstdlib>

// ROOT include(s):
#include <TSystem.h>
#include <TError.h>

// Local include(s):
#include "core/include/SErrorHandler.h"
#include "core/include/SLogger.h"

/// Local map to translate between ROOT and SFrame message levels
static std::map< int, SMsgType > msgLevelMap;

/// Function setting up the usage of SLogger for the ROOT messages
Int_t SetSErrorHandler();

/**
 * This function is the "SFrame version" of DefaultErrorHandler defined in the
 * TError.h header. By calling
 *
 * <code>
 * SetErrorHandler( SErrorHandler )
 * </code>
 *
 * somewhere at the beginning of the application, we can channel all ROOT
 * messages through our own message logging facility.
 *
 * @param level ROOT message level
 * @param abort Flag telling that the process should abort execution
 * @param location The source of the message
 * @param message The message itself
 */
void SErrorHandler( int level, Bool_t abort, const char* location,
                    const char* message ) {

   // Veto some message locations:
   TString tlocation( location );
   if( tlocation.Contains( "NotifyMemory" ) ) {
      return;
   }

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

/**
 * The following code makes sure that
 * <code>SetErrorHandler(SErrorHandler)</code> is called when loading the
 * SFrameCore library. This way all ROOT messages get printed using SLogger on
 * the PROOF workers from the moment the SFrame libraries are loaded. (This is
 * one of the first things that the workers do...)
 *
 * I "stole" the idea for this kind of code from RooFit actually...
 *
 * @returns A dummy integer value
 */
Int_t SetSErrorHandler() {

   // Set up SFrame's error handler:
   SetErrorHandler( SErrorHandler );

   // Report this feat:
   SLogger logger( "SetSErrorHandler" );
   logger << DEBUG << "Redirected ROOT messages to SFrame's logger"
          << SLogger::endmsg;

   return 0;
}

// Call the function:
static Int_t dummy = SetSErrorHandler();
