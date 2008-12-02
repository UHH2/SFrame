// $Id: sframe_main.cxx,v 1.3.2.2 2008-12-02 18:50:28 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
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
#include <TError.h>

// SFrame include(s):
#include "../include/SCycleController.h"
#include "../include/SError.h"
#include "../include/SLogger.h"
#include "../include/SErrorHandler.h"

// Global logging object
static SLogger g_logger( "sframe_main" );

// Declaration of function printing the usage of the executable
void usage( char** argv );

int main( int argc, char** argv ) {

   if( ( argc != 2 ) || ( argc == 2 && std::string( argv[ 1 ] ) == "-h" ) ) {
      usage( argv );
      return 1;
   }

   const char* filename = argv[ 1 ];

   gROOT->SetBatch( kTRUE );
   SetErrorHandler( SErrorHandler );

   try { // This is where I catch anything not handled internally...

      SCycleController my_analysis( filename );
      my_analysis.Initialize();
      my_analysis.ExecuteAllCycles();

   } catch( const SError& error ) {

      g_logger << FATAL << "SError exception caught" << SLogger::endmsg;
      g_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      g_logger << FATAL << "--> Stopping execution" << SLogger::endmsg;
      return 1;

   } catch( const std::exception& error ) {

      g_logger << FATAL << "STD exception caught" << SLogger::endmsg;
      g_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      g_logger << FATAL << "--> Stopping execution" << SLogger::endmsg;
      return 1;

   } catch( ... ) {

      g_logger << FATAL << "Some unknown exception caught" << SLogger::endmsg;
      g_logger << FATAL << "--> Stopping execution" << SLogger::endmsg;
      return 1;

   }

   return 0;
}

void usage( char** argv ) {

   g_logger << INFO << SLogger::endmsg;
   g_logger << INFO << "Main executable to run an SFrame-based cycle analysis."
            << SLogger::endmsg;
   g_logger << INFO << "\n\tUsage: " << argv[ 0 ] << " \'xml filename\'"
            << std::endl << SLogger::endmsg;

   return;
}
