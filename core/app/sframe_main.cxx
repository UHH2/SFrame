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
 * With the current implementation of SFrame, it's sufficient to link
 * this file only against the core SFrame library, if your analysis
 * has to use additional libraries, you can specify them in the
 * configuration XML file.
 *
 ***************************************************************************/

// STL include(s):
#include <string>
#include <iostream>

// ROOT include(s):
#include <TROOT.h>

// SFrame include(s):
#include "../include/SCycleController.h"
#include "../include/SError.h"
#include "../include/SLogger.h"

// Global logging object
static SLogger m_logger( "sframe_main" );

// Declaration of function printing the usage of the executable
void usage( char** argv );

int main( int argc, char** argv ) {

   // Check if the application received the expected single configuration file
   // name:
   if( ( argc != 2 ) || ( argc == 2 && std::string( argv[ 1 ] ) == "-h" ) ) {
      usage( argv );
      return 1;
   }

   // A convenience variable:
   const char* filename = argv[ 1 ];

   // Set ROOT into batch mode. This is how PROOF knows not to create
   // graphical windows showing the progress of the event processing.
   gROOT->SetBatch( kTRUE );

   try { // This is where I catch anything not handled internally...

      SCycleController my_analysis( filename );
      my_analysis.Initialize();
      my_analysis.ExecuteAllCycles();

   } catch( const SError& error ) {
      REPORT_FATAL( "SError exception caught" );
      REPORT_FATAL( "Message: " << error.what() );
      REPORT_FATAL( "--> Stopping execution" );
      return 255;
   } catch( const std::exception& error ) {
      REPORT_FATAL( "STD exception caught" );
      REPORT_FATAL( "Message: " << error.what() );
      REPORT_FATAL( "--> Stopping execution" );
      return 254;
   } catch( ... ) {
      REPORT_FATAL( "Some unknown exception caught" );
      REPORT_FATAL( "--> Stopping execution" );
      return 253;
   }

   // Return gracefully:
   return 0;
}

/**
 * This little function is used to print some basic usage information about
 * the executable.
 *
 * @param argv All the command line arguments received by the application
 */
void usage( char** argv ) {

   m_logger << INFO << SLogger::endmsg;
   m_logger << INFO << "Main executable to run an SFrame-based cycle analysis."
            << SLogger::endmsg;
   m_logger << INFO << "\n\tUsage: " << argv[ 0 ] << " \'xml filename\'"
            << std::endl << SLogger::endmsg;

   return;
}
