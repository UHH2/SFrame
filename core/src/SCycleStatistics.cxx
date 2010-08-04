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
#include <TCollection.h>
#include <TDirectory.h>

// Local include(s):
#include "../include/SCycleStatistics.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleStatistics );
#endif // DOXYGEN_IGNORE

SCycleStatistics::SCycleStatistics( const char* name, Long64_t procEvents,
                                    Long64_t skipEvents )
   : TNamed( name, "SFrame cycle statistics" ),
     m_processedEvents( procEvents ), m_skippedEvents( skipEvents ),
     m_logger( "SCycleStatistics" ) {

}

Long64_t SCycleStatistics::GetProcessedEvents() const {

   return m_processedEvents;

}

void SCycleStatistics::SetProcessedEvents( Long64_t events ) {

   m_processedEvents = events;
   return;

}

Long64_t SCycleStatistics::GetSkippedEvents() const {

   return m_skippedEvents;

}

void SCycleStatistics::SetSkippedEvents( Long64_t events ) {

   m_skippedEvents = events;
   return;

}

/**
 * The merging is done in a *very* simple manner, just adding the member variables.
 */
Int_t SCycleStatistics::Merge( TCollection* coll ) {

   //
   // Return right away if the input is flawed:
   //
   if( ! coll ) return 0;
   if( coll->IsEmpty() ) return 0;

   m_logger << VERBOSE << "Merging statistics object" << SLogger::endmsg;

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
         m_logger << ERROR << "Trying to merge \"" << obj->ClassName()
                  << "\" object into \"" << this->ClassName() << "\"" << SLogger::endmsg;
         continue;
      }

      //
      // Add the statistics from one worker:
      //
      m_processedEvents += sobj->m_processedEvents;
      m_skippedEvents   += sobj->m_skippedEvents;

      m_logger << VERBOSE << sobj->m_processedEvents << " events processed on one worker"
               << SLogger::endmsg;
      m_logger << VERBOSE << sobj->m_skippedEvents << " events skipped on one worker"
               << SLogger::endmsg;

   }

   m_logger << DEBUG << "Merged statistics objects" << SLogger::endmsg;

   return 1;

}

Int_t SCycleStatistics::Write( const char* name, Int_t option, Int_t bufsize ) const {

   TObject* original_obj;
   if( ( original_obj = gDirectory->Get( GetName() ) ) ) {
      m_logger << DEBUG << "Merging object \"" << GetName()
               << "\" with already existing object..." << SLogger::endmsg;

      SCycleStatistics* sobject = dynamic_cast< SCycleStatistics* >( original_obj );
      if( ! sobject ) {
         m_logger << WARNING << "Already existing object with name \""
                  << original_obj->GetName() << "\" is not of type SCycleStatistics!"
                  << SLogger::endmsg;
         m_logger << WARNING << "Merging is not possible, so it will be overwritten..."
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

Int_t SCycleStatistics::Write( const char* name, Int_t option, Int_t bufsize ) {

   return const_cast< const SCycleStatistics* >( this )->Write( name, option, bufsize );
}
