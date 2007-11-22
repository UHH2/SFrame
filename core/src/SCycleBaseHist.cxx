// $Id: SCycleBaseHist.cxx,v 1.2 2007-11-22 18:19:26 krasznaa Exp $
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

// STL include(s):
#include <vector>
#include <string>

// ROOT include(s):
#include <TDirectory.h>

// Local inlcude(s):
#include "../include/SCycleBaseHist.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseHist );
#endif // DOXYGEN_IGNORE

using namespace std;

/**
 * The constructor initialises the base class and the member variables.
 */
SCycleBaseHist::SCycleBaseHist()
   : SCycleBaseBase(), m_outputFile( 0 ), m_outputFileName( "" ) {

   m_logger << VERBOSE << "SCycleBaseHist constructed" << SLogger::endmsg;

}

/**
 * Another one of the "I don't do anything" destructors.
 */
SCycleBaseHist::~SCycleBaseHist() {

   m_logger << VERBOSE << "SCycleBaseHist destructed" << SLogger::endmsg;

}

/**
 * This function is called by the framework to get the object in a
 * configured state when running the analysis. The user is not actually
 * allowed to call this function as its hidden by SCycleBase, but
 * nevertheless: <strong>The users should leave this function
 * alone.</strong>
 *
 * @param outputFile     Pointer to the output file's top directory
 * @param outputFileName Name of the output file
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
