// $Id$
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
#include <TList.h>
#include <TFile.h>

// Local inlcude(s):
#include "../include/SCycleBaseHist.h"
#include "../include/SCycleOutput.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseHist )
#endif // DOXYGEN_IGNORE

/**
 * The constructor initialises the base class and the member variables.
 */
SCycleBaseHist::SCycleBaseHist()
   : SCycleBaseBase(), m_histoMap(), m_output( 0 ), m_input( 0 ) {

   REPORT_VERBOSE( "SCycleBaseHist constructed" );
}

void SCycleBaseHist::SetHistOutput( TList* output ) {

   m_output = output;
   m_histoMap.clear();
   return;
}

TList* SCycleBaseHist::GetHistOutput() const {

   return m_output;
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
 *   TGraph mygraph( n, x_array, y_array );<br/>
 *   mygraph.SetName( "MyGraph" );<br/>
 *   WriteObj( mygraph );
 * </code>
 *
 * @param obj       Constant reference to the object to be written out
 * @param directory Optional directory name in which to save the object
 */
void SCycleBaseHist::WriteObj( const TObject& obj,
                               const char* directory ) throw( SError ) {

   GetTempDir()->cd();

   const TString path = ( directory ? directory + TString( "/" ) : "" ) +
      TString( obj.GetName() );
   SCycleOutput* out = dynamic_cast< SCycleOutput* >( m_output->FindObject( path ) );
   if( ! out ) {
      out = new SCycleOutput( obj.Clone(), path, directory );
      m_output->TList::AddLast( out );
   }

   gROOT->cd(); // So that the temporary objects would be created
                // in a general memory space.

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
TH1* SCycleBaseHist::Hist( const char* name, const char* dir ) throw( SError ) {

   TH1* result;

   std::pair< std::string, std::string > this_pair( name, ( dir ? dir : "" ) );
   std::map< std::pair< std::string, std::string >, TH1* >::const_iterator it;
   if( ( it = m_histoMap.find( this_pair ) ) != m_histoMap.end() ) {
      result = it->second;
   } else {
      REPORT_VERBOSE( "Hist(): Using Retrieve for name \""
                      << name << "\" and dir \"" << ( dir ? dir : "" ) << "\"" );
      result = Retrieve< TH1 >( name, dir ); // This line can throw an exception...
      m_histoMap[ this_pair ] = result;
   }

   return result;
}

void SCycleBaseHist::SetHistInputFile( TFile* file ) {

   m_input = file;
   return;
}

TFile* SCycleBaseHist::GetHistInputFile() const {

   return m_input;
}

/**
 * This function is used internally to put all the output TObject-s into a
 * separate directory in memory. This way they don't clash with the objects
 * created in the "default" in-memory directory.
 *
 * @returns A pointer to the common temporary directory
 */
TDirectory* SCycleBaseHist::GetTempDir() const {

   static TDirectory* tempdir = 0;

   if( ! tempdir ) {
      gROOT->cd();
      tempdir = gROOT->mkdir( "SFrameTempDir" );
      if( ! tempdir ) {
         REPORT_ERROR( "Temporary directory could not be created" );
      }
   }

   return tempdir;
}
