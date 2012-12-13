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
#include <TProof.h>
#include <TProofMgr.h>
#include <TProofLog.h>
#include <TMacro.h>
#include <TList.h>
#include <TObjArray.h>
#include <TObjString.h>

// Local include(s):
#include "../include/SProofManager.h"

// Initialize the static variable:
SProofManager* SProofManager::m_instance = 0;

/**
 * The destructor cleans up the open connections by calling the Cleanup()
 * function internally.
 */
SProofManager::~SProofManager() {

   Cleanup();
}

/**
 * This function should be used to access the single existing object.
 *
 * @returns The single SProofManager object
 */
SProofManager* SProofManager::Instance() {

   if( ! m_instance ) {
      m_instance = new SProofManager();
   }

   return m_instance;
}

/**
 * This function can be used basically in the same way as one would use
 * <code>TProof::Open(...)</code>.
 *
 * The function first looks at its internal cache to see if the connection
 * is already open, and if it is, it just returns the cached TProof object.
 * If the connection is not yet available, it creates it using
 * <code>TProof::Open(...)</code>, and adds the created object to its internal
 * cache.
 *
 * It is possible to trigger special actions/configurations by placing special
 * strings into the URL field. The extra tokens always have to be given with a
 * semicolon separator after the PROOF server name. The following options are
 * recognised:
 *
 *  - "<server>;MemProfMaster": Profile the memory usage of the PROOF master
 *    process using Valgrind
 *  - "<server>;MemProfWorkers": Profile the memory usage of the PROOF worker
 *    processes using Valgrind.
 *
 * @param url   Name of the PROOF server
 * @param param Additional parameters given to TProof::Open(...)
 * @returns The created TProof object
 */
TProof* SProofManager::Open( const TString& url,
                             const TString& param ) throw( SError ) {

   // Copy the contents of the parameters, as we may have to change them:
   TString urlcopy( url );
   TString paramcopy( param );

   // The user may specify some special additional tags in the URL string,
   // which have to be searched for here:
   TObjArray* tokens = url.Tokenize( ";" );
   if( tokens->GetEntries() > 1 ) {
      TObjString* token = dynamic_cast< TObjString* >( tokens->At( 0 ) );
      if( ! token ) {
         REPORT_ERROR( "The tokenized array contains something that's not a "
                       "TObjString!" );
         delete tokens;
         throw SError( "Problems with tokenizing PROOF URL",
                       SError::SkipCycle );
      }
      urlcopy = token->GetString();
   }
   // We override the parameters given to the function if extra tokens are
   // found. Then again, the SFrame code never gives anything as "param" to the
   // function anyway. (May change at one point.)
   if( ( tokens->GetEntries() > 1 ) && ( paramcopy != "" ) ) {
      m_logger << WARNING << "Extra parameters provided both in the URL and "
               << "the extra parameters field. Using the one(s) from the "
               << "URL field." << SLogger::endmsg;
      paramcopy = "";
   }
   for( Int_t i = 1; i < tokens->GetEntries(); ++i ) {
      TObjString* tokenobj = dynamic_cast< TObjString* >( tokens->At( i ) );
      if( ! tokenobj ) {
         REPORT_ERROR( "The tokenized array contains something that's not a "
                       "TObjString!" );
         delete tokens;
         throw SError( "Problems with tokenizing PROOF URL",
                       SError::SkipCycle );
      }
      const TString token = tokenobj->GetString();
      // Did the user ask for the memory-leak profiling of the PROOF master?
      if( token == "MemProfMaster" ) {
         m_logger << INFO << "Running memory profiling on the master node"
                  << SLogger::endmsg;
         paramcopy = "valgrind=master";
         TProof::AddEnvVar( "PROOF_MASTER_WRAPPERCMD",
                            "valgrind_opts:--leak-check=full "
                            "--track-origins=yes --num-callers=32" );
      }
      // Did the user ask for the memory-leak profiling of the PROOF workers?
      else if( token == "MemProfWorkers" ) {
         m_logger << INFO << "Running memory profiling on the worker nodes"
                  << SLogger::endmsg;
         paramcopy = "valgrind=workers";
         TProof::AddEnvVar( "PROOF_SLAVE_WRAPPERCMD",
                            "valgrind_opts:--leak-check=full "
                            "--track-origins=yes --num-callers=32" );
      } else {
         REPORT_ERROR( "Unknown extra parameter specified: " << token );
      }
      // Extend the memory available to the PROOF processes in all memory
      // profiling jobs:
      if( token.BeginsWith( "MemProf" ) ) {
         // This should make sure that at least 10 GBs are available to the
         // process:
         TProof::AddEnvVar( "PROOF_RESMEMMAX",  "10000" );
         TProof::AddEnvVar( "PROOF_VIRTMEMMAX", "10000" );
      }
   }
   REPORT_VERBOSE( "Using URL: " << urlcopy << ", Param: " << paramcopy );
   // Clean up:
   delete tokens;

   // Check if the connection has already been opened. Notice that we're
   // using the original URL and parameters here.
   const ConnMap_t::key_type connection = std::make_pair( url, param );
   ConnMap_t::const_iterator conn;
   if( ( conn = m_connections.find( connection ) ) != m_connections.end() ) {
      m_logger << DEBUG << "Connection to \"" << url << "\" is already open"
               << SLogger::endmsg;
      return conn->second.first;
   }

   // Try to open the connection:
   TProof* server = TProof::Open( urlcopy, paramcopy );
   if( ! server ) {
      REPORT_ERROR( "Couldn't open connection to: " << url );
      throw SError( "Couldn't open connection to: " + url,
                    SError::SkipCycle );
   } else {
      m_logger << INFO << "Connection opened to \"" << url << "\""
               << SLogger::endmsg;
   }

   // Remember that the server is connected, but not initialized yet:
   m_connections[ connection ] = std::make_pair( server, kFALSE );

   return server;
}

/**
 * This function is used to decide if a given PROOF server has already been
 * configured (packages uploaded, compiled and loaded). This helps avoid having
 * to wait while packages load multiple times in a job that runs many separately
 * configured cycles.
 *
 * @param url   Name of the PROOF server
 * @param param Additional parameters given to TProof::Open(...)
 * @returns <code>kTRUE</code> if the server is already configured, or
 *          <code>kFALSE</code> if it's not
 */
Bool_t SProofManager::IsConfigured( const TString& url,
                                    const TString& param ) const {
   
   // Check if the connection has already been opened:
   ConnMap_t::key_type connection = std::make_pair( url, param );
   ConnMap_t::const_iterator conn = m_connections.find( connection );
   if( conn != m_connections.end() ) {
      // Return the configuration state:
      return conn->second.second;
   }
   
   // If the server is not even connected, then it is definitely not configured:
   REPORT_ERROR( "Asking about a server that's not yet connected (\""
                 << url << "\", \"" << param << "\")" );
   return kFALSE;
}

/**
 * This function can be used to set the "configured state" of a PROOF server
 * connection. "Configured" means that all necessary PAR packages have already
 * been uploaded to, compiled and loaded on the server.
 *
 * @param url   Name of the PROOF server
 * @param param Additional parameters given to TProof::Open(...)
 * @param state Configuration state (<code>kTRUE</code> for configured,
 *              <code>kFALSE</code> for not configured.)
 */
void SProofManager::SetConfigured( const TString& url,
                                   const TString& param,
                                   Bool_t state ) throw( SError ) {

   // Make sure the connection is open. This call can throw an error
   // if unsuccessful, so no point in checking its return value.
   Open( url, param );

   // Now find it in our internal cache:
   ConnMap_t::key_type connection = std::make_pair( url, param );
   ConnMap_t::iterator conn = m_connections.find( connection );
   if( conn == m_connections.end() ) {
      REPORT_FATAL( "Internal logic error discovered" );
      throw SError( "Internal logic error discovered",
                    SError::StopExecution );
   }

   // Update the state:
   conn->second.second = state;

   return;
}

/**
 * This function can be used to clean up the PROOF connections. Even if it's
 * only called at the termination of the sframe_main program, it's still very
 * much needed. PROOF has the habit of producing a crash at the very last moment
 * in the application if the user doesn't delete its objects properly.
 *
 * The function first retrieves and prints all the worker logs from all the
 * connections, then it does the cleanup.
 */
void SProofManager::Cleanup() {

   PrintWorkerLogs();

   if( m_connections.size() ) {
      TProofMgr* mgr = m_connections.begin()->second.first->GetManager();
      for( ConnMap_t::iterator server = m_connections.begin();
           server != m_connections.end(); ++server ) {
         delete server->second.first;
      }
      delete mgr;
   }
   m_connections.clear();

   return;
}

/**
 * The constructor just initializes the member variables, and doesn't do
 * anything in addition.
 */
SProofManager::SProofManager()
   : m_connections(), m_logger( "SProofManager" ) {

}

/**
 * Function that retrieves and prints the log messages from all the worker
 * nodes in all the open PROOF connections. It tries to print the log messages
 * in a nice way, identifying the originator of each log.
 */
void SProofManager::PrintWorkerLogs() const {

   //
   // Loop over all the connections:
   //
   for( ConnMap_t::const_iterator server = m_connections.begin();
        server != m_connections.end(); ++server ) {

      //
      // Message identifying the server:
      //
      m_logger << INFO
               << "***************************************************************"
               << SLogger::endmsg;
      m_logger << INFO << "*" << SLogger::endmsg;
      m_logger << INFO << "* Printing all worker logs from server:"
               << SLogger::endmsg;
      m_logger << INFO << "*     " << server->first.first << SLogger::endmsg;
      m_logger << INFO << "*" << SLogger::endmsg;
      m_logger << INFO
               << "***************************************************************"
               << SLogger::endmsg;

      //
      // Get info about the slaves:
      //
      TList* slaveInfos = server->second.first->GetListOfSlaveInfos();

      //
      // Retrieve all logs:
      //
      TProofLog* log = server->second.first->GetManager()->GetSessionLogs();
      TList* logList = log->GetListOfLogs();
      for( Int_t i = 0; i < logList->GetSize(); ++i ) {

         //
         // Access the log of a single node:
         //
         TProofLogElem* element =
            dynamic_cast< TProofLogElem* >( logList->At( i ) );
         if( ! element ) {
            REPORT_ERROR( "Log element not recognised!" );
            continue;
         }

         //
         // Find "the name" of the node. TProofLogElem objects only know that
         // they came from node "0.2" for instance. This small loop matches
         // these identifiers to the proper node names in the slaveInfos list.
         //
         // If the identifier is not found in the list, then it has to be the
         // master...
         //
         TString nodeName = server->second.first->GetMaster();
         for( Int_t i = 0; i < slaveInfos->GetSize(); ++i ) {

            // Access the TSlaveInfo object:
            TSlaveInfo* info =
               dynamic_cast< TSlaveInfo* >( slaveInfos->At( i ) );
            if( ! info ) {
               REPORT_ERROR( "Couldn't use a TSlaveInfo object!" );
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
         // to m_logger. The log lines of the nodes will already be formatted,
         // so printing them through SLogger would just look ugly.
         //
         m_logger << INFO
                  << "=================================================="
                  << SLogger::endmsg;
         m_logger << INFO << "Output from node: " << nodeName << " ("
                  << element->GetName() << ")" << SLogger::endmsg;

         element->GetMacro()->Print();

         m_logger << INFO
                  << "=================================================="
                  << SLogger::endmsg;

      }

      // It's up to us to delete the TProofLog object:
      delete log;
   }

   return;
}
