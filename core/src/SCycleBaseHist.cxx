// $Id: SCycleBaseHist.cxx,v 1.4 2008-01-28 18:40:33 krasznaa Exp $
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
#include <TH1.h>

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
 * Function for writing any kind of object inheriting from TObject into
 * the output file. It is meant to be used with objects that are
 * created once, then they don't have to be touched again. (Like
 * TGraph and friends.)
 *
 * To write out a TGraph for instance, you could write something like:
 *
 * <code>
 *   TGraph mygraph( n, x_array, y_array );
 *   mygraph.SetName( "MyGraph" );
 *   Write( mygraph );
 * </code>
 *
 * @param obj       Constant reference to the object to be written out
 * @param directory Optional directory name in which to save the object
 */
void SCycleBaseHist::Write( const TObject& obj,
                            const char* directory ) throw( SError ) {

   // Find the correct directory in the output file:
   TDirectory* dir = 0;
   if( directory ) {
      dir = this->CdInOutput( directory );
   } else {
      dir = m_outputFile;
   }
   dir->cd();

   // Persistify the object:
   if( ! obj.Write() ) {
      m_logger << ERROR << "Couldn't write out object \"" << obj.GetName()
               << "\" to directory: " << ( directory ? directory : "" )
               << SLogger::endmsg;
   } else {
      m_logger << VERBOSE << "Persistified object \"" << obj.GetName()
               << "\" to directory: " << ( directory ? directory : "" )
               << SLogger::endmsg;
   }

   gROOT->cd(); // So that the temporary objects would not be
                // created in the file itself...

   return;
}

/**
 * This function is very similar to SCycleBaseHist::Retrieve. It looks for
 * a <strong>1-dimensional histogram</strong> in the output file with a
 * given name in a given directory and returns a pointer to it.
 *
 * The important difference wrt. SCycleBaseHist::Retrieve is that it only
 * uses the slow ROOT methods for finding the histogram once for each
 * new output file. It uses a caching mechanism for all histograms that
 * were already searched for, making the n-th search much faster than
 * that performed by SCycleBaseHist::Retrieve. It's still slower than
 * using separate pointers, but not by much.
 *
 * It should be especially useful when handling a lot of histograms.
 * Having a pointer for each of these histograms can be a pain above
 * a certain number. Instead you can book a histogram once in
 * SCycleBase::BeginInputData and then you can access it with:
 *
 * <code>
 *  In BeginInputData:
 *    Book( TH1D( "hist", "Histogram", 100, 0.0, 100.0 ) );
 *
 *  In ExecuteEvent:
 *    Hist( "hist" )->Fill( 50.0 );
 * </code>
 *
 * @param name The name of the histogram
 * @param dir  The name of the directory the histogram is in
 */
TH1* SCycleBaseHist::Hist( const char* name, const char* dir ) {

   TH1* result;

   pair< const char*, const char* > this_pair( name, dir );
   map< pair< const char*, const char* >, TH1* >::const_iterator it;
   if( ( it = m_histoMap.find( this_pair ) ) != m_histoMap.end() ) {
      result = it->second;
   } else {
      m_logger << VERBOSE << "Hist(): Using Retrieve for name \""
               << name << "\" and dir \"" << ( dir ? dir : "" ) << "\"" << SLogger::endmsg;
      result = m_histoMap[ this_pair ] = Retrieve< TH1 >( name, dir );
   }

   return result;

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
   m_histoMap.clear();
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
