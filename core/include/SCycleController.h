// Dear emacs, this is -*- c++ -*-
// $Id: SCycleController.h,v 1.3.2.2 2008-12-02 18:50:28 krasznaa Exp $
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
class TProof;
class SCycleBase;

/**
 *   @short Class controlling SFrame analyses
 *
 *          This is the main class that should be instantiated by
 *          the user in an analysis. It takes care of reading the
 *          analysis's configuration from an XML file, creating,
 *          configuring and running all the analysis "cycles".
 *
 *          It is instantiated and configured correctly in the
 *          <strong>sframe_main</strong> executable, so the user
 *          should probably not care about it too much.
 *
 * @version $Revision: 1.3.2.2 $
 */
class SCycleController : public TObject {

public:
   /// Constructor specifying the configuration file
   SCycleController( const TString& xmlConfigFile );
   /// Default destructor
   virtual ~SCycleController();

   /// Initialise the analysis from the configuration file
   virtual void Initialize() throw( SError );
   /// Execute the analysis loop for all configured cycles
   virtual void ExecuteAllCycles() throw( SError );
   /// Execute the analysis loop for the cycle next in line
   virtual void ExecuteNextCycle() throw( SError );
   /// Set the name of the configuration file
   /**
    * All configuration of the analysis is done in a single XML file.
    * The file name from which this configuration should be read
    * is specified with this function.
    */
   virtual void SetConfig( const TString& xmlConfigFile ) { m_xmlConfigFile = xmlConfigFile; }

   /// Add one analysis cycle to the end of all existing cycles
   void AddAnalysisCycle( SCycleBase* cycleAlg );

   /// Get the index of the current cycle
   UInt_t GetCurCycle() { return m_curCycle; }

private:
   /// Delete all analysis cycle objects from memory
   void DeleteAllAnalysisCycles();
   void InitProof( const TString& server );
   void ShutDownProof();
   void WriteCycleOutput( TList* olist, const TString& filename ) const;
   void PrintWorkerLogs() const;

   /// vector holding all analysis cycles to be executed
   std::vector< SCycleBase* > m_analysisCycles;
   /// Packages that have to be loaded on the PROOF cluster
   std::vector< TString > m_parPackages;

   UInt_t  m_curCycle;
   Bool_t  m_isInitialized;
   TString m_xmlConfigFile;

   TProof* m_proof;

   mutable SLogger m_logger;

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleController, 0 );
#endif // DOXYGEN_IGNORE

}; // class SCycleController

#endif // SFRAME_CORE_SCycleController_H
