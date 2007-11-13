// Dear emacs, this is -*- c++ -*-
// $Id: SCycleController.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SCycleController_H
#define SFRAME_CORE_SCycleController_H

// STL include(s):
#include <vector>

// ROOT include(s):
#include "TObject.h"
#include "TString.h"

// Local include(s):
#include "SLogger.h"
#include "SError.h"

// Forward declaration(s):
class SCycleBase;

/**
 * This is the main class that should be instantiated by the user in
 * an analysis. It takes care of reading the analysis's configuration
 * from an XML file, creating, configuring and running all the
 * analysis "cycles".
 */
class SCycleController : public TObject {

public:
   SCycleController( const TString& xmlConfigFile );
   virtual ~SCycleController();

   virtual void Initialize() throw( SError );
   virtual void ExecuteAllCycles() throw( SError );
   virtual void ExecuteNextCycle() throw( SError );
   virtual void SetConfig( const TString& xmlConfigFile ) { m_xmlConfigFile = xmlConfigFile; }

   /// Add one analysis cycle to the end of all existing cycles.
   void AddAnalysisCycle( SCycleBase* cycleAlg );

   virtual UInt_t GetCurCycle() { return m_curCycle; }

private:
   /// Delete all analysis cycle objects from memory!
   void DeleteAllAnalysisCycles();

   /// vector holding all analysis cycles to be executed
   std::vector< SCycleBase* > m_analysisCycles;

   UInt_t  m_curCycle;
   Bool_t  m_isInitialized;
   TString m_xmlConfigFile;

   mutable SLogger m_logger;

   ClassDef( SCycleController, 0 );

}; // class SCycleController

#endif // SFRAME_CORE_SCycleController_H
