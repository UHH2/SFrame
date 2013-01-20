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

// ROOT include(s):
#include <TTree.h>
#include <TSystem.h>

// Local include(s):
#include "../include/SCycleBaseExec.h"
#include "../include/SInputData.h"
#include "../include/SCycleConfig.h"
#include "../include/SCycleStatistics.h"
#include "../include/SLogWriter.h"
#include "../include/STreeType.h"
#include "../include/SConstants.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseExec )
#endif // DOXYGEN_IGNORE

/**
 * The constructor just initialises some member variable(s).
 */
SCycleBaseExec::SCycleBaseExec()
   : m_nProcessedEvents( 0 ), m_nSkippedEvents( 0 ) {

   SetLogName( this->GetName() );
   REPORT_VERBOSE( "SCycleBaseExec constructed" );
}

/**
 * This function is called by ROOT/PROOF when the processing of a job (input
 * data) starts on the PROOF master. In LOCAL mode it is just called before
 * all the other functions before starting to process the events of the current
 * input data.
 */
void SCycleBaseExec::Begin( TTree* ) {

   REPORT_VERBOSE( "Running initialization on master" );

   try {

      //
      // Configure the base classes:
      //
      this->SetHistOutput( fOutput );
      this->SetNTupleInput( fInput );
      this->SetNTupleOutput( fOutput );
      this->SetConfInput( fInput );

      // Make sure the configuration is available for the cycle:
      this->ReadConfig();
      // Let the user initialize his/her code:
      this->BeginMasterInputData( *m_inputData );

   } catch( const SError& error ) {
      REPORT_FATAL( "Exception caught with message: " << error.what() );
      throw;
   }

   return;
}

/**
 * This function is called by ROOT/PROOF when the processing of a job (input
 * data) starts on the PROOF worker. In LOCAL mode it is called after
 * <code>Begin(...)</code>, before the event processing would start.
 */
void SCycleBaseExec::SlaveBegin( TTree* ) {

   REPORT_VERBOSE( "Running initialization on slave" );

   try {

      // Read the cycle/input data configuration:
      this->ReadConfig();

      //
      // Configure the base classes:
      //
      this->SetHistOutput( fOutput );
      this->SetNTupleInput( fInput );
      this->SetNTupleOutput( fOutput );
      this->SetConfInput( fInput );

      m_outputTrees.clear();

      //
      // Create the output tree(s) if necessary:
      //
      if( m_inputData->GetTrees( STreeType::OutputSimpleTree ) ||
          m_inputData->GetTrees( STreeType::OutputMetaTree ) ) {
         this->CreateOutputTrees( *m_inputData, m_outputTrees );
      }

      // Let the user code initialize itself:
      this->BeginInputData( *m_inputData );

   } catch( const SError& error ) {
      REPORT_FATAL( "Exception caught with message: " << error.what() );
      throw;
   }

   // Reset the internal variable(s):
   m_nProcessedEvents = 0;
   m_nSkippedEvents = 0;
   m_firstInit = kTRUE;

   // Print what just happened:
   m_logger << INFO << "Initialised InputData \"" << m_inputData->GetType()
            << "\" (Version:" << m_inputData->GetVersion()
            << ") on worker node" << SLogger::endmsg;

   return;
}

/**
 * This function is called by ROOT/PROOF when a new file is opened by the
 * analysis. But the code doesn't try to connect to the input file at this
 * point just yet, it only stores the pointer to the main TTree of the input
 * file, and only goes through with the initialization of the file in
 * <code>Notify()</code>.
 *
 * @param main_tree The main event-level TTree of the input files
 */
void SCycleBaseExec::Init( TTree* main_tree ) {

   REPORT_VERBOSE( "Caching the pointer to the main input tree" );
   m_inputTree = main_tree;

   return;
}

/**
 * This function is called by ROOT/PROOF when a new input file should be
 * connected to. The code accesses all the input TTrees of the newly opened
 * input file at this point, and lets the user code connect to the input
 * file's objects.
 *
 * @returns <code>kTRUE</code> if everything went fine, <code>kFALSE</code>
 *          if there was some problem
 */
Bool_t SCycleBaseExec::Notify() {

   REPORT_VERBOSE( "Accessing a new input file" );

   // Should not run the initialization when it's first called in LOCAL mode.
   // ROOT always calls Notify() twice in this mode. Note that this behavior
   // might change in future ROOT versions...
   if( ( GetConfig().GetRunMode() == SCycleConfig::LOCAL ) && m_firstInit ) {
      m_firstInit = kFALSE;
      return kTRUE;
   }

   // Connect to all objects of the input file:
   TDirectory* inputFile = 0;
   try {

      this->LoadInputTrees( *m_inputData, m_inputTree, inputFile );
      this->SetHistInputFile( inputFile );
      this->BeginInputFile( *m_inputData );

   } catch( const SError& error ) {
      REPORT_FATAL( "Exception caught with message: " << error.what() );
      throw;
   }

#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 26, 0 )
   // Tell the cache to learn the access pattern for the configured number
   // of entries:
   if( GetConfig().GetCacheLearnEntries() > 0 ) {
      m_inputTree->SetCacheLearnEntries( GetConfig().GetCacheLearnEntries() );
   } else {
      // If it's set to a negative number, add all the branches to the cache.
      // Otherwise (it's 0) trust that the user already added all the necessary
      // branches inside BeginInputFile(...).
      if( GetConfig().GetCacheLearnEntries() < 0 ) {
         m_inputTree->AddBranchToCache( "*", kTRUE );
      }
      m_inputTree->StopCacheLearningPhase();
   }

   // According to user reports, trying to turn on TTreeCache in LOCAL mode
   // leads to hard-to-detect, but serious problems. (The results don't match
   // up with the ones acquired without using a cache.) So, for now the code
   // doesn't try to use a cache in this case.
   if( GetConfig().GetUseTreeCache() &&
       ( GetConfig().GetRunMode() == SCycleConfig::LOCAL ) ) {
      m_logger << WARNING << "Can't use a TTreeCache in LOCAL mode, sorry..."
               << SLogger::endmsg;
   }
#endif // ROOT_VERSION...

   // Return gracefully:
   return kTRUE;
}

/**
 * This function is called by ROOT/PROOF to process one particular event in the
 * analysis code. The cycle makes sure that the current event is loaded from the
 * input file, and lets the cycle's execution function run.
 *
 * @param entry The entry that should be processed from the input TTree(s)
 * @returns <code>kTRUE</code> if everything went correctly, or
 *          <code>kFALSE</code> if there was a problem
 */
Bool_t SCycleBaseExec::Process( Long64_t entry ) {

   // Execute the analysis code, looking out for any thrown exceptions:
   Bool_t skipEvent = kFALSE;
   try {

      this->GetEvent( entry );
      m_inputData->SetEventTreeEntry( entry );
      this->ExecuteEvent( *m_inputData, this->CalculateWeight( *m_inputData,
                                                               entry ) );

   } catch( const SError& error ) {
      if( error.request() <= SError::SkipEvent ) {
         REPORT_VERBOSE( "Exeption caught while processing event" );
         REPORT_VERBOSE( " Message: " << error.what() );
         REPORT_VERBOSE( " --> Skipping event!" );
         skipEvent = kTRUE;
      } else {
         REPORT_FATAL( "Exception caught while processing event" );
         REPORT_FATAL( "Message: " << error.what() );
         throw;
      }
   }

   // Write a new event to the output TTree(s) if the event doesn't have to be
   // skipped:
   if( ! skipEvent ) {
      int nbytes = 0;
      std::vector< TTree* >::iterator tree_itr = m_outputTrees.begin();
      std::vector< TTree* >::iterator tree_end = m_outputTrees.end();
      for( ; tree_itr != tree_end; ++tree_itr ) {
         nbytes = ( *tree_itr )->Fill();
         if( nbytes < 0 ) {
            REPORT_ERROR( "Write error occured in tree \""
                          << ( *tree_itr )->GetName() << "\"" );
            // Stop the execution, as this is a serious problem:
            throw SError( "TTree write error occured",
                          SError::StopExecution );
         } else if( nbytes == 0 ) {
            m_logger << WARNING << "No data written to tree \""
                     << ( *tree_itr )->GetName() << "\"" << SLogger::endmsg;
         }
      }
   } else {
      ++m_nSkippedEvents;
   }

   ++m_nProcessedEvents;
   if( ! ( m_nProcessedEvents % 1000 ) ) {
      // Only print these messages in local mode in INFO level. In PROOF mode
      // they're only needed for debugging.
      m_logger << ( GetConfig().GetRunMode() == SCycleConfig::LOCAL ? INFO :
                    DEBUG )
               << "Processing entry: " << entry << " ("
               << ( m_nProcessedEvents - 1 ) << " / "
               << ( m_inputData->GetNEventsMax() < 0 ?
                    m_inputData->GetEventsTotal() :
                    m_inputData->GetNEventsMax() )
               << " events processed so far)" << SLogger::endmsg;
   }

   // Return gracefully:
   return kTRUE;
}

/**
 * This function is called by ROOT/PROOF on the worker nodes when the event
 * processing finished. The code first lets the user code do any final
 * operations by calling <code>EndInputData(...)</code>, then makes sure that
 * all output information is properly stored, to end up in the correct place.
 */
void SCycleBaseExec::SlaveTerminate() {

   REPORT_VERBOSE( "Running finalization on slave" );

   //
   // Tell the user cycle that the InputData has ended:
   //
   try {
      this->EndInputData( *m_inputData );
   } catch( const SError& error ) {
      REPORT_FATAL( "Exception caught with message: " << error.what() );
      throw;
   }

   //
   // Write the objects that are meant to be merged in-file, into
   // the output file:
   //
   this->WriteHistObjects();

   //
   // Write the node statistics to the output:
   //
   SCycleStatistics* stat = new SCycleStatistics( SFrame::RunStatisticsName,
                                                  m_nProcessedEvents,
                                                  m_nSkippedEvents );
   fOutput->Add( stat );

   // Close the output file:
   this->CloseOutputFile();

   // Reset the ntuple handling component:
   this->ClearCachedTrees();

   m_logger << INFO << "Terminated InputData \"" << m_inputData->GetType()
            << "\" (Version:" << m_inputData->GetVersion()
            << ") on worker node" << SLogger::endmsg;

   // Return gracefully:
   return;
}

/**
 * This function is called by ROOT/PROOF on the master node after all events
 * have been processed. The code just calls the user's
 * <code>EndMasterInputData(...)</code> function, and doesn't do anything else
 * in addition itself.
 */
void SCycleBaseExec::Terminate() {

   REPORT_VERBOSE( "Running finalization on the master" );

   try {
      this->EndMasterInputData( *m_inputData );
   } catch( const SError& error ) {
      REPORT_FATAL( "Exception caught with message: " << error.what() );
      throw;
   }

   // Return gracefully:
   return;
}

/**
 * This function takes care of accessing the cycle configuration objects on the
 * master and worker nodes.
 */
void SCycleBaseExec::ReadConfig() throw( SError ) {

   //
   // Read the overall cycle configuration:
   //
   TObject* tobj = fInput->FindObject( SFrame::CycleConfigName );
   SCycleConfig* config = dynamic_cast< SCycleConfig* >( tobj );
   if( ! config ) {
      REPORT_FATAL( "Couldn't retrieve the cycle configuration" );
      throw SError( "Couldn't find cycle configuration object",
                    SError::SkipCycle );
      return;
   }

   // Configure this object:
   this->SetConfig( *config );
   SLogWriter::Instance()->SetMinType( config->GetMsgLevel() );

   //
   // Read which InputData we're processing at the moment:
   //
   tobj = fInput->FindObject( SFrame::CurrentInputDataName );
   m_inputData = dynamic_cast< SInputData* >( tobj );
   if( ! m_inputData ) {
      REPORT_FATAL( "Couldn't retrieve the input data definition currently "
                    "being processed" );
      throw SError( "Couldn't find current input data configuration object",
                    SError::SkipCycle );
      return;
   }

   // Return gracefully:
   return;
}

/**
 * I just implemented this function to get rid of a warning seen with some
 * compilers about this class hiding TObject's ExecuteEvent function...
 */
void SCycleBaseExec::ExecuteEvent( Int_t /*event*/, Int_t /*px*/,
                                   Int_t /*py*/ ) {

   REPORT_ERROR( "This function should never get called!" );
   return;
}
