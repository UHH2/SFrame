// Dear emacs, this is -*- c++ -*-
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

#ifndef SFRAME_CORE_SCycleBaseHist_H
#define SFRAME_CORE_SCycleBaseHist_H

// STL include(s):
#include <map>
#include <string>

// ROOT include(s):
#include <TObject.h>
#include <TString.h>

// Local include(s):
#include "ISCycleBaseHist.h"
#include "ISCycleBaseNTuple.h"
#include "SCycleBaseBase.h"
#include "SError.h"

// Forward declaration(s):
class TDirectory;
class TH1;
class TList;

/**
 *   @short Histogramming part of SCycleBase
 *
 *          Class holding all the histogramming functionality
 *          available to the analysis cycles. Unfortunately
 *          the object has to be initialised for each new InputData,
 *          which is done in the SCycleBase::ExecuteInputData function.
 *          It's error prone, but I haven't found any nicer way of
 *          doing it...
 *
 * @version $Revision$
 */
class SCycleBaseHist : public virtual ISCycleBaseHist,
                       public virtual ISCycleBaseNTuple,
                       public virtual SCycleBaseBase {

public:
   /// Default constructor
   SCycleBaseHist();

   /// Set which list should be used for the histogramming output
   virtual void SetHistOutput( TList* output );
   /// Check which list should be used for the histogramming output
   virtual TList* GetHistOutput() const;

   /// Function placing a ROOT object in the output file
   template< class T > T* Book( const T& histo,
                                const char* directory = 0,
                                Bool_t inFile = kFALSE ) throw( SError );
   /// Function searching for a ROOT object in the output file
   template< class T > T* Retrieve( const char* name,
                                    const char* directory = 0,
                                    Bool_t outputOnly = kFALSE ) throw( SError );
   /// Function retrieving all ROOT objects of this name from the input file
   template< class T >
   std::vector< T* > RetrieveAll( const char* name,
                                  const char* directory = 0 ) throw( SError );
   /// Function for persistifying a ROOT object to the output
   void WriteObj( const TObject& obj,
                  const char* directory = 0,
                  Bool_t inFile = kFALSE ) throw( SError );

   /// Function searching for 1-dimensional histograms in the output file
   TH1* Hist( const char* name, const char* dir = 0 ) throw( SError );

protected:
   /// Set the current input file
   virtual void SetHistInputFile( TDirectory* file );
   /// Get the currently set input file
   virtual TDirectory* GetHistInputFile() const;

   /// Write the objects meant to be merged using the output file
   virtual void WriteHistObjects();

private:
   /// Function creating a temporary directory in memory
   TDirectory* GetTempDir() const;

#ifndef __MAKECINT__
   /// Map used by the Hist function
   std::map< std::pair< std::string, std::string >, TH1* > m_histoMap;
   /// List of objects to be merged using the output file
   TList m_fileOutput;
#endif // __MAKECINT__

   TList* m_proofOutput; ///< PROOF output list
   TDirectory* m_inputFile; ///< Currently open input file

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleBaseHist, 0 )
#endif // DOXYGEN_IGNORE

}; // class SCycleBaseHist

// Don't include the templated function(s) when we're generating
// a dictionary:
#ifndef __CINT__
#include "SCycleBaseHist.icc"
#endif

#endif // SFRAME_CORE_SCycleBaseHist_H
