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

// ROOT include(s):
#include <TProof.h>
#include <TProofMgr.h>
#include <TProofLog.h>
#include <TMacro.h>
#include <TList.h>

// Local include(s):
#include "../include/SProofManager.h"

// Initialize the static variable:
SProofManager* SProofManager::m_instance = 0;

/**
 * The destructor cleans up the open connections by calling the Cleanup() function
 * internally.
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
 * TProof::Open(...).
 *
 * The function first looks at its internal cache to see if the connection
 * is already open, and if it is, it just returns the cached TProof object.
 * If the connection is not yet available, it creates it using TProof::Open(...),
 * and adds the created object to its internal cache.
 *
 * @param url Name of the PROOF server
 * @param param Additional parameters given to TProof::Open(...)
 * @returns The created TProof object
 */
TProof* SProofManager::Open( const TString& url, const TString& param ) throw( SError ) {

   // Check if the connection has already been opened:
   ConnMap_t::key_type connection = std::make_pair( url, param );
   ConnMap_t::const_iterator conn;
   if( ( conn = m_connections.find( connection ) ) != m_connections.end() ) {
      m_logger << DEBUG << "Connection to \"" << url << "\" is already open"
               << SLogger::endmsg;
      return conn->second;
   }

   // Try to open the connection:
   TProof* server = TProof::Open( url, param );
   if( ! server ) {
      m_logger << ERROR << "Couldn't open connection to: " << url << SLogger::endmsg;
      throw SError( "Couldn't open connection to: " + url,
                    SError::SkipCycle );
   } else {
      m_logger << INFO << "Connection opened to \"" << url << "\"" << SLogger::endmsg;
   }

   m_connections[ connection ] = server;

   return server;
}

/**
 * This function can be used to clean up the PROOF connections. Even if it's only called
 * at the termination of the sframe_main program, it's still very much needed. PROOF has
 * the habit of producing a crash at the very last moment in the application if the user
 * doesn't delete its objects properly.
 *
 * The function first retrieves and prints all the worker logs from all the connections,
 * then it does the cleanup.
 */
void SProofManager::Cleanup() {

   PrintWorkerLogs();

   if( m_connections.size() ) {
      TProofMgr* mgr = m_connections.begin()->second->GetManager();
      for( ConnMap_t::iterator server = m_connections.begin();
           server != m_connections.end(); ++server ) {
         delete server->second;
      }
      delete mgr;
   }
   m_connections.clear();

   return;
}

/**
 * The constructor just initializes the member variables, and doesn't do anything
 * in addition.
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
      m_logger << INFO << "***************************************************************"
               << SLogger::endmsg;
      m_logger << INFO << "*" << SLogger::endmsg;
      m_logger << INFO << "* Printing all worker logs from server:"
               << SLogger::endmsg;
      m_logger << INFO << "*     " << server->first.first << SLogger::endmsg;
      m_logger << INFO << "*" << SLogger::endmsg;
      m_logger << INFO << "***************************************************************"
               << SLogger::endmsg;

      //
      // Get info about the slaves:
      //
      TList* slaveInfos = server->second->GetListOfSlaveInfos();

      //
      // Retrieve all logs:
      //
      TProofLog* log = server->second->GetManager()->GetSessionLogs();
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
         TString nodeName = server->second->GetMaster();
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

      // It's up to us to delete the TProofLog object:
      delete log;
   }

   return;

}
