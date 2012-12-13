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

// System include(s):
#include <sys/types.h>
#include <dirent.h>

// ROOT include(s):
#include <TSystem.h>
#include <TObjArray.h>
#include <TObjString.h>

// Local include(s):
#include "../include/SParLocator.h"

/// Name of the environment variable defining the directories to be searched
static const char* PAR_PATH_NAME = "PAR_PATH";

//
// Initialise the static members:
//
std::list< TString > SParLocator::m_parDirs;
SLogger              SParLocator::m_logger( "SParLocator" );

/**
 * @param parName Name of the file to look for
 * @returns The full path name of the file if found, "" otherwise
 */
TString SParLocator::Locate( const TString& parName ) {

   // Read in the list of directories to be searched:
   if( ! m_parDirs.size() ) {
      ReadParDirs();
   }

   // If the full path name is defined in the configuration, don't bother
   // looking for the file:
   if( parName.Contains( "/" ) ) {
      m_logger << DEBUG << "Treating received file name as full path name..."
               << SLogger::endmsg;
      return parName;
   }

   //
   // Loop over all the directories:
   //
   for( std::list< TString >::const_iterator dir = m_parDirs.begin();
        dir != m_parDirs.end(); ++dir ) {

      //
      // Loop over all the files in the directory (Google magic...)
      // This should be pretty POSIX compliant...
      //
      DIR* d = ::opendir( *dir );
      struct dirent* file;
      while( ( file = ::readdir( d ) ) != NULL ) {
         if( parName == file->d_name ) {
            m_logger << DEBUG << parName << " found in directory: " << *dir
                     << SLogger::endmsg;
            ::closedir( d );
            return ( *dir + "/" + parName );
         }
      }
      ::closedir( d );

   }

   REPORT_ERROR( parName << " couldn't be found" );
   return "";
}

/**
 * This internal function processes the PAR_PATH environment variable by
 * splitting it into separate path names. The result is stored in m_parDirs.
 */
void SParLocator::ReadParDirs() {

   // Clear the current directories:
   m_parDirs.clear();

   //
   // Get the environment variable and split it up:
   //
   TString par_path = gSystem->Getenv( PAR_PATH_NAME );
   TObjArray* par_array = par_path.Tokenize( ":" );

   //
   // Add all proper path names to the list:
   //
   for( Int_t i = 0; i < par_array->GetSize(); ++i ) {

      TObjString* path_element =
         dynamic_cast< TObjString* >( par_array->At( i ) );
      if( ! path_element ) continue;

      if( path_element->GetString() != "" ) {
         m_parDirs.push_back( path_element->GetString() );
      }
   }

   // Clean up after the tokenization:
   delete par_array;

   //
   // Add a failsafe if the environment variable was empty:
   //
   if( ! m_parDirs.size() ) {
      m_logger << WARNING << "No directories set in the " << PAR_PATH_NAME
               << " environment variable" << SLogger::endmsg;
      m_logger << WARNING << "Only the local directory will be searched!"
               << SLogger::endmsg;
      m_parDirs.push_back( "./" );
   }

   return;
}
