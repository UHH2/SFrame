// Dear emacs, this is -*- c++ -*-
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
#include "SCycleBaseBase.h"
#include "SError.h"

// Forward declaration(s):
class TDirectory;
class TH1;
class TList;
class SCycleBase;
template< class T > class SToolBaseT;

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
                       public virtual SCycleBaseBase {

   /// To enable the usage of the protected functions for SToolBase
   friend class SToolBaseT< SCycleBase >;

public:
   /// Default constructor
   SCycleBaseHist();

   /// Set which list should be used for the histogramming output
   virtual void SetHistOutput( TList* output );
   /// Check which list should be used for the histogramming output
   virtual TList* GetHistOutput() const;

protected:
   /// Function placing a ROOT object in the output file
   template< class T > T* Book( const T& histo,
                                const char* directory = 0 ) throw( SError );
   /// Function searching for a ROOT object in the output file
   template< class T > T* Retrieve( const char* name,
                                    const char* directory = 0 ) throw( SError );
   /// Function for persistifying a ROOT object to the output
   void WriteObj( const TObject& obj,
                  const char* directory = 0 ) throw( SError );

   /// Function searching for 1-dimensional histograms in the output file
   TH1* Hist( const char* name, const char* dir = 0 );

private:
   TDirectory* GetTempDir() const;

   /// Map used by the Hist function
   std::map< std::pair< std::string, std::string >, TH1* > m_histoMap;

   TList* m_output;

}; // class SCycleBaseHist

// Don't include the templated function(s) when we're generating
// a dictionary:
#ifndef __CINT__
#include "SCycleBaseHist.icc"
#endif

#endif // SFRAME_CORE_SCycleBaseHist_H
