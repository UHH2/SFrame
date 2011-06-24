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
#include <limits>

// ROOT include(s):
#include <TDOMParser.h>
#include <TXMLNode.h>
#include <TXMLDocument.h>
#include <TXMLAttr.h>
#include <TStopwatch.h>
#include <TSystem.h>
#include <TClass.h>
#include <TROOT.h>
#include <TPython.h>
#include <TChain.h>
#include <TList.h>
#include <TFile.h>
#include <TProof.h>
#include <TProofOutputFile.h>
#include <TDSet.h>
#include <TEnv.h>
#include <TFileMerger.h>

// Local include(s):
#include "../include/SCycleController.h"
#include "../include/ISCycleBase.h"
#include "../include/SLogWriter.h"
#include "../include/SConstants.h"
#include "../include/SParLocator.h"
#include "../include/SCycleStatistics.h"
#include "../include/SFileMerger.h"
#include "../include/SOutputFile.h"
#include "../include/SCycleConfig.h"
#include "../include/SCycleOutput.h"
#include "../include/SProofManager.h"

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
     m_proof( 0 ), m_logger( "SCycleController" ) {

}

/**
 * This destructor actually does something. (Yay!)
 * It deletes all the analysis cycles that have been created from the
 * configuration in the XML file, and closes the connection to the
 * PROOF server.
 */
SCycleController::~SCycleController() {

   std::vector< ISCycleBase* >::const_iterator it = m_analysisCycles.begin();
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
               if( ! cycleClass || ! cycleClass->InheritsFrom( "ISCycleBase" ) ) {
                  SError error( SError::SkipCycle );
                  error << "Loading of class \"" << cycleName << "\" failed";
                  throw error;
               }

               ISCycleBase* cycle = reinterpret_cast< ISCycleBase* >( cycleClass->New() );

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
               REPORT_VERBOSE( "Trying to load library \"" << libraryName << "\"" );

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
               REPORT_VERBOSE( "Trying to load python library \"" << libraryName
                               << "\"" );

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
               REPORT_ERROR( "Exception caught while processing node: "
                             << nodes->GetNodeName() );
               REPORT_ERROR( "Message: " << error.what() );
               REPORT_ERROR( "--> Skipping cycle!" );

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

   m_logger << INFO << "Entering ExecuteAllCycles()" << SLogger::endmsg;

   std::vector< ISCycleBase* >::const_iterator it = m_analysisCycles.begin();
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
   ISCycleBase* cycle     = m_analysisCycles.at( m_curCycle );
   TString      cycleName = cycle->GetName();

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

         REPORT_VERBOSE( "Uploading package: " << pkg );
         if( m_proof->UploadPackage( pkg ) ) {
            REPORT_ERROR( "There was a problem with uploading "
                          << *package );
            throw SError( *package + " could not be uploaded to PROOF",
                          SError::SkipCycle );
         }

         Ssiz_t slash_pos = pkg.Last( '/' );
         pkg.Remove( 0, slash_pos + 1 );
         if( pkg.EndsWith( ".par", TString::kIgnoreCase ) ) {
            pkg.Remove( pkg.Length() - 4, 4 );
         }

         m_logger << INFO << "Enabling package: " << pkg << SLogger::endmsg;
         if( m_proof->EnablePackage( pkg, kTRUE ) ) {
            REPORT_ERROR( "There was a problem with enabling "
                          << *package );
            throw SError( *package + " could not be enabled on PROOF",
                          SError::SkipCycle );
         }

      }

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
         REPORT_VERBOSE( "New output file will be opened for ID type: "
                         << id->GetType() );
      } else {
         --previous_id;
         if( ( previous_id->GetType() == id->GetType() ) &&
             ( previous_id->GetVersion() == id->GetVersion() ) ) {
            updateOutput = kTRUE;
            REPORT_VERBOSE( "Output file will be updated for ID type: "
                            << id->GetType() );
         } else {
            updateOutput = kFALSE;
            REPORT_VERBOSE( "New output file will be opened for ID type: "
                            << id->GetType() );
         }
      }

      //
      // Each input data has to have at least one input tree:
      //
      if( ! id->HasInputTrees() ) {
         REPORT_ERROR( "No input trees defined in input data " << id->GetType() );
         REPORT_ERROR( "Skipping it from processing" );
         continue;
      }

      // Find the first event-level input tree in the configuration:
      REPORT_VERBOSE( "Finding the name of the main event-level input TTree..." );
      const char* treeName = 0;
      for( std::map< Int_t, std::vector< STree > >::const_iterator trees =
              id->GetTrees().begin(); trees != id->GetTrees().end(); ++trees ) {
         for( std::vector< STree >::const_iterator st = trees->second.begin();
              st != trees->second.end(); ++st ) {
            if( ( st->type & STree::INPUT_TREE ) && ( st->type & STree::EVENT_TREE ) ) {
               treeName = st->treeName.Data();
               break;
            }
         }
      }
      if( ! treeName ) {
         REPORT_ERROR( "Can't determine input TTree name for input data "
                       << id->GetType() );
         REPORT_ERROR( "Skipping it from processing" );
         continue;
      } else {
         REPORT_VERBOSE( "The name of the main event-level input TTree is: "
                         << treeName );
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
      // Retrieve the configuration object list from the cycle:
      //
      const TList& configList = cycle->GetConfigurationObjects();

      //
      // Calculate how many events to process:
      //
      Long64_t evmax = ( id->GetNEventsMax() == -1 ? std::numeric_limits< Long64_t >::max() :
                         id->GetNEventsMax() );

      // This will point to the created output objects:
      TList* outputs = 0;

      //
      // The cycle can be run in two modes:
      //
      if( config.GetRunMode() == SCycleConfig::LOCAL ) {

         if( id->GetDataSets().size() ) {
            REPORT_ERROR( "Can't use DataSet-s as input in LOCAL mode!" );
            REPORT_ERROR( "Skipping InputData type: " << id->GetType()
                          << " version: " << id->GetVersion() );
            continue;
         }

         //
         // Create a chain with all the specified input files:
         //
         REPORT_VERBOSE( "Creating TChain to run the cycle on..." );
         TChain chain( treeName );
         for( std::vector< SFile >::const_iterator file = id->GetSFileIn().begin();
              file != id->GetSFileIn().end(); ++file ) {
            REPORT_VERBOSE( "Adding file: " << file->file );
            chain.AddFile( file->file );
         }

         //
         // Give the configuration to the cycle by hand:
         //
         TList list;
         list.Add( &config );
         list.Add( &inputData );
         for( Int_t i = 0; i < configList.GetSize(); ++i ) {
            list.Add( configList.At( i ) );
         }
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
            REPORT_ERROR( "PROOF server doesn't seem to be available: "
                          << m_proof->GetManager()->GetUrl() );
            REPORT_ERROR( "Aborting execution of cycle!" );
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
         //
         // This is just a temporary measure to avoid crashes with MC10b D3PDs:
         //
         m_proof->SetParameter( "PROOF_UseTreeCache", ( Int_t ) 0 );
         gEnv->SetValue( "Proof.StatsHist", 1 );
         m_proof->AddInput( &config );
         m_proof->AddInput( &inputData );
         m_proof->AddInput( &proofOutputFile );
         for( Int_t i = 0; i < configList.GetSize(); ++i ) {
            m_proof->AddInput( configList.At( i ) );
         }

         if( id->GetDataSets().size() ) {

            // Merge the dataset names in the way that PROOF expects them. This is
            // "<dataset 1>|<dataset 2>|...". Note that this only works in ROOT
            // versions newer than 5.27/02, but SInputData should take care about
            // removing multiple datasets when using an "old" ROOT release.
            TString dsets = "";
            for( std::vector< SDataSet >::const_iterator ds = id->GetDataSets().begin();
                 ds != id->GetDataSets().end(); ++ds ) {

               if( ds != id->GetDataSets().begin() ) {
                  dsets += "|";
               }
               dsets += ds->name + "#" + treeName;
            }

            // Process the events:
            if( m_proof->Process( dsets, cycle->GetName(), "", evmax,
                                  id->GetNEventsSkip() ) == -1 ) {
               REPORT_ERROR( "There was an error processing:" );
               REPORT_ERROR( "  Cycle      = " << cycle->GetName() );
               REPORT_ERROR( "  ID type    = " << inputData.GetType() );
               REPORT_ERROR( "  ID version = " << inputData.GetVersion() );
               REPORT_ERROR( "Stopping the execution of this cycle!" );
               break;
            }

         } else if( id->GetSFileIn().size() ) {

            //
            // Check if the validation was skipped. If it was, then the SInputData objects
            // didn't create a TDSet object of its own. So we have to create a simple one
            // here. Otherwise just use the TDSet created by SInputData.
            //
            if( id->GetSkipValid() ) {

               // Create the dataset object first:
               TChain chain( treeName );
               for( std::vector< SFile >::const_iterator file = id->GetSFileIn().begin();
                    file != id->GetSFileIn().end(); ++file ) {
                  chain.Add( file->file );
               }
               TDSet set( chain );

               // Process the events:
               if( m_proof->Process( &set, cycle->GetName(), "", evmax,
                                     id->GetNEventsSkip() ) == -1 ) {
                  REPORT_ERROR( "There was an error processing:" );
                  REPORT_ERROR( "  Cycle      = " << cycle->GetName() );
                  REPORT_ERROR( "  ID type    = " << inputData.GetType() );
                  REPORT_ERROR( "  ID version = " << inputData.GetVersion() );
                  REPORT_ERROR( "Stopping the execution of this cycle!" );
                  break;
               }

            } else {

               //
               // Run the cycle on PROOF. Unfortunately the checking of the "successfullness"
               // of the PROOF job is not working too well... Even after a *lot* of error
               // messages the TProof::Process(...) command can still return a success code,
               // which can lead to nasty crashes...
               //
               if( m_proof->Process( id->GetDSet(), cycle->GetName(), "", evmax,
                                     id->GetNEventsSkip() ) == -1 ) {
                  REPORT_ERROR( "There was an error processing:" );
                  REPORT_ERROR( "  Cycle      = " << cycle->GetName() );
                  REPORT_ERROR( "  ID type    = " << inputData.GetType() );
                  REPORT_ERROR( "  ID version = " << inputData.GetVersion() );
                  REPORT_ERROR( "Stopping the execution of this cycle!" );
                  break;
               }
            }

         } else {
            REPORT_ERROR( "Nothing was executed using PROOF!" );
         }

         outputs = m_proof->GetOutputList();

      } else {
         throw SError( "Running mode not recognised!", SError::SkipCycle );
      }

      if( ! outputs ) {
         REPORT_ERROR( "Cycle output could not be retrieved." );
         REPORT_ERROR( "NOT writing the output of cycle \""
                       << cycle->GetName() << "\", ID \"" << inputData.GetType()
                       << "\", Version \"" << inputData.GetVersion() << "\"" );
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

      // This cleanup is giving me endless trouble on the NYU Tier3 with
      // ROOT 5.28c. So, knowing no better solution, I just disabled it
      // on new ROOT versions for now...
#if ROOT_VERSION_CODE < ROOT_VERSION( 5, 28, 0 )
      outputs->SetOwner( kTRUE );
#endif
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
void SCycleController::AddAnalysisCycle( ISCycleBase* cycleAlg ) {

   m_analysisCycles.push_back( cycleAlg );
   return;
}

/**
 *  Delete all analysis cycle objects from memory!
 */
void SCycleController::DeleteAllAnalysisCycles() {

   m_logger << INFO << "Deleting all analysis cycle algorithms from memory"
            << SLogger::endmsg;

   std::vector< ISCycleBase* >::const_iterator it = m_analysisCycles.begin();
   for( ; it != m_analysisCycles.end(); ++it ) {
      delete ( *it );
   }

   m_analysisCycles.clear();

   return;
}

void SCycleController::InitProof( const TString& server, const Int_t& nodes ) {

   //
   // Open the connection:
   //
   m_logger << INFO << "Opening PROOF connection to: " << server
            << SLogger::endmsg;
   m_proof = SProofManager::Instance()->Open( server );
   if( nodes > 0 ) m_proof->SetParallel( nodes );

   return;
}

void SCycleController::ShutDownProof() {

   //
   // Clean up the PROOF connection(s):
   //
   SProofManager::Instance()->Cleanup();
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

      // Merge the files into the output file using TFileMerger:
      TFileMerger merger;
      for( std::vector< TString >::const_iterator mfile = filesToMerge.begin();
           mfile != filesToMerge.end(); ++mfile ) {
         merger.AddFile( *mfile );
      }
      if( ! merger.OutputFile( filename + ".tfm.temp" ) ) {
         REPORT_ERROR( "Couldn't open output file for merging: " << filename );
         return;
      }
      if( ! merger.Merge() ) {
         REPORT_ERROR( "There was an error merging the files" );
         return;
      }

      // Remove the temporary files:
      for( std::vector< TString >::const_iterator mfile = filesToMerge.begin();
           mfile != filesToMerge.end(); ++mfile ) {
         gSystem->Unlink( *mfile );
         // This is not too nice, but for LOCAL running we also have to remove
         // the temporary directory that the file was in:
         if( mfile->Contains( SFrame::ProofOutputFileName ) ) {
            TString dirname = gSystem->DirName( *mfile );
            if( dirname != "." ) {
               gSystem->Unlink( dirname );
            }
         }
      }

      // Now merge the resulting file into our histogram file using SFileMerger:
      SFileMerger merger2;
      merger2.AddInput( filename + ".tfm.temp" );
      merger2.SetOutput( filename );
      merger2.Merge();

      // Remove the file created by TFileMerger:
      gSystem->Unlink( filename + ".tfm.temp" );

   }

   return;
}
