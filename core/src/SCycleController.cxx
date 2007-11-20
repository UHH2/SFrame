// $Id: SCycleController.cxx,v 1.2 2007-11-20 15:38:04 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author Andreas Hoecker  <Andreas.Hocker@cern.ch>       - CERN
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

// STL include(s):
#include <iomanip>
#include <iostream>

// ROOT include(s):
#include "TDOMParser.h"
#include "TXMLNode.h"
#include "TXMLDocument.h"
#include "TXMLAttr.h"
#include "TStopwatch.h"
#include "TSystem.h"
#include "TClass.h"

// Local include(s):
#include "../include/SCycleController.h"
#include "../include/SCycleBase.h"
#include "../include/SLogWriter.h"

ClassImp( SCycleController );

SCycleController::SCycleController( const TString& xmlConfigFile )
   : m_curCycle( 0 ), m_isInitialized( kFALSE ), m_xmlConfigFile( xmlConfigFile ),
     m_logger( this ) {

}

SCycleController::~SCycleController() {

   std::vector< SCycleBase* >::const_iterator it = m_analysisCycles.begin();
   for( ; it != m_analysisCycles.end(); ++it) {
      delete ( *it );
   }

}

void SCycleController::Initialize() throw( SError ) {

   m_logger << INFO << "Initializing" << SLogger::endmsg;

   // first clean up everything in case this is called multiple times
   m_curCycle = 0;
   this->DeleteAllAnalysisCycles();

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
      SLogWriter::instance()->setMinType( type );

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
               TString outputdirectory = "";
               TString postfix = "";
               Double_t targetlumi= 0.;

               attribIt = nodes->GetAttributes();
               curAttr = 0;
               while ( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
                  if( curAttr->GetName() == TString( "Name" ) )
                     cycleName = curAttr->GetValue();
                  else if( curAttr->GetName() == TString( "OutputDirectory" ) )
                     outputdirectory = curAttr->GetValue();
                  else if( curAttr->GetName() == TString( "PostFix" ) )
                     postfix = curAttr->GetValue();
                  else if( curAttr->GetName() == TString( "TargetLumi" ) )
                     targetlumi = atof ( curAttr->GetValue() );
               }

               TClass* tmp = gROOT->GetClass( cycleName.c_str(), true );
               if( !tmp || !tmp->InheritsFrom( "SCycleBase" ) ) {
                  SError error( SError::SkipCycle );
                  error << "Loading of class \"" << cycleName << "\" failed";
                  throw error;
               }
               SCycleBase* cycle = ( SCycleBase* )tmp->New();
               cycle->SetOutputDirectory( outputdirectory );
               cycle->SetPostFix( postfix );
               cycle->SetTargetLumi( targetlumi );

               m_logger << INFO << "Found cycle '" << cycleName <<"' with"<< SLogger::endmsg;
               m_logger << INFO << "OutputDirectory '" << outputdirectory 
                        << "'; PostFix = '" << postfix 
                        << "'; targetlumi = " << targetlumi << SLogger::endmsg;

               TXMLNode* xmlNode = nodes->GetChildren();
               if( xmlNode != 0 ) {
                  cycle->Initialize( xmlNode );
                  this->AddAnalysisCycle( cycle );
               } else {
                  SError error( SError::SkipCycle );
                  error << "Cycle xml node \"" << nodes->GetNodeName()
                        << "\" has no children";
                  throw error;
               }

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
               if( ! gSystem->Load( libraryName.c_str() ) ) {
                  m_logger << DEBUG << "Library loaded: \"" << libraryName << "\"" 
                           << SLogger::endmsg;
               } else {
                  SError error( SError::StopExecution );
                  error << "Library failed to load: \"" << libraryName << "\"";
                  throw error;
               }

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

   // set object status to be ready
   m_isInitialized = kTRUE;

   return;
}

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

void SCycleController::ExecuteNextCycle() throw( SError ) {

   std::string cycleName = m_analysisCycles.at( m_curCycle )->GetName();
   m_logger << INFO << "Executing Cycle #" << m_curCycle << " ('" << cycleName << "')"
            << SLogger::endmsg;

   if( ! m_isInitialized ) {
      throw SError( "SCycleController is not initialized",
                    SError::StopExecution );
   }

   TStopwatch timer;
   timer.Start();

   try {

      m_analysisCycles.at( m_curCycle )->BeginCycle();
      m_analysisCycles.at( m_curCycle )->ExecuteInputData();
      m_analysisCycles.at( m_curCycle )->EndCycle();

      m_logger << INFO << "Executed Cycle #" << m_curCycle << " ('" << cycleName << "')"
               << SLogger::endmsg;

   } catch( const SError& error ) {
      //
      // This is where I catch "cycle level" problems:
      //
      if( error.request() <= SError::SkipCycle ) {
         m_logger << ERROR << "Caught exception in  Cycle #" << m_curCycle
                  << " ('" << cycleName << "')" << SLogger::endmsg;
         m_logger << ERROR << "Message: " << error.what() << SLogger::endmsg;
      } else {
         throw;
      }
   }

   Double_t nev = static_cast< Double_t >( m_analysisCycles.at( m_curCycle )->NumberOfProcessedEvents() );
   ++m_curCycle;

   timer.Stop();

   m_logger.setf( std::ios::fixed );
   m_logger << INFO << std::setw( 10 ) << std::setfill( ' ' ) << std::setprecision( 0 )
            << nev << " Events - Real time " << std::setw( 6 ) << std::setprecision( 2 )
            << timer.RealTime() << " s  - " << std::setw( 5 )
            << std::setprecision( 0 ) << ( nev / timer.RealTime() ) << " Hz | CPU time "
            << std::setw( 6 ) << std::setprecision( 2 ) << timer.CpuTime() << " s  - "
            << std::setw( 5 ) << std::setprecision( 0 ) << ( nev / timer.CpuTime() )
            << " Hz" << SLogger::endmsg;

   return;
}

void SCycleController::AddAnalysisCycle( SCycleBase* cycleAlg ) {

   m_analysisCycles.push_back( cycleAlg );
   return;

}

/*
  Delete all analysis cycle objects from memory!
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
