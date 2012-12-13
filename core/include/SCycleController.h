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

#ifndef SFRAME_CORE_SCycleController_H
#define SFRAME_CORE_SCycleController_H

// STL include(s):
#include <vector>

// ROOT include(s):
#include "TString.h"

// Local include(s):
#include "SLogger.h"
#include "SError.h"

// Forward declaration(s):
class TProof;
class ISCycleBase;

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
 * @version $Revision$
 */
class SCycleController {

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
   virtual void SetConfig( const TString& xmlConfigFile ) {
      m_xmlConfigFile = xmlConfigFile;
   }

   /// Add one analysis cycle to the end of all existing cycles
   void AddAnalysisCycle( ISCycleBase* cycleAlg );

   /// Get the index of the current cycle
   UInt_t GetCurCycle() { return m_curCycle; }

private:
   /// Delete all analysis cycle objects from memory
   void DeleteAllAnalysisCycles();
   /// "Historic" function initializing the PROOF connection
   void InitProof( const TString& server, Int_t nodes);
   /// "Historic" function, closing the current PROOF connection
   void ShutDownProof();
   /// Function creating/updating the output file of the last cycle
   void WriteCycleOutput( TList* olist, const TString& filename,
                          const TString& config,
                          Bool_t update ) const;

   /// vector holding all analysis cycles to be executed
   std::vector< ISCycleBase* > m_analysisCycles;
   /// Packages that have to be loaded on the PROOF cluster
   std::vector< TString > m_parPackages;

   UInt_t  m_curCycle; ///< Index of the current cycle in the list
   /// Status flag showing if the object is initialized
   Bool_t  m_isInitialized;
   TString m_xmlConfigFile; ///< Name of the configuration file read

   TProof* m_proof; ///< Pointer to the currently used PROOF object

   mutable SLogger m_logger; ///< Message logger object

}; // class SCycleController

#endif // SFRAME_CORE_SCycleController_H
