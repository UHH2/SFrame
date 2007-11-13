// $Id: SCycleBaseHist.cxx,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author Andreas Hoecker  <Andreas.Hocker@cern.ch>       - CERN
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

// STL include(s):
#include <vector>
#include <string>

// ROOT include(s):
#include <TDirectory.h>

// Local inlcude(s):
#include "../include/SCycleBaseHist.h"

ClassImp( SCycleBaseHist );

using namespace std;

/**
 * Default constructor.
 */
SCycleBaseHist::SCycleBaseHist()
   : m_outputFile( 0 ), m_outputFileName( "" ) {

}

/**
 * Default destructor.
 */
SCycleBaseHist::~SCycleBaseHist() {

}

/**
 * Function to be called for each new output file.
 */
void SCycleBaseHist::InitHistogramming( TDirectory* outputFile,
                                        const TString& outputFileName ) {

   m_outputFile = outputFile;
   m_outputFileName = outputFileName;
   return;

}

/**
 * Function finding the specified directory in the output file if it exists,
 * and creating it if it doesn't. The function is quite slow, so be careful
 * with using it...
 */
TDirectory* SCycleBaseHist::CdInOutput( const char* cpath ) throw( SError ) {

   TDirectory* currentDir = 0;
   // Check whether the directory already exists:
   if( ! ( currentDir = m_outputFile->GetDirectory( m_outputFileName + ":/" + cpath ) ) ) {

      //
      // Break up the supplied string into the directory names that it contains:
      //
      string path( cpath );
      vector< string > directories;
      // To get rid of a leading '/':
      string::size_type previous_pos = ( path[ 0 ] == '/' ? 1 : 0 );
      string::size_type current_pos = 0;
      for( ; ; ) {

         current_pos = path.find( '/', previous_pos );
         directories.push_back( path.substr( previous_pos, current_pos - previous_pos ) );
         if( current_pos == path.npos ) break;
         previous_pos = current_pos + 1;

      }

      //
      // Walk through the directories one by one, creating the ones that
      // don't exist.
      //
      currentDir = m_outputFile;
      TDirectory* tempDir = 0;
      for( vector< string >::const_iterator dir = directories.begin();
           dir != directories.end(); ++dir ) {
         m_logger << VERBOSE << "Going to directory: " << *dir << SLogger::endmsg;
         // If the directory doesn't exist, create it:
         if( ! ( tempDir = currentDir->GetDirectory( dir->c_str() ) ) ) {
            m_logger << VERBOSE << "Directory doesn't exist, creating it..."
                     << SLogger::endmsg;
            if( ! ( tempDir = currentDir->mkdir( dir->c_str(), "dummy title" ) ) ) {
               SError error( SError::SkipInputData );
               error << "Couldn't create directory: " << path
                     << " in the output file!";
               throw error;
            }
         }
         currentDir = tempDir;
      }

   }

   currentDir->cd();
   return currentDir;

}
