// $Id: SCycleBaseExec.cxx,v 1.4.2.5 2009-06-15 11:10:13 krasznaa Exp $
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

// ROOT include(s):
#include <TTree.h>
#include <TFile.h>
#include <TProofOutputFile.h>
#include <TSystem.h>

// Local include(s):
#include "../include/SCycleBaseExec.h"
#include "../include/SInputData.h"
#include "../include/SCycleConfig.h"
#include "../include/SConstants.h"
#include "../include/SCycleStatistics.h"
#include "../include/SOutputFile.h"
#include "../include/SLogWriter.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseExec );
#endif // DOXYGEN_IGNORE

/**
 * The constructor just initialises some member variable(s).
 */
SCycleBaseExec::SCycleBaseExec()
   : m_nProcessedEvents( 0 ), m_nSkippedEvents( 0 ) {

   SetLogName( this->GetName() );
   m_logger << VERBOSE << "SCycleBaseExec constructed" << SLogger::endmsg;

}

/**
 * The destructor doesn't do anything.
 */
SCycleBaseExec::~SCycleBaseExec() {

   m_logger << VERBOSE << "SCycleBaseExec destructed" << SLogger::endmsg;

}

void SCycleBaseExec::Begin( TTree* ) {

   m_logger << VERBOSE << "In SCycleBaseExec::Begin()" << SLogger::endmsg;

   try {

      //
      // Configure the base classes to write to the TSelector output object:
      //
      this->SetHistOutput( fOutput );
      this->SetNTupleOutput( fOutput );

      this->ReadConfig();
      this->BeginMasterInputData( *m_inputData );

   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught in Begin( TTree* )"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      throw;
   }

   m_logger << INFO << "Initialised cycle on master node" << SLogger::endmsg;

   return;

}

void SCycleBaseExec::SlaveBegin( TTree* ) {

   m_logger << VERBOSE << "In SCycleBaseExec::SlaveBegin()" << SLogger::endmsg;

   try {

      this->ReadConfig();

      //
      // Configure the base classes to write to the TSelector output object:
      //
      this->SetHistOutput( fOutput );
      this->SetNTupleOutput( fOutput );

      m_outputTrees.clear();

      //
      // Open a PROOF output file for the ntuple(s):
      //
      if( m_inputData->GetOutputTrees().size() ) {

         TProofOutputFile* proofFile = 0;

         TNamed* out =
            dynamic_cast< TNamed* >( fInput->FindObject( SFrame::ProofOutputName ) );
         if( out ) {
            proofFile =
               new TProofOutputFile( gSystem->BaseName( TUrl( out->GetTitle() ).GetFile() ) );
            proofFile->SetOutputFileName( out->GetTitle() );
            fOutput->Add( proofFile );
         } else {
            m_logger << DEBUG << "No PROOF output file specified in configuration -> "
                     << "Running in LOCAL mode" << SLogger::endmsg;
            proofFile = 0;
            fOutput->Add( new SOutputFile( "SFrameOutputFile", SFrame::ProofOutputFileName ) );
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
            if( ! ( m_outputFile = new TFile( SFrame::ProofOutputFileName, "RECREATE" ) ) ) {
               m_logger << WARNING << "Couldn't open output file: "
                        << SFrame::ProofOutputFileName << SLogger::endmsg;
               m_logger << WARNING << "Saving the ntuples to memory" << SLogger::endmsg;
            } else {
               m_logger << DEBUG << "LOCAL temp file opened with name: "
                        << SFrame::ProofOutputFileName << SLogger::endmsg;
            }
         }

         this->CreateOutputTrees( *m_inputData, m_outputTrees, m_outputFile );

      }

      this->BeginInputData( *m_inputData );

   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught in SlaveBegin( TTree* )"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      throw;
   }

   m_nProcessedEvents = 0;
   m_nSkippedEvents = 0;

   m_logger << INFO << "Initialised InputData \"" << m_inputData->GetType()
            << "\" (Version:" << m_inputData->GetVersion()
            << ") on worker node" << SLogger::endmsg;

   return;

}

void SCycleBaseExec::Init( TTree* main_tree ) {

   m_logger << VERBOSE << "In SCycleBaseExec::Init(...)" << SLogger::endmsg;

   m_inputTree = main_tree;

   return;

}

Bool_t SCycleBaseExec::Notify() {

   try {

      this->LoadInputTrees( *m_inputData, m_inputTree );
      this->BeginInputFile( *m_inputData );

   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught in Notify()"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      throw;
   }

   return kTRUE;

}

Bool_t SCycleBaseExec::Process( Long64_t entry ) {

   Bool_t skipEvent = kFALSE;
   try {

      this->GetEvent( entry );
      this->ExecuteEvent( *m_inputData, this->CalculateWeight( *m_inputData,
                                                               entry ) );

   } catch( const SError& error ) {
      if( error.request() <= SError::SkipEvent ) {
         m_logger << VERBOSE << "Exeption caught while processing event"
                  << SLogger::endmsg;
         m_logger << VERBOSE << "Message: " << error.what() << SLogger::endmsg;
         m_logger << VERBOSE << "--> Skipping event!" << SLogger::endmsg;
         skipEvent = kTRUE;
      } else {
         m_logger << FATAL << "Exception caught while processing event"
                  << SLogger::endmsg;
         m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
         throw;
      }
   }

   if( ! skipEvent ) {
      int nbytes = 0;
      for( std::vector< TTree* >::iterator tree = m_outputTrees.begin();
           tree != m_outputTrees.end(); ++tree ) {
         nbytes = ( *tree )->Fill();
         if( nbytes < 0 ) {
            m_logger << ERROR << "Write error occured in tree \""
                     << ( *tree )->GetName() << "\"" << SLogger::endmsg;
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
      m_logger << INFO << "Processing entry: " << entry << " ("
               << ( m_nProcessedEvents - 1 ) << " / "
               << ( m_inputData->GetNEventsMax() < 0 ? m_inputData->GetEventsTotal() :
                    m_inputData->GetNEventsMax() )
               << " events processed so far)" << SLogger::endmsg;
   }

   return kTRUE;

}

void SCycleBaseExec::SlaveTerminate() {

   m_logger << VERBOSE << "In SCycleBaseExec::SlaveTerminate()" << SLogger::endmsg;

   try {
      this->EndInputData( *m_inputData );
   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught in SlaveTerminate()"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
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

      for( std::vector< TTree* >::iterator tree = m_outputTrees.begin();
           tree != m_outputTrees.end(); ++tree ) {
         ( *tree )->AutoSave();
         delete ( *tree );
      }

      m_outputFile->Write();
      m_outputFile->Close();
      delete m_outputFile;
      m_outputFile = 0;
      m_outputTrees.clear();

   }

   m_logger << INFO << "Terminated InputData \"" << m_inputData->GetType()
            << "\" (Version:" << m_inputData->GetVersion()
            << ") on worker node" << SLogger::endmsg;

   return;

}

void SCycleBaseExec::Terminate() {

   m_logger << VERBOSE << "In SCycleBaseExec::Terminate()" << SLogger::endmsg;

   try {
      this->EndMasterInputData( *m_inputData );
   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught in Terminate()"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
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
      m_logger << FATAL << "Couldn't retrieve the cycle configuration"
               << SLogger::endmsg;
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
      m_logger << FATAL << "Couldn't retrieve the input data definition currently "
               << "being processed" << SLogger::endmsg;
      throw SError( "Couldn't find current input data configuration object", SError::SkipCycle );
      return;
   }

   return;

}
