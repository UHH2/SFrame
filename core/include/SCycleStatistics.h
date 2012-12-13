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

#ifndef SFRAME_CORE_SCycleStatistics_H
#define SFRAME_CORE_SCycleStatistics_H

// ROOT include(s):
#include <TNamed.h>

// Local include(s):
#include "../include/SLogger.h"

// Forward declaration(s):
class TCollection;

/**
 *   @short Special object to collect information from the workers with
 *
 *          I'm a bit disappointed in PROOF at this point... :-/ I tried
 *          getting some basic information out of it, like how many events were
 *          processed, but couldn't do it. (I could get other, non interesting
 *          info however.) So I decided to collect this information by hand.
 *
 *          This class is used by the framework internally to send statistics
 *          information from the workers to the master node.
 *
 * @version $Revision$
 */
class SCycleStatistics : public TNamed {

public:
   /// Constructor with all current parameters
   SCycleStatistics( const char* name = "", Long64_t procEvents = 0,
                     Long64_t skipEvents = 0 );

   /// Get the number of processed events
   Long64_t GetProcessedEvents() const;
   /// Set the number of processed events
   void SetProcessedEvents( Long64_t events );

   /// Get the number of skipped events
   Long64_t GetSkippedEvents() const;
   /// Set the number of skipped events
   void SetSkippedEvents( Long64_t events );

   /// Function merging the information from the worker nodes
   Int_t Merge( TCollection* coll );
   /// Write the object in the current output directory (const version)
   virtual Int_t Write( const char* name = 0, Int_t option = 0,
                        Int_t bufsize = 0 ) const;
   /// Write the object in the current output directory (non-const version)
   virtual Int_t Write( const char* name = 0, Int_t option = 0,
                        Int_t bufsize = 0 );

private:
   Long64_t m_processedEvents; ///< The number of processed events
   Long64_t m_skippedEvents;   ///< The number of skipped events

   /// Message logger object
   mutable SLogger m_logger; //!

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleStatistics, 1 )
#endif // DOXYGEN_IGNORE

}; // class SCycleStatistics

#endif // SFRAME_CORE_SCycleStatistics_H
