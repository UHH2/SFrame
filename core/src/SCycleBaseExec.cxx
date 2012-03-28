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

// System include(s):
#include <cstdlib>

// ROOT include(s):
#include <TTree.h>
#include <TFile.h>
#include <TProofOutputFile.h>
#include <TSystem.h>
#include <TTreeCache.h>

// Local include(s):
#include "../include/SCycleBaseExec.h"
#include "../include/SInputData.h"
#include "../include/SCycleConfig.h"
#include "../include/SConstants.h"
#include "../include/SCycleStatistics.h"
#include "../include/SOutputFile.h"
#include "../include/SLogWriter.h"
#include "../include/STreeType.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseExec );
#endif // DOXYGEN_IGNORE

/**
 * The constructor just initialises some member variable(s).
 */
SCycleBaseExec::SCycleBaseExec()
   : m_nProcessedEvents( 0 ), m_nSkippedEvents( 0 ), m_treeCache( 0 ) {

   SetLogName( this->GetName() );
   REPORT_VERBOSE( "SCycleBaseExec constructed" );
}

void SCycleBaseExec::Begin( TTree* ) {

   REPORT_VERBOSE( "Running initialization on master" );

   try {

      //
      // Configure the base classes to write to the TSelector output object:
      //
      this->SetHistOutput( fOutput );
      this->SetNTupleOutput( fOutput );
      this->SetConfInput( fInput );

      this->ReadConfig();
      this->BeginMasterInputData( *m_inputData );

   } catch( const SError& error ) {
      REPORT_FATAL( "Exception caught with message: " << error.what() );
      throw;
   }

   return;
}

void SCycleBaseExec::SlaveBegin( TTree* ) {

   REPORT_VERBOSE( "Running initialization on slave" );

   try {

      this->ReadConfig();

      //
      // Configure the base classes to write to the TSelector output object:
      //
      this->SetHistOutput( fOutput );
      this->SetNTupleOutput( fOutput );
      this->SetConfInput( fInput );

      m_outputTrees.clear();

      //
      // Open a PROOF output file for the ntuple(s):
      //
      if( m_inputData->GetTrees( STreeType::OutputSimpleTree ) ||
          m_inputData->GetTrees( STreeType::OutputMetaTree ) ) {

         TProofOutputFile* proofFile = 0;
         char* tempDirName = 0;

         TNamed* out =
            dynamic_cast< TNamed* >( fInput->FindObject( SFrame::ProofOutputName ) );
         if( out ) {
            proofFile =
               new TProofOutputFile( gSystem->BaseName( TUrl( out->GetTitle() ).GetFile() ) );
            proofFile->SetOutputFileName( out->GetTitle() );
            tempDirName = 0;
            fOutput->Add( proofFile );
         } else {
            m_logger << DEBUG << "No PROOF output file specified in configuration -> "
                     << "Running in LOCAL mode" << SLogger::endmsg;
            proofFile = 0;
            // Use a more or less POSIX method for creating a unique file name:
            tempDirName = new char[ 100 ];
            sprintf( tempDirName, "%s", SFrame::ProofOutputDirName );
            if( ! mkdtemp( tempDirName ) ) {
               REPORT_FATAL( "Couldn't create temporary directory name from template: "
                             << SFrame::ProofOutputDirName );
               return;
            }
            fOutput->Add( new SOutputFile( "SFrameOutputFile", TString( tempDirName ) +
                                           "/" +  SFrame::ProofOutputFileName ) );
         }

         if( proofFile ) {
            if( ! ( m_outputFile = proofFile->OpenFile( "RECREATE" ) ) ) {
               m_logger << WARNING << "Couldn't open output file: "
                        << proofFile->GetDir() << "/" << proofFile->GetFileName()
                        << SLogger::endmsg;
               m_logger << WARNING << "Saving the ntuples to memory" << SLogger::endmsg;
            } else {
               m_logger << DEBUG << "PROOF temp file opened with name: "
                        << m_outputFile->GetName() << SLogger::endmsg;
            }
         } else {
            if( ! tempDirName ) {
               REPORT_FATAL( "No temporary directory name? There's some serious error "
                             "in the code!" );
               return;
            }

            // Open an intermediate file in this temporary directory:
            if( ! ( m_outputFile = TFile::Open( TString( tempDirName ) + "/" +
                                                SFrame::ProofOutputFileName , "RECREATE" ) ) ) {
               m_logger << WARNING << "Couldn't open output file: "
                        << tempDirName << "/" << SFrame::ProofOutputFileName << SLogger::endmsg;
               m_logger << WARNING << "Saving the ntuples to memory" << SLogger::endmsg;
            } else {
               m_logger << DEBUG << "LOCAL temp file opened with name: "
                        << tempDirName << "/" << SFrame::ProofOutputFileName << SLogger::endmsg;
            }
         }

         this->CreateOutputTrees( *m_inputData, m_outputTrees, m_outputFile );

         if( tempDirName ) delete[] tempDirName;

      } else {
         m_outputFile = 0;
      }

      this->BeginInputData( *m_inputData );

   } catch( const SError& error ) {
      REPORT_FATAL( "Exception caught with message: " << error.what() );
      throw;
   }

   m_nProcessedEvents = 0;
   m_nSkippedEvents = 0;
   m_firstInit = kTRUE;

   m_logger << INFO << "Initialised InputData \"" << m_inputData->GetType()
            << "\" (Version:" << m_inputData->GetVersion()
            << ") on worker node" << SLogger::endmsg;

   return;
}

void SCycleBaseExec::Init( TTree* main_tree ) {

   REPORT_VERBOSE( "Caching the pointer to the main input tree" );
   m_inputTree = main_tree;

   return;
}

Bool_t SCycleBaseExec::Notify() {

   REPORT_VERBOSE( "Accessing a new input file" );

   // Should not run the initialization when it's first called in LOCAL mode.
   // ROOT always calls Notify() twice in this mode. Note that this behavior
   // might change in future ROOT versions...
   if( ( GetConfig().GetRunMode() == SCycleConfig::LOCAL ) && m_firstInit ) {
      m_firstInit = kFALSE;
      return kTRUE;
   }

   TFile* inputFile = 0;
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
      // Otherwise (it's 0) trust that the user already added all the necessary branches
      // inside BeginInputFile(...).
      if( GetConfig().GetCacheLearnEntries() < 0 ) {
         m_inputTree->AddBranchToCache( "*", kTRUE );
      }
      m_inputTree->StopCacheLearningPhase();
   }

   // If the tree caching is turned on in LOCAL mode, make sure that this
   // actually happens:
   if( GetConfig().GetUseTreeCache() &&
       ( GetConfig().GetRunMode() == SCycleConfig::LOCAL ) ) {
      if( ! inputFile ) {
         m_logger << WARNING << "No input file? Can't set up TTreeCache!" << SLogger::endmsg;
      } else {
         m_inputTree->SetCacheSize( GetConfig().GetCacheSize() );
         m_treeCache = dynamic_cast< TTreeCache* >( inputFile->GetCacheRead() );
         if( ! m_treeCache ) {
            REPORT_FATAL( "Couldn't create TTreeCache" );
            throw SError( "Couldn't create TTreeCache",
                          SError::StopExecution );
         }
         m_treeCache->UpdateBranches( m_inputTree );
      }
   }
#endif // ROOT_VERSION...

   return kTRUE;
}

Bool_t SCycleBaseExec::Process( Long64_t entry ) {

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

   if( ! skipEvent ) {
      int nbytes = 0;
      for( std::vector< TTree* >::iterator tree = m_outputTrees.begin();
           tree != m_outputTrees.end(); ++tree ) {
         nbytes = ( *tree )->Fill();
         if( nbytes < 0 ) {
            REPORT_ERROR( "Write error occured in tree \""
                          << ( *tree )->GetName() << "\"" );
         } else if( nbytes == 0 ) {
            m_logger << WARNING << "No data written to tree \""
                     << ( *tree )->GetName() << "\"" << SLogger::endmsg;
         }
      }
   } else {
      ++m_nSkippedEvents;
   }

   ++m_nProcessedEvents;
   if( ! ( m_nProcessedEvents % 1000 ) ) {
      // Only print these messages in local mode in INFO level. In PROOF mode they're
      // only needed for debugging.
      m_logger << ( GetConfig().GetRunMode() == SCycleConfig::LOCAL ? INFO : DEBUG )
               << "Processing entry: " << entry << " ("
               << ( m_nProcessedEvents - 1 ) << " / "
               << ( m_inputData->GetNEventsMax() < 0 ? m_inputData->GetEventsTotal() :
                    m_inputData->GetNEventsMax() )
               << " events processed so far)" << SLogger::endmsg;
   }

   return kTRUE;
}

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
   // Write the node statistics to the output:
   //
   SCycleStatistics* stat = new SCycleStatistics( SFrame::RunStatisticsName,
                                                  m_nProcessedEvents, m_nSkippedEvents );
   fOutput->Add( stat );

   //
   // Close the output file:
   //
   if( m_outputFile ) {

      m_logger << DEBUG << "Closing output file: " << m_outputFile->GetName()
               << SLogger::endmsg;

      // Save all the output trees into the output file:
      this->SaveOutputTrees( m_outputFile );

      // Close the output file and reset the variables:
      m_outputFile->SaveSelf( kTRUE );
      m_outputFile->Close();
      delete m_outputFile;
      m_outputFile = 0;
      m_outputTrees.clear();

   }

   // Reset the ntuple handling component:
   this->ClearCachedTrees();

   m_logger << INFO << "Terminated InputData \"" << m_inputData->GetType()
            << "\" (Version:" << m_inputData->GetVersion()
            << ") on worker node" << SLogger::endmsg;

   return;
}

void SCycleBaseExec::Terminate() {

   REPORT_VERBOSE( "Running finalization on the master" );

   try {
      this->EndMasterInputData( *m_inputData );
   } catch( const SError& error ) {
      REPORT_FATAL( "Exception caught with message: " << error.what() );
      throw;
   }

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
   SCycleConfig* config =
      dynamic_cast< SCycleConfig* >( fInput->FindObject( SFrame::CycleConfigName ) );
   if( ! config ) {
      REPORT_FATAL( "Couldn't retrieve the cycle configuration" );
      throw SError( "Couldn't find cycle configuration object", SError::SkipCycle );
      return;
   }
   this->SetConfig( *config );
   SLogWriter::Instance()->SetMinType( config->GetMsgLevel() );

   //
   // Read which InputData we're processing at the moment:
   //
   m_inputData =
      dynamic_cast< SInputData* >( fInput->FindObject( SFrame::CurrentInputDataName ) );
   if( ! m_inputData ) {
      REPORT_FATAL( "Couldn't retrieve the input data definition currently "
                    << "being processed" );
      throw SError( "Couldn't find current input data configuration object",
                    SError::SkipCycle );
      return;
   }

   return;
}
