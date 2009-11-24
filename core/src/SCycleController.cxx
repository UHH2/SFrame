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
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstdlib>

// ROOT include(s):
#include "TDOMParser.h"
#include "TXMLNode.h"
#include "TXMLDocument.h"
#include "TXMLAttr.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TClass.h"
#include "TROOT.h"
#include "TPython.h"
#include <TChain.h>
#include <TList.h>
#include <TFile.h>
#include <TProof.h>
#include <TProofLog.h>
#include <TProofOutputFile.h>
#include <TDSet.h>
#include <TMacro.h>
#include <TQueryResult.h>
#include <TEnv.h>

// Local include(s):
#include "../include/SCycleController.h"
#include "../include/SCycleBase.h"
#include "../include/SLogWriter.h"
#include "../include/SConstants.h"
#include "../include/SParLocator.h"
#include "../include/SCycleStatistics.h"
#include "../include/SFileMerger.h"
#include "../include/SOutputFile.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleController );
#endif // DOXYGEN_IGNORE

/**
 * The user has to specify a configuration file already at the construction
 * of the object. This configuration file will be used later in
 * SCycleController::Initialize to perform the configuration of the
 * analysis job.
 *
 * @param xmlConfigFile The name of the configuration file
 */
SCycleController::SCycleController( const TString& xmlConfigFile )
   : m_curCycle( 0 ), m_isInitialized( kFALSE ), m_xmlConfigFile( xmlConfigFile ),
     m_proof( 0 ), m_logger( this ) {

}

/**
 * This destructor actually does something. (Yay!)
 * It deletes all the analysis cycles that have been created from the
 * configuration in the XML file, and closes the connection to the
 * PROOF server.
 */
SCycleController::~SCycleController() {

   std::vector< SCycleBase* >::const_iterator it = m_analysisCycles.begin();
   for( ; it != m_analysisCycles.end(); ++it) {
      delete ( *it );
   }

   ShutDownProof();

}

/**
 * This is the first function that should be called for the object after
 * it is created. It reads in the configuration of the analysis from
 * the configuration file specified earlier, creates and configures
 * all the analysis cycles defined in the configuration.
 *
 * @callgraph
 */
void SCycleController::Initialize() throw( SError ) {

   m_logger << INFO << "Initializing" << SLogger::endmsg;

   // Just for kicks, lets measure the time it needs to initialise the
   // analysis:
   TStopwatch timer;
   timer.Start();

   // first clean up everything in case this is called multiple times
   m_curCycle = 0;
   this->DeleteAllAnalysisCycles();
   m_parPackages.clear();

   // --------------- xml read
   m_logger << INFO << "read xml file: '" << m_xmlConfigFile << "'" << SLogger::endmsg;

   TDOMParser xmlparser;

   // This is a new feature only available in the newest ROOT
   // nightlies. It makes it possible to have the input file
   // definitions in external XML files that are imported in
   // the main configuration XML file. It's conventient when
   // using a lot of the same input files in different cycles.
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 17, 4 )
   xmlparser.SetReplaceEntities( kTRUE );
#endif

   Int_t parseError = xmlparser.ParseFile( m_xmlConfigFile );
   if( parseError ) {
      SError error( SError::StopExecution );
      error << "Loading of xml document \"" << m_xmlConfigFile
            << "\" failed";
      throw error;
   }

   // --------------- interpret xml file

   // read cycles and libraries

   TXMLDocument* xmldoc = xmlparser.GetXMLDocument();
   TXMLNode* rootNode = xmldoc->GetRootNode();

   if( rootNode->GetNodeName() == TString( "JobConfiguration" ) ) {
      std::string jobName = "";
      std::string outputLevelString = "";
      TListIter attribIt( rootNode->GetAttributes() );
      TXMLAttr* curAttr( 0 );
      while ( (curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
         if( curAttr->GetName() == TString( "JobName" ) )
            jobName = curAttr->GetValue();
         else if( curAttr->GetName() == TString( "OutputLevel" ) )
            outputLevelString = curAttr->GetValue();
      }
      SMsgType type = DEBUG;
      if( outputLevelString == "VERBOSE" ) type = VERBOSE;
      else if( outputLevelString == "DEBUG" )   type = DEBUG;
      else if( outputLevelString == "INFO" )    type = INFO;
      else if( outputLevelString == "WARNING" ) type = WARNING;
      else if( outputLevelString == "ERROR" )   type = ERROR;
      else if( outputLevelString == "FATAL" )   type = FATAL;
      else if( outputLevelString == "ALWAYS" )  type = ALWAYS;
      SLogWriter::Instance()->SetMinType( type );

      TXMLNode* nodes = rootNode->GetChildren();

      // now loop over nodes
      while( nodes != 0 ) {
         if( ! nodes->HasAttributes() ) {
            nodes = nodes->GetNextNode();
            continue;
         }

         try { // For catching "cycle level" problems...

            if( nodes->GetNodeName() == TString( "Cycle" ) ) {

               std::string cycleName = "";

               attribIt = nodes->GetAttributes();
               curAttr = 0;
               while ( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
                  if( curAttr->GetName() == TString( "Name" ) )
                     cycleName = curAttr->GetValue();
               }

               TClass* cycleClass = gROOT->GetClass( cycleName.c_str(), true );
               if( ! cycleClass || ! cycleClass->InheritsFrom( "SCycleBase" ) ) {
                  SError error( SError::SkipCycle );
                  error << "Loading of class \"" << cycleName << "\" failed";
                  throw error;
               }

               SCycleBase* cycle = reinterpret_cast< SCycleBase* >( cycleClass->New() );

               m_logger << INFO << "Created cycle '" << cycleName << "'"
                        << SLogger::endmsg;

               cycle->Initialize( nodes );
               this->AddAnalysisCycle( cycle );

            } else if( nodes->GetNodeName() == TString( "Library" ) ) {

               std::string libraryName = "";
               attribIt = nodes->GetAttributes();
               curAttr = 0;
               while ( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
                  if( curAttr->GetName() == TString( "Name" ) )
                     libraryName = curAttr->GetValue();
               }
               m_logger << VERBOSE << "Trying to load library \"" << libraryName << "\""
                        << SLogger::endmsg;

               int ret = 0;
               if( ( ret = gSystem->Load( libraryName.c_str() ) ) >= 0 ) {
                  m_logger << DEBUG << "Library loaded: \"" << libraryName << "\"" 
                           << SLogger::endmsg;
               } else {
                  SError error( SError::StopExecution );
                  error << "Library failed to load: \"" << libraryName
                        << "\"\nRet. Val.: " << ret;
                  throw error;
               }

            } else if( nodes->GetNodeName() == TString( "PyLibrary" ) ) {

               std::string libraryName = "";
               attribIt = nodes->GetAttributes();
               curAttr = 0;
               while ( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
                  if( curAttr->GetName() == TString( "Name" ) )
                     libraryName = curAttr->GetValue();
               }
               m_logger << DEBUG << "Trying to load python library \"" << libraryName
                        << "\"" << SLogger::endmsg;

               std::ostringstream command;
               command << "import " << libraryName;

               TPython::Exec( command.str().c_str() );

            } else if( nodes->GetNodeName() == TString( "Package" ) ) {

               TString packageName;
               attribIt = nodes->GetAttributes();
               curAttr = 0;
               while ( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
                  if( curAttr->GetName() == TString( "Name" ) )
                     packageName = curAttr->GetValue();
               }
               m_logger << DEBUG << "Using PROOF ARchive package: " << packageName
                        << SLogger::endmsg;

               m_parPackages.push_back( packageName );

            }

         } catch( const SError& error ) {
            //
            // This is where I catch "cycle level" problems:
            //
            if( error.request() <= SError::SkipCycle ) {
               // If just this cycle has to be skipped:
               m_logger << ERROR << "Exception caught while processing node: "
                        << nodes->GetNodeName() << SLogger::endmsg;
               m_logger << ERROR << "Message: " << error.what() << SLogger::endmsg;
               m_logger << ERROR << "--> Skipping cycle!" << SLogger::endmsg;

               nodes = nodes->GetNextNode();
               continue;
            } else {
               // If this is more serious:
               throw;
            }
         }

         nodes = nodes->GetNextNode();

      } // end loop over nodes

      m_logger << INFO << "Job '" << jobName << "' configured" << SLogger::endmsg;

   } else {
      SError error( SError::StopExecution );
      error << "XML root node " << rootNode->GetNodeName()
            << " has wrong format";
      throw error;
   }

   // --------------- end of xml interpretation

   // Print how much time it took to initialise the analysis:
   timer.Stop();
   m_logger << INFO << "Time needed for initialisation: " << std::setw( 6 )
            << std::setprecision( 2 ) << timer.RealTime() << " s"
            << SLogger::endmsg;

   // Print memory consumption after initialising the analysis:
   ProcInfo_t procinfo;
   gSystem->GetProcInfo( &procinfo );
   m_logger << DEBUG << "Memory consumption after initialisation:" << SLogger::endmsg;
   m_logger.setf( std::ios::fixed );
   m_logger << DEBUG << "  Resident mem.: " << std::setw( 7 ) << procinfo.fMemResident
            << " kB; Virtual mem.: " << std::setw( 7 ) << procinfo.fMemVirtual
            << " kB" << SLogger::endmsg;

   // set object status to be ready
   m_isInitialized = kTRUE;

   return;
}

/**
 * This function runs all the analysis cycles that were configured
 * one after another. The main program should call this after
 * SCycleController::Initialize.
 *
 * It actually uses SCycleController::ExecuteNextCycle to execute each
 * of the cycles.
 *
 * @see SCycleController::ExecuteNextCycle
 * @callgraph
 */
void SCycleController::ExecuteAllCycles() throw( SError ) {

   if( ! m_isInitialized ) {
      throw SError( "SCycleController is not initialized",
                    SError::StopExecution );
   }

   m_logger << INFO << "entering ExecuteAllCycles()" << SLogger::endmsg;

   std::vector< SCycleBase* >::const_iterator it = m_analysisCycles.begin();
   for( ; it != m_analysisCycles.end(); ++it ) {
      this->ExecuteNextCycle();
   }

   return;
}

/**
 * This function is responsible for calling the functions of the current
 * cycle in line to execute its analysis. It also measures the time needed
 * to execute the cycles, and prints it as an info message after the
 * analysis has finished.
 *
 * @callgraph
 */
void SCycleController::ExecuteNextCycle() throw( SError ) {

   if( ! m_isInitialized ) {
      throw SError( "SCycleController is not initialized",
                    SError::StopExecution );
   }

   //
   // Measure the total time needed for this cycle:
   //
   TStopwatch timer;
   timer.Start();

   //
   // Access the current cycle:
   //
   SCycleBase* cycle     = m_analysisCycles.at( m_curCycle );
   TString     cycleName = cycle->GetName();

   //
   // Create a copy of the cycle configuration, so that it can be given to PROOF:
   //
   SCycleConfig config = cycle->GetConfig();
   config.SetName( SFrame::CycleConfigName );
   config.ArrangeInputData(); // To handle multiple ID of the same type...
   config.ValidateInput(); // This is needed for the proper weighting...
   config.SetMsgLevel( SLogWriter::Instance()->GetMinType() ); // For the correct msg level...
   cycle->SetConfig( config );

   m_logger << INFO << "Executing Cycle #" << m_curCycle << " ('" << cycleName << "') "
            << ( config.GetRunMode() == SCycleConfig::LOCAL ? "locally" :
                 "on PROOF" )
            << SLogger::endmsg;

   //
   // Make some initialisation steps before starting the cycle:
   //
   if( config.GetRunMode() == SCycleConfig::PROOF ) {

      //
      // Connect to the PROOF server:
      //
      InitProof( config.GetProofServer(), config.GetProofNodes() );

      //
      // Upload and compile all the packages specified in the configuration:
      //
      for( std::vector< TString >::const_iterator package = m_parPackages.begin();
           package != m_parPackages.end(); ++package ) {

         // Find the full path name of the package:
         TString pkg = SParLocator::Locate( *package );
         if( pkg == "" ) continue;

         m_logger << VERBOSE << "Uploading package: " << pkg << SLogger::endmsg;
         if( m_proof->UploadPackage( pkg ) ) {
            m_logger << ERROR << "There was a problem with uploading "
                     << *package << SLogger::endmsg;
            throw SError( *package + " could not be uploaded to PROOF",
                          SError::SkipCycle );
         }

         Ssiz_t slash_pos = pkg.Last( '/' );
         pkg.Remove( 0, slash_pos + 1 );
         if( pkg.EndsWith( ".par", TString::kIgnoreCase ) ) {
            pkg.Remove( pkg.Length() - 4, 4 );
         }

         m_logger << DEBUG << "Enabling package: " << pkg << SLogger::endmsg;
         if( m_proof->EnablePackage( pkg, kTRUE ) ) {
            m_logger << ERROR << "There was a problem with enabling "
                     << *package << SLogger::endmsg;
            throw SError( *package + " could not be enabled on PROOF",
                          SError::SkipCycle );
         }

      }

   } else {
      //
      // Shut down a possibly open connection:
      //
      ShutDownProof();
   }

   // Number of processed events:
   Long64_t procev = 0;
   // Number of skipped events:
   Long64_t skipev = 0;

   //
   // The begin cycle function has to be called here by hand:
   //
   cycle->BeginCycle();

   //
   // Loop over all defined input data types:
   //
   for( SCycleConfig::id_type::const_iterator id = config.GetInputData().begin();
        id != config.GetInputData().end(); ++id ) {

      //
      // Decide how to write the output file at the end of processing this InputData.
      // The InputData objects should be arranged by their type at this point...
      //
      Bool_t updateOutput = kFALSE;
      SCycleConfig::id_type::const_iterator previous_id = id;
      if( previous_id == config.GetInputData().begin() ) {
         updateOutput = kFALSE;
         m_logger << VERBOSE << "New output file will be opened for ID type: "
                  << id->GetType() << SLogger::endmsg;
      } else {
         --previous_id;
         if( ( previous_id->GetType() == id->GetType() ) &&
             ( previous_id->GetVersion() == id->GetVersion() ) ) {
            updateOutput = kTRUE;
            m_logger << VERBOSE << "Output file will be updated for ID type: "
                     << id->GetType() << SLogger::endmsg;
         } else {
            updateOutput = kFALSE;
            m_logger << VERBOSE << "New output file will be opened for ID type: "
                     << id->GetType() << SLogger::endmsg;
         }
      }

      //
      // Each input data has to have at least one input tree:
      //
      if( ! id->GetInputTrees().size() ) {
         m_logger << ERROR << "No input trees defined in input data " << id->GetType()
                  << SLogger::endmsg;
         m_logger << ERROR << "Skipping it from processing" << SLogger::endmsg;
         continue;
      }

      m_logger << INFO << "Processing input data type: " << id->GetType()
               << " version: " << id->GetVersion() << SLogger::endmsg;

      //
      // Create a copy of the input data configuration, so that it can be
      // given to PROOF:
      //
      SInputData inputData = *id;
      inputData.SetName( SFrame::CurrentInputDataName );

      //
      // Calculate how many events to process:
      //
      Long64_t evmax = ( id->GetNEventsMax() == -1 ? 100000000 :
                         id->GetNEventsMax() );

      // This will point to the created output objects:
      TList* outputs = 0;

      //
      // The cycle can be run in two modes:
      //
      if( config.GetRunMode() == SCycleConfig::LOCAL ) {

         //
         // Create a chain with all the specified input files:
         //
         TChain chain( id->GetInputTrees().at( 0 ).treeName );
         for( std::vector< SFile >::const_iterator file = id->GetSFileIn().begin();
              file != id->GetSFileIn().end(); ++file ) {
            chain.Add( file->file );
         }

         //
         // Give the configuration to the cycle by hand:
         //
         TList list;
         list.Add( &config );
         list.Add( &inputData );
         cycle->SetInputList( &list );

         //
         // Run the cycle:
         //
         chain.Process( cycle, "", evmax, id->GetNEventsSkip() );
         outputs = cycle->GetOutputList();

      } else if( config.GetRunMode() == SCycleConfig::PROOF ) {

         //
         // Check that the PROOF server is available and ready. For instance it's not
         // a good idea to send a job to a server that crashed on the previous
         // input data...
         //
         if( ! m_proof->IsValid() ) {
            m_logger << ERROR << "PROOF server doesn't seem to be available: "
                     << m_proof->GetManager()->GetUrl() << SLogger::endmsg;
            m_logger << ERROR << "Aborting execution of cycle!" << SLogger::endmsg;
            break;
         }

         // This object describes how to create the temporary PROOF output
         // files in the cycles:
         TNamed proofOutputFile( TString( SFrame::ProofOutputName ),
                                 ( config.GetProofWorkDir() == "" ? "./" :
                                   config.GetProofWorkDir() + "/" ) +
                                 cycle->GetName() + "-" + inputData.GetType() +
                                 "-" + inputData.GetVersion() + "-TempNTuple.root" );

         //
         // Clear the query results from memory (Thanks to Gerri!):
         //
         if( m_proof->GetQueryResults() ) {
            m_proof->GetQueryResults()->SetOwner( kTRUE );
            m_proof->GetQueryResults()->Clear();
            m_proof->GetQueryResults()->SetOwner( kFALSE );
         }

         //
         // Give the configuration to PROOF, and tweak it a little:
         //
         m_proof->ClearInput();
         // Only output a maximum of 10 messages per node about memory usage per query:
         Long64_t eventsPerNode = inputData.GetEventsTotal() / m_proof->GetParallel();
         m_proof->SetParameter( "PROOF_MemLogFreq",
                                ( Long64_t ) ( eventsPerNode > 10000 ?
                                               ( eventsPerNode / 10 ) :
                                               1000 ) );
         m_proof->SetParameter( "PROOF_MaxSlavesPerNode", ( Long_t ) 9999999 );
         gEnv->SetValue( "Proof.StatsHist", 1 );
         m_proof->AddInput( &config );
         m_proof->AddInput( &inputData );
         m_proof->AddInput( &proofOutputFile );

         //
         // Run the cycle on PROOF. Unfortunately the checking of the "successfullness"
         // of the PROOF job is not working too well... Even after a *lot* of error
         // messages the TProof::Process(...) command can still return a success code,
         // which can lead to nasty crashes...
         //
         if( m_proof->Process( id->GetDSet(), cycle->GetName(), "", evmax,
                               id->GetNEventsSkip() ) == -1 ) {
            m_logger << ERROR << "There was an error processing:" << SLogger::endmsg;
            m_logger << ERROR << "  Cycle      = " << cycle->GetName() << SLogger::endmsg;
            m_logger << ERROR << "  ID type    = " << inputData.GetType()
                     << SLogger::endmsg;
            m_logger << ERROR << "  ID version = " << inputData.GetVersion()
                     << SLogger::endmsg;
            m_logger << ERROR << "Stopping the execution of this cycle!" << SLogger::endmsg;
            break;
         }
         outputs = m_proof->GetOutputList();

      } else {
         throw SError( "Running mode not recognised!", SError::SkipCycle );
      }

      if( ! outputs ) {
         m_logger << ERROR << "Cycle output could not be retrieved." << SLogger::endmsg;
         m_logger << ERROR << "NOT writing the output of cycle \""
                  << cycle->GetName() << "\", ID \"" << inputData.GetType()
                  << "\", Version \"" << inputData.GetVersion() << "\""
                  << SLogger::endmsg;
         continue;
      }

      //
      // Collect the statistics from this input data:
      //
      SCycleStatistics* stat =
         dynamic_cast< SCycleStatistics* >( outputs->FindObject( SFrame::RunStatisticsName ) );
      if( stat ) {
         procev += stat->GetProcessedEvents();
         skipev += stat->GetSkippedEvents();
      } else {
         m_logger << WARNING << "Cycle statistics not received from: "
                  << cycle->GetName() << SLogger::endmsg;
         m_logger << WARNING << "Printed statistics will not be correct!"
                  << SLogger::endmsg;
      }

      //
      // Write out the objects produced by the cycle:
      //
      TString outputFileName = config.GetOutputDirectory() + cycleName + "." +
         id->GetType() + "." + id->GetVersion() + config.GetPostFix() + ".root";
      outputFileName.ReplaceAll( "::", "." );
      WriteCycleOutput( outputs, outputFileName, updateOutput );
      outputs->SetOwner( kTRUE );
      outputs->Clear();

   }

   //
   // The end cycle function has to be called here by hand:
   //
   cycle->EndCycle();

   // The cycle processing is done at this point:
   timer.Stop();

   m_logger << INFO << "Overall cycle statistics:" << SLogger::endmsg;
   m_logger.setf( std::ios::fixed );
   m_logger << INFO << std::setw( 10 ) << std::setfill( ' ' ) << std::setprecision( 0 )
            << procev << " Events - Real time " << std::setw( 6 ) << std::setprecision( 2 )
            << timer.RealTime() << " s  - " << std::setw( 5 )
            << std::setprecision( 0 ) << ( procev / timer.RealTime() ) << " Hz | CPU time "
            << std::setw( 6 ) << std::setprecision( 2 ) << timer.CpuTime() << " s  - "
            << std::setw( 5 ) << std::setprecision( 0 ) << ( procev / timer.CpuTime() )
            << " Hz" << SLogger::endmsg;

   ++m_curCycle;
   return;
}

/**
 * This function could be used to add a cycle created in the main executable
 * by hand, but it's not being used. Instead all the cycles are created
 * by this class internally according to the configuration in the
 * XML file.
 *
 * @param cycleAlg The cycle that should be added
 */
void SCycleController::AddAnalysisCycle( SCycleBase* cycleAlg ) {

   m_analysisCycles.push_back( cycleAlg );
   return;

}

/**
 *  Delete all analysis cycle objects from memory!
 */
void SCycleController::DeleteAllAnalysisCycles() {

   m_logger << INFO << "Deleting all analysis cycle algorithms from memory"
            << SLogger::endmsg;

   std::vector< SCycleBase* >::const_iterator it = m_analysisCycles.begin();
   for( ; it != m_analysisCycles.end(); ++it ) {
      delete ( *it );
   }

   m_analysisCycles.clear();

   return;
}

void SCycleController::InitProof( const TString& server, const Int_t& nodes ) {

   //
   // Check if the connection has to be (re)opened:
   //
   if( m_proof ) {
      // Unfortunately this check can not be true...
      if( m_proof->GetManager()->GetUrl() == server ) return;
      ShutDownProof();
   }

   //
   // Open the connection:
   //
   m_logger << INFO << "Opening PROOF connection to: " << server
            << SLogger::endmsg;
   m_proof = TProof::Open( server );
   if( nodes > 0 ) m_proof->SetParallel( nodes );

   return;

}

void SCycleController::ShutDownProof() {

   //
   // Check if there is a connection:
   //
   if( ! m_proof ) return;

   //
   // Print the worker logs only in DEBUG or VERBOSE mode. Normally we're not interested
   // in the event processing messages...
   //
   m_logger << INFO << "***************************************************************"
            << SLogger::endmsg;
   m_logger << INFO << "*                                                             *"
            << SLogger::endmsg;
   m_logger << INFO << "* Printing all worker logs before closing PROOF connection... *"
            << SLogger::endmsg;
   m_logger << INFO << "*                                                             *"
            << SLogger::endmsg;
   m_logger << INFO << "***************************************************************"
            << SLogger::endmsg;
   PrintWorkerLogs();

   //
   // Close the connection by deleting the objects in a specific order:
   //
   m_logger << DEBUG << "Closing PROOF connection to: "
            << m_proof->GetManager()->GetUrl() << SLogger::endmsg;
   TProofMgr* mgr = m_proof->GetManager();
   delete m_proof;
   delete mgr;

   m_proof = 0;

   return;

}

void SCycleController::WriteCycleOutput( TList* olist,
                                         const TString& filename, Bool_t update ) const {

   m_logger << INFO << "Writing output of \""
            << m_analysisCycles.at( m_curCycle )->GetName() << "\" to: "
            << filename << SLogger::endmsg;

   //
   // Open the output file:
   //
   TFile outputFile( filename , ( update ? "UPDATE" : "RECREATE" ) );

   //
   // List of files holding TTrees:
   //
   std::vector< TString > filesToMerge;

   //
   // Merge the memory objects into the output file:
   //
   for( Int_t i = 0; i < olist->GetSize(); ++i ) {

      outputFile.cd();

      if( dynamic_cast< SCycleOutput* >( olist->At( i ) ) ) {
         olist->At( i )->Write();
      } else if( dynamic_cast< TProofOutputFile* >( olist->At( i ) ) ) {
         TProofOutputFile* pfile = dynamic_cast< TProofOutputFile* >( olist->At( i ) );
         filesToMerge.push_back( pfile->GetOutputFileName() );
      } else if ( dynamic_cast< SOutputFile* >( olist->At( i ) ) ) {
         SOutputFile* sfile = dynamic_cast< SOutputFile* >( olist->At( i ) );
         filesToMerge.push_back( sfile->GetFileName() );
      } else {
         /*
         TDirectory* proofdir = outputFile.GetDirectory( "PROOF" );
         if( ! proofdir ) {
            proofdir = outputFile.mkdir( "PROOF", "PROOF related objects" );
         }
         proofdir->cd();
         olist->At( i )->Write();
         */
      }
      m_logger << DEBUG << "Written object: " << olist->At( i )->GetName()
               << SLogger::endmsg;

   }

   //
   // Write and close the output file:
   //
   outputFile.Write();
   outputFile.Close();

   //
   // Merge the TTree contents of the temporary files into our output file:
   //
   if( filesToMerge.size() ) {

      m_logger << DEBUG << "Merging disk-resident TTrees into \""
               << filename << "\"" << SLogger::endmsg;

      // Merge the files into the output file using SFileMerger:
      SFileMerger merger;
      for( std::vector< TString >::const_iterator mfile = filesToMerge.begin();
           mfile != filesToMerge.end(); ++mfile ) {
         merger.AddInput( *mfile );
      }
      merger.SetOutput( filename );
      merger.Merge();

      // Remove the temporary files:
      for( std::vector< TString >::const_iterator mfile = filesToMerge.begin();
           mfile != filesToMerge.end(); ++mfile ) {
         gSystem->Unlink( *mfile );
      }

   }

   return;

}

/**
 * This internal function collects the log files from all the nodes (the master
 * and the slaves) and prints them to the screen. Currently the nodes print waaay
 * too much information. Hopefully 5.22 will remove most of these...
 */
void SCycleController::PrintWorkerLogs() const {

   //
   // Make sure that a connection is open:
   //
   if( ! m_proof ) {
      m_logger << ERROR << "Not in PROOF mode --> Can't call PrintWorkerLogs()!"
               << SLogger::endmsg;
      return;
   }

   //
   // Get info about the slaves:
   //
   TList* slaveInfos = m_proof->GetListOfSlaveInfos();

   //
   // Retrieve all logs:
   //
   TProofLog* log = m_proof->GetManager()->GetSessionLogs();
   TList* logList = log->GetListOfLogs();
   for( Int_t i = 0; i < logList->GetSize(); ++i ) {

      //
      // Access the log of a single node:
      //
      TProofLogElem* element = dynamic_cast< TProofLogElem* >( logList->At( i ) );
      if( ! element ) {
         m_logger << ERROR << "Log element not recognised!" << SLogger::endmsg;
         continue;
      }

      //
      // Find "the name" of the node. TProofLogElem objects only know that they
      // came from node "0.2" for instance. This small loop matches these
      // identifiers to the proper node names in the slaveInfos list.
      //
      // If the identifier is not found in the list, then it has to be the master:
      TString nodeName = m_proof->GetMaster();
      for( Int_t i = 0; i < slaveInfos->GetSize(); ++i ) {

         // Access the TSlaveInfo object:
         TSlaveInfo* info = dynamic_cast< TSlaveInfo* >( slaveInfos->At( i ) );
         if( ! info ) {
            m_logger << ERROR << "Couldn't use a TSlaveInfo object!" << SLogger::endmsg;
            continue;
         }
         // Check if this TSlaveInfo describes the source of the log:
         if( ! strcmp( element->GetName(), info->GetOrdinal() ) ) {
            nodeName = info->GetName();
            break;
         }
      }

      //
      // Print the log. Note that we don't need to redirect the log lines
      // to m_logger. The log lines of the nodes will already be formatted, so
      // printing them through SLogger would just look ugly.
      //
      m_logger << INFO << "=================================================="
               << SLogger::endmsg;
      m_logger << INFO << "Output from node: " << nodeName << " ("
               << element->GetName() << ")" << SLogger::endmsg;

      element->GetMacro()->Print();

      m_logger << INFO << "=================================================="
               << SLogger::endmsg;

   }

   delete log;

   return;

}
