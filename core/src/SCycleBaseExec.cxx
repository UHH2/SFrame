// $Id: SCycleBaseExec.cxx,v 1.4.2.2 2008-12-02 18:50:28 krasznaa Exp $
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
#include <TError.h>

// Local include(s):
#include "../include/SCycleBaseExec.h"
#include "../include/SInputData.h"
#include "../include/SCycleConfig.h"
#include "../include/SErrorHandler.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseExec );
#endif // DOXYGEN_IGNORE

/**
 * The constructor just initialises some member variable(s).
 */
SCycleBaseExec::SCycleBaseExec()
   : m_nProcessedEvents( 0 ) {

   SetLogName( this->GetName() );
   SetErrorHandler( SErrorHandler ); // Redirect ROOT messages to SErrorHandler
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

   SCycleConfig* config =
      dynamic_cast< SCycleConfig* >( fInput->FindObject( "CycleConfig" ) );
   if( ! config ) {
      m_logger << FATAL << "Couldn't retrieve the cycle configuration"
               << SLogger::endmsg;
      return;
   }
   this->SetConfig( *config );

   try {
      this->BeginCycle();
   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught while initialising cycle"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      throw;
   }

   m_logger << INFO << "Initialised cycle on master node" << SLogger::endmsg;

   return;

}

void SCycleBaseExec::SlaveBegin( TTree* ) {

   m_logger << VERBOSE << "In SCycleBaseExec::SlaveBegin()" << SLogger::endmsg;

   SCycleConfig* config =
      dynamic_cast< SCycleConfig* >( fInput->FindObject( "CycleConfig" ) );
   if( ! config ) {
      m_logger << FATAL << "Couldn't retrieve the cycle configuration"
               << SLogger::endmsg;
      return;
   }
   this->SetConfig( *config );

   m_inputData =
      dynamic_cast< SInputData* >( fInput->FindObject( "CurrentInputData" ) );
   if( ! m_inputData ) {
      m_logger << FATAL << "Couldn't retrieve the input data definition currently "
               << "being processed" << SLogger::endmsg;
      return;
   }

   this->SetHistOutput( fOutput );
   this->SetNTupleOutput( fOutput );

   m_outputTrees.clear();

   try {

      this->CreateOutputTrees( *m_inputData, m_outputTrees );
      this->BeginInputData( *m_inputData );

   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught while initialising input data"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      throw;
   }

   m_nProcessedEvents = 0;

   m_logger << INFO << "Initialised InputData \"" << m_inputData->GetType()
            << "\"on worker node" << SLogger::endmsg;

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
      m_logger << FATAL << "Exception caught while initialising input file"
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
   }

   ++m_nProcessedEvents;
   if( ! ( m_nProcessedEvents % 1000 ) ) {
      m_logger << INFO << "Processed events: " << m_nProcessedEvents
               << SLogger::endmsg;
   }

   return kTRUE;

}

void SCycleBaseExec::SlaveTerminate() {

   m_logger << VERBOSE << "In SCycleBaseExec::SlaveTerminate()" << SLogger::endmsg;

   try {
      this->EndInputData( *m_inputData );
   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught while ending input data"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      throw;
   }

   m_logger << INFO << "Terminated InputData \"" << m_inputData->GetType()
            << "\"on worker node" << SLogger::endmsg;

   return;

}

void SCycleBaseExec::Terminate() {

   m_logger << VERBOSE << "In SCycleBaseExec::Terminate()" << SLogger::endmsg;

   try {
      this->EndCycle();
   } catch( const SError& error ) {
      m_logger << FATAL << "Exception caught while ending cycle"
               << SLogger::endmsg;
      m_logger << FATAL << "Message: " << error.what() << SLogger::endmsg;
      throw;
   }

   return;

}
