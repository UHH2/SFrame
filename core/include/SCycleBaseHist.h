// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBaseHist.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SCycleBaseHist_H
#define SFRAME_CORE_SCycleBaseHist_H

// ROOT include(s):
#include <TString.h>

// Local include(s):
#include "SCycleBaseBase.h"
#include "SError.h"

// Forward declaration(s):
class TDirectory;

/**
 * Class holding all the histogramming functionality available
 * to the analysis cycles. Unfortunately the object has to be
 * initialised for each new InputData, which is done in the
 * SCycleBase::ExecuteInputData() function. It's error prone, but
 * I haven't found any nicer way of doing it...
 */
class SCycleBaseHist : public virtual SCycleBaseBase {

public:
   SCycleBaseHist();
   virtual ~SCycleBaseHist();

protected:
   template< class T > T* Book( const T& histo,
                                const char* directory = "" ) throw( SError );
   template< class T > T* Retrieve( const char* name,
                                    const char* directory = "" ) throw( SError );

   void InitHistogramming( TDirectory* outputFile, const TString& outputFileName );

private:
   TDirectory* CdInOutput( const char* path ) throw( SError );

   TDirectory* m_outputFile;
   TString     m_outputFileName;

   ClassDef( SCycleBaseHist, 0 );

}; // class SCycleBaseHist

// Don't include the templated function(s) when we're generating
// a dictionary:
#ifndef __CINT__
#include "SCycleBaseHist.icc"
#endif

#endif // SFRAME_CORE_SCycleBaseHist_H
