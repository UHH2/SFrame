// $Id: SCycleController.cxx,v 1.6.2.4 2008-12-04 17:02:19 krasznaa Exp $
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

// ROOT include(s):
#include "TDOMParser.h"
#include "TXMLNode.h"
#include "TXMLDocument.h"
#include "TXMLAttr.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TClass.h"
#include "TList.h"
#include "TROOT.h"
#include "TPython.h"
#include <TChain.h>
#include <TList.h>
#include <TFile.h>
#include <TProof.h>
#include <TProofLog.h>
#include <TDSet.h>
#include <TMacro.h>
#include <TQueryResult.h>
#include <TEnv.h>

// Local include(s):
#include "../include/SCycleController.h"
#include "../include/SCycleBase.h"
#include "../include/SLogWriter.h"

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

   if( m_proof ) {
      TProofMgr* mgr = m_proof->GetManager();
      delete m_proof;
      delete mgr;
   }

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

   TStopwatch timer;
   timer.Start();

   // Retrieve memory consumption before executing the analysis:
   ProcInfo_t mem_before;
   gSystem->GetProcInfo( &mem_before );

   SCycleBase* cycle     = m_analysisCycles.at( m_curCycle );
   std::string cycleName = cycle->GetName();

   SCycleConfig config = cycle->GetConfig();
   config.SetName( "CycleConfig" );

   m_logger << INFO << "Executing Cycle #" << m_curCycle << " ('" << cycleName << "') "
            << ( config.GetRunMode() == SCycleConfig::LOCAL ? "locally" :
                 "on PROOF" )
            << SLogger::endmsg;

   if( config.GetRunMode() == SCycleConfig::PROOF ) {
      //
      // Connect to the PROOF server:
      //
      InitProof( config.GetProofServer() );

      //
      // Upload and compile all the packages specified in the configuration:
      //
      for( std::vector< TString >::const_iterator package = m_parPackages.begin();
           package != m_parPackages.end(); ++package ) {
         TString pkg = *package;
         m_logger << VERBOSE << "Uploading package: " << pkg << SLogger::endmsg;
         m_proof->UploadPackage( pkg );

         Ssiz_t slash_pos = pkg.Last( '/' );
         pkg.Remove( 0, slash_pos + 1 );
         if( pkg.EndsWith( ".par", TString::kIgnoreCase ) ) {
            pkg.Remove( pkg.Length() - 4, 4 );
         }

         m_logger << DEBUG << "Enabling package: " << pkg << SLogger::endmsg;
         m_proof->EnablePackage( pkg, kTRUE );

      }
   } else {
      //
      // Shut down a possibly open connection:
      //
      ShutDownProof();
   }

   // Number of processed events:
   Long64_t nev = 0;

   for( SCycleConfig::id_type::const_iterator id = config.GetInputData().begin();
        id != config.GetInputData().end(); ++id ) {

      if( ! id->GetInputTrees().size() ) {
         throw SError( "No input trees defined", SError::SkipCycle );
      }

      m_logger << INFO << "Processing input data: " << id->GetType()
               << SLogger::endmsg;

      SInputData inputData = *id;
      inputData.SetName( "CurrentInputData" );

      TChain chain( id->GetInputTrees().at( 0 ).treeName );
      for( std::vector< SFile >::const_iterator file = id->GetSFileIn().begin();
           file != id->GetSFileIn().end(); ++file ) {
         chain.Add( file->file );
      }

      Long64_t evmax = ( id->GetNEventsMax() == -1 ? 100000000 :
                         id->GetNEventsMax() );
      TList* outputs = 0;

      if( config.GetRunMode() == SCycleConfig::LOCAL ) {

         TList list;
         list.Add( &config );
         list.Add( &inputData );
         cycle->SetInputList( &list );

         chain.Process( cycle, "", evmax );
         outputs = cycle->GetOutputList();

      } else if( config.GetRunMode() == SCycleConfig::PROOF ) {

         m_proof->ClearInput();
         m_proof->SetParameter( "PROOF_MemLogFreq", ( Long64_t ) 1000 );
         gEnv->SetValue( "Proof.StatsHist", 1 );
         m_proof->AddInput( &config );
         m_proof->AddInput( &inputData );

         TDSet set( chain );
         m_proof->Process( &set, cycle->GetName(), "", evmax );
         nev += m_proof->GetQueryResults()->GetEntries();
         outputs = m_proof->GetOutputList();

         PrintWorkerLogs();

      } else {
         throw SError( "Running mode not recognised!", SError::SkipCycle );
      }

      WriteCycleOutput( outputs, TString( cycleName.c_str() ) + "." +
                        id->GetType() + ".root" );
      outputs->Clear();

   }

   // Retrieve memory consumption after executing the analysis:
   ProcInfo_t mem_after;
   gSystem->GetProcInfo( &mem_after );

   // If we ran locally then this has to be extracted here:
   if( config.GetRunMode() != SCycleConfig::PROOF ) {
      nev = cycle->NumberOfProcessedEvents();
   }

   timer.Stop();

   m_logger << INFO << "Overall cycle statistics:" << SLogger::endmsg;
   m_logger.setf( std::ios::fixed );
   m_logger << INFO << std::setw( 10 ) << std::setfill( ' ' ) << std::setprecision( 0 )
            << nev << " Events - Real time " << std::setw( 6 ) << std::setprecision( 2 )
            << timer.RealTime() << " s  - " << std::setw( 5 )
            << std::setprecision( 0 ) << ( nev / timer.RealTime() ) << " Hz | CPU time "
            << std::setw( 6 ) << std::setprecision( 2 ) << timer.CpuTime() << " s  - "
            << std::setw( 5 ) << std::setprecision( 0 ) << ( nev / timer.CpuTime() )
            << " Hz" << SLogger::endmsg;
   m_logger << DEBUG << "Memory growth while executing cycle #"
            << m_curCycle << ":" << SLogger::endmsg;
   m_logger << DEBUG << "   Resident mem.: " << std::setw( 6 )
            << ( mem_after.fMemResident - mem_before.fMemResident )
            << " kB; " << std::setw( 7 )
            << ( ( mem_after.fMemResident - mem_before.fMemResident ) / nev )
            << " kB / event" << SLogger::endmsg;
   m_logger << DEBUG << "   Virtual mem. : " << std::setw( 6 )
            << ( mem_after.fMemVirtual - mem_before.fMemVirtual )
            << " kB; " << std::setw( 7 )
            << ( ( mem_after.fMemVirtual - mem_before.fMemVirtual ) / nev )
            << " kB / event" << SLogger::endmsg;

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

void SCycleController::InitProof( const TString& server ) {

   if( m_proof ) {
      if( m_proof->GetManager()->GetUrl() == server ) return;
      ShutDownProof();
   }

   m_logger << INFO << "Opening PROOF connection to: " << server
            << SLogger::endmsg;

   m_proof = TProof::Open( server );

   return;

}

void SCycleController::ShutDownProof() {

   if( ! m_proof ) return;

   m_logger << DEBUG << "Closing PROOF connection to: "
            << m_proof->GetManager()->GetUrl() << SLogger::endmsg;

   TProofMgr* mgr = m_proof->GetManager();
   delete m_proof;
   delete mgr;

   m_proof = 0;

   return;

}

void SCycleController::WriteCycleOutput( TList* olist,
                                         const TString& filename ) const {

   m_logger << INFO << "Writing output of \""
            << m_analysisCycles.at( m_curCycle )->GetName() << "\" to: "
            << filename << SLogger::endmsg;

   //
   // Open the output file:
   //
   TFile outputFile( filename, "RECREATE" );

   //
   // Write out each output object:
   //
   for( Int_t i = 0; i < olist->GetSize(); ++i ) {

      outputFile.cd();
      // Anything that's not wrapped in an SCycleOutput object, is
      // produced internally by PROOF, so it's put into a directory
      // called "PROOF":
      if( dynamic_cast< SCycleOutput* >( olist->At( i ) ) ) {
         olist->At( i )->Write();
      } else {
         TDirectory* proofdir = outputFile.GetDirectory( "PROOF" );
         if( ! proofdir ) {
            proofdir = outputFile.mkdir( "PROOF", "PROOF related objects" );
         }
         proofdir->cd();
         olist->At( i )->Write();
      }
      m_logger << DEBUG << "Written object: " << olist->At( i )->GetName()
               << SLogger::endmsg;

   }

   outputFile.Write();
   outputFile.Close();

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
