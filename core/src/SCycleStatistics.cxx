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
#include <TCollection.h>
#include <TDirectory.h>

// Local include(s):
#include "../include/SCycleStatistics.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleStatistics )
#endif // DOXYGEN_IGNORE

/**
 * @param name The name of the statistics object
 * @param procEvents Number of processed events
 * @param skipEvents Number of skipped events
 */
SCycleStatistics::SCycleStatistics( const char* name, Long64_t procEvents,
                                    Long64_t skipEvents )
   : TNamed( name, "SFrame cycle statistics" ),
     m_processedEvents( procEvents ), m_skippedEvents( skipEvents ),
     m_logger( "SCycleStatistics" ) {

}

/**
 * @returns The number of processed events
 */
Long64_t SCycleStatistics::GetProcessedEvents() const {

   return m_processedEvents;
}

/**
 * @param events The number of processed events
 */
void SCycleStatistics::SetProcessedEvents( Long64_t events ) {

   m_processedEvents = events;
   return;
}

/**
 * @returns The number of skipped events
 */
Long64_t SCycleStatistics::GetSkippedEvents() const {

   return m_skippedEvents;
}

/**
 * @param events The number of skipped events
 */
void SCycleStatistics::SetSkippedEvents( Long64_t events ) {

   m_skippedEvents = events;
   return;
}

/**
 * The merging is done in a *very* simple manner, just adding up the member
 * variables.
 *
 * @param coll The collection of objects to merge into this one
 * @returns Zero if some problem happened, something else if everything was okay
 */
Int_t SCycleStatistics::Merge( TCollection* coll ) {

   //
   // Return right away if the input is flawed:
   //
   if( ! coll ) return 0;
   if( coll->IsEmpty() ) return 0;

   REPORT_VERBOSE( "Merging statistics object" );

   //
   // Select the elements from the collection that can actually be merged:
   //
   TIter next( coll );
   TObject* obj = 0;
   while( ( obj = next() ) ) {

      //
      // See if it is an SCycleStatistics object itself:
      //
      SCycleStatistics* sobj = dynamic_cast< SCycleStatistics* >( obj );
      if( ! sobj ) {
         REPORT_ERROR( "Trying to merge \"" << obj->ClassName()
                       << "\" object into \"" << this->ClassName() << "\"" );
         continue;
      }

      //
      // Add the statistics from one worker:
      //
      m_processedEvents += sobj->m_processedEvents;
      m_skippedEvents   += sobj->m_skippedEvents;

      REPORT_VERBOSE( sobj->m_processedEvents
                      << " events processed on one worker" );
      REPORT_VERBOSE( sobj->m_skippedEvents
                      << " events skipped on one worker" );
   }

   m_logger << DEBUG << "Merged statistics objects" << SLogger::endmsg;

   return 1;
}

/**
 * This function is not really used actually. It would make it possible to write
 * out the cycle statistics into the output file, but the code doesn't do this
 * at the moment.
 *
 * The function is actually smart enough to handle all SFrame setup situations
 * correctly, so it can update objects in an existing output file.
 *
 * @param name The name under which the object should be saved
 * @param option Extra option influencing the write operation
 * @param bufsize The size of the buffer used in the file writing
 */
Int_t SCycleStatistics::Write( const char* name, Int_t option,
                               Int_t bufsize ) const {

   TObject* original_obj;
   if( ( original_obj = gDirectory->Get( GetName() ) ) ) {
      m_logger << DEBUG << "Merging object \"" << GetName()
               << "\" with already existing object..." << SLogger::endmsg;

      SCycleStatistics* sobject =
         dynamic_cast< SCycleStatistics* >( original_obj );
      if( ! sobject ) {
         m_logger << WARNING << "Already existing object with name \""
                  << original_obj->GetName()
                  << "\" is not of type SCycleStatistics!" << SLogger::endmsg;
         m_logger << WARNING
                  << "Merging is not possible, so it will be overwritten..."
                  << SLogger::endmsg;
      } else {
         TList list;
         list.Add( const_cast< SCycleStatistics* >( this ) );
         sobject->Merge( &list );
         return 1;
      }

   }

   // Call the standard ROOT write function:
   return TObject::Write( name, option, bufsize );
}

/**
 * This function is just a different signature for the same writing function
 * that is implemented in the constant version. See that function for more
 * details.
 */
Int_t SCycleStatistics::Write( const char* name, Int_t option, Int_t bufsize ) {

   return const_cast< const SCycleStatistics* >( this )->Write( name, option,
                                                                bufsize );
}
