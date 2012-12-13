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

#ifndef SFRAME_CORE_SProofManager_H
#define SFRAME_CORE_SProofManager_H

// STL include(s):
#include <map>

// ROOT include(s):
#include <TString.h>

// Local include(s):
#include "SLogger.h"
#include "SError.h"

// Forward declaration(s):
class TProof;

/**
 *   @short Singleton class managing the PROOF connection(s)
 *
 *          When handling PROOF (PQ2) datasets, the code needs to access the
 *          PROOF server in multiple places. Since the PROOF connection needs
 *          some time to properly terminate after deleting the appropriate
 *          objects, and of course SFrame should not sit waiting until that
 *          happens, it's just easier to keep the connections open.
 *
 *          The Open(...) function can be user pretty similarly to
 *          TProof::Open(...). The advantage of this function is that it
 *          remembers which connections are already open, and it also deletes
 *          them when instructed (or when deleted).
 *
 * @version $Revision$
 */
class SProofManager {

   /// Typedef for the internal cache
   typedef std::map< std::pair< TString, TString >,
                     std::pair< TProof*, Bool_t > > ConnMap_t;

public:
   /// Destructor that deletes the open connections
   ~SProofManager();

   /// Singleton accessor function
   static SProofManager* Instance();

   /// Function to open/access a PROOF connection
   TProof* Open( const TString& url,
                 const TString& param = "" ) throw( SError );
   /// Function to check if a PROOF server connection is configured already
   Bool_t IsConfigured( const TString& url, const TString& param = "" ) const;
   /// Set a given PROOF server to "configured" state
   void SetConfigured( const TString& url, const TString& param = "",
                       Bool_t state = kTRUE ) throw( SError );
   /// Function deleting all the open PROOF connections
   void Cleanup();

private:
   /// The constructor is private, following the singleton design
   SProofManager();
   /// Function printing the logs of all the workers from all the connections
   void PrintWorkerLogs() const;

   /// Internal cache of the open connections
   ConnMap_t m_connections;

   /// Singleron instance of the object
   static SProofManager* m_instance;
   /// Object for printing messages to the terminal
   mutable SLogger m_logger;

}; // class SProofManager

#endif // SFRAME_CORE_SProofManager_H
