// $Id: SCycleBase.cxx,v 1.2 2007-11-22 18:19:26 krasznaa Exp $
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
#include <vector>
#include <sstream>
#include <cstdlib>

// ROOT include(s):
#include <TFile.h>

// Local include(s):
#include "../include/SCycleBase.h"
#include "../include/SInputData.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBase );
#endif // DOXYGEN_IGNORE

using namespace std;

/**
 * The constructor only silently creates the base objects and initialises
 * memer variables.
 */
SCycleBase::SCycleBase()
   : m_nProcessedEvents( 0 ) {

   m_logger << VERBOSE << "SCycleBase constructed" << SLogger::endmsg;

}


/**
 * Another one of the "I don't do anything" destructors.
 */
SCycleBase::~SCycleBase() {

   m_logger << VERBOSE << "SCycleBase destructed" << SLogger::endmsg;

}

/**
 * Function steering the execution of the analysis cycle. This is the function
 * called by the framework to issue the cycle to perform the analysis.
 *
 * @callgraph
 */
void SCycleBase::ExecuteInputData() throw( SError ) {

   for( std::vector< SInputData >::iterator iD = m_inputData.begin(); iD != m_inputData.end();
        ++iD ) {
      this->CheckInputFiles( *iD );
   }

   vector< TTree* > outputTrees;
   TFile* outputFile = 0;

   // iterate over SInputData objects
   for( std::vector< SInputData >::const_iterator iD = m_inputData.begin();
        iD != m_inputData.end(); ++iD ) {

      m_logger << DEBUG << "ExecuteInputData: process InputData object of type \""  
               << iD->GetType() << "\"" << SLogger::endmsg;

      // check whether this InputData object is of the same type as
      // the previous one, in which case output trees and other
      // objects will be kept and not recreated
      std::vector< SInputData >::const_iterator previous = iD;
      if(previous == m_inputData.begin()) m_firstInputDataOfMany = true;
      else {
         --previous;
         if(previous->GetType() != iD->GetType()) {
            m_firstInputDataOfMany = true;
            //sanity check
            if(m_keepOutputFile == true) {
               SError error( SError::SkipInputData );
               error << "Previous InputData object kept the output file open" 
                     << ", but the current InputData object is supposed to be the very first "
                     << " of its kind ! ";
               throw error;
            }
         }
         else {
            m_firstInputDataOfMany = false;
            m_logger << INFO << "This is not the first InputData object of type \"" 
                     << iD->GetType() 
                     << "\", output trees and other objects will not be recreated!"
                     << SLogger::endmsg;
         }
      }

      // check whether output file should be created or not, now that
      // the inputData objects are ordered according to their type
      std::vector< SInputData >::const_iterator next = iD;
      ++next;
      if( next != m_inputData.end() ) {
         if( next->GetType() == iD->GetType() ) {
            m_logger << INFO << "Keep output file "
                     << "open for next InputData object of type " 
                     << next->GetType() << SLogger::endmsg;
            m_keepOutputFile = true;
         } else m_keepOutputFile = false;
      } else m_keepOutputFile = false;

      try { // For catching InputData level problems...

         // count the number of events processed for this SInputData
         Long64_t nProcessedEventsForThisInputData = 0 ;

         // get the number of events to be processed for this SInputData
         Long64_t evToProcess = iD->GetNEventsMax();
         Long64_t totalEvents = iD->GetEventsTotal();
         if( evToProcess < 0 ) {
            evToProcess = totalEvents;
            m_logger << INFO << "Events to process : " << evToProcess << SLogger::endmsg;
         } else {
            m_logger << INFO << "Events to process : " << evToProcess << " / " << totalEvents
                     << SLogger::endmsg;
         }

         Bool_t InputDataIsInitialised = false;

         outputTrees.clear();
         outputFile = 0;
         if( m_firstInputDataOfMany ) {
            // open output file and create output trees therein
            this->CreateOutputTrees( *iD, outputTrees, outputFile );
            this->InitHistogramming( outputFile, this->GetOutputFileName() );
         }

         // now loop over input files, for each file get all the input
         // trees, fill inputTrees with the trees, and then
         // process the events for one file at a time
         const std::vector< SFile >& sfile = iD->GetSFileIn();
         for( std::vector< SFile >::const_iterator sf = sfile.begin();
              sf != sfile.end(); ++sf ) {

            TFile* file = 0;

            try { // For catching input file level problems...

               // stop the loop if number of processed events exceeds maxevents
               if( nProcessedEventsForThisInputData >= evToProcess ) break;

               this->LoadInputTrees( *iD, sf->file.Data(), file );
               this->ConnectEVSyncVariable();
               if( ! InputDataIsInitialised ) {
                  this->BeginInputData( *iD );
                  InputDataIsInitialised = true;
               }
               this->BeginInputFile( *iD );
               // loop over all entries
               for( Long64_t currentEvent = 0; currentEvent < GetNEvents(); ++currentEvent ) {

                  try { // For catching event level problems...

                     // stop the loop if number of processed events exceeds maxevents
                     if( nProcessedEventsForThisInputData >= evToProcess ) break;

                     // count the number of processed events in this SInputData
                     nProcessedEventsForThisInputData++;

                     // count the number of processed events in this cycle
                     m_nProcessedEvents++;

                     if( ( currentEvent % 1000 ) == 0 ) {
                        m_logger << INFO << "Processed events: " << currentEvent << " / "
                                 << GetNEvents() << SLogger::endmsg;
                     }

                     this->GetEntry( currentEvent );

                     this->SyncEVTrees();

                     this->ExecuteEvent( *iD, this->CalculateWeight( *iD, currentEvent ) );

                     // if ExecuteEvent, fill output trees
                     int nbytes = 0;
                     for( std::vector< TTree* >::iterator tree = outputTrees.begin();
                          tree != outputTrees.end(); ++tree ) {
                        nbytes = ( *tree )->Fill();
                        if( nbytes < 0 ) {
                           m_logger << WARNING << "Tree " << ( *tree )->GetName()
                                    << " write error occurred" << SLogger::endmsg;
                        } else if( nbytes == 0 ) {
                           m_logger << WARNING << "Tree " << ( *tree )->GetName()
                                    << " no data written" << SLogger::endmsg;
                        }
                     }

                  } catch( const SError& error ) {
                     //
                     // This is where I catch "event level" problems.
                     //
                     if( error.request() <= SError::SkipEvent ) {
                        // If just this event has to be skipped:
                        m_logger << VERBOSE << "Exeption caught while processing event: "
                                 << currentEvent << SLogger::endmsg;
                        m_logger << VERBOSE << "Message: " << error.what() << SLogger::endmsg;
                        m_logger << VERBOSE << "--> Skipping event!" << SLogger::endmsg;
                        continue;
                     } else {
                        // If this is more serious:
                        throw;
                     }
                  }

               } // end of loop over entries in the input file

               // close file
               file->Close();
               if( file ) delete file;

            } catch( const SError& error ) {
               //
               // This is where I catch "file level" problems:
               //
               if( error.request() <= SError::SkipFile ) {
                  // If just this file has to be skipped:
                  m_logger << ERROR << "Exception caught while processing file:"
                           << SLogger::endmsg;
                  m_logger << ERROR << "  " << sf->file.Data() << SLogger::endmsg;
                  m_logger << ERROR << "Message: " << error.what() << SLogger::endmsg;
                  m_logger << ERROR << "--> Skipping file!" << SLogger::endmsg;

                  continue;
               } else {
                  // If this is more serious:
                  throw;
               }
            }

         } // end loop over input files

         // finalise this SInputData element
         this->EndInputData( *iD );

         // save data:
         outputFile->cd();
         // save output trees
         for( vector< TTree* >::iterator tree = outputTrees.begin(); tree != outputTrees.end();
              ++tree ) {
            ( *tree )->AutoSave();
            if( ! m_keepOutputFile ) delete ( *tree );
         }
         if( ! m_keepOutputFile ) {
            // write outfile
            outputFile->Write();
            // close outfile
            outputFile->Close();
            delete outputFile;
            outputFile = 0;
            outputTrees.clear();
         }

      } catch( const SError& error ) {
         //
         // This is where I catch "InputData level" problems:
         //
         if( error.request() <= SError::SkipInputData ) {
            // If just this InputData has to be skipped:
            m_logger << ERROR << "Exception caught while processing InputData type: "
                     << iD->GetType() << SLogger::endmsg;
            m_logger << ERROR << "Message: " << error.what() << SLogger::endmsg;
            m_logger << ERROR << "--> Skipping InputData!" << SLogger::endmsg;

            //
            // Clean up before continuing:
            //
            if( outputFile ) {
               // save data:
               outputFile->cd();
               if( ! m_keepOutputFile ) {
                  // write outfile
                  outputFile->Write();
                  // close outfile
                  outputFile->Close();
                  delete outputFile;
                  outputFile = 0;
                  outputTrees.clear();
               }
            }
            continue;

         } else {
            // If this is more serious:
            throw;
         }
      }

   } // end loop over input data

   m_logger << INFO << "Finished processing input data" << SLogger::endmsg;
   return;

}

/**
 * Function checking the existence and basic contents of the specified input files.
 */
void SCycleBase::CheckInputFiles( SInputData& iD ) throw( SError ) {

   std::vector< SFile >& sfile = iD.GetSFileIn();
   for( std::vector< SFile >::iterator sf = sfile.begin(); sf != sfile.end(); ++sf ) {

      TFile* file = this->OpenInputFile( sf->file.Data() );

      const std::vector< STree >& sInTree = iD.GetInputTrees();
      Bool_t firstPassed = kFALSE;
      Long64_t entries = 0;
      Int_t numberOfBranches = 0;
      // try to load all the input trees
      for( vector< STree >::const_iterator st = sInTree.begin(); st != sInTree.end(); ++st ) {
         TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
         if( ! tree ) {
            SError error( SError::SkipFile );
            error << "Tree " << st->treeName << " doesn't exist in File "
                  << file->GetName();
            throw error;
         } else {
            if( firstPassed && tree->GetEntries() != entries ) {
               SError error( SError::SkipFile );
               error << "Conflict in number of entries - Tree " << tree->GetName()
                     << " has " << tree->GetEntries() << ", NOT "
                     << entries;
               throw error;
            } else if( ! firstPassed ) {
               firstPassed = kTRUE;
               entries = tree->GetEntries();
            }
            Int_t branchesThisTree = tree->GetNbranches();
            m_logger << DEBUG << branchesThisTree << " branches in tree " << tree->GetName() 
                     << SLogger::endmsg;
            numberOfBranches += branchesThisTree;
         }
      }

      sf->events = entries;
      iD.AddEvents( entries );

      // check EV trees
      const std::vector< SEVTree >& sEVInTree = iD.GetEVInputTrees();
      for( vector< SEVTree >::const_iterator st = sEVInTree.begin();
           st != sEVInTree.end(); ++st ) {
         TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
         if( ! tree ) {
            SError error( SError::SkipFile );
            error << "Tree " << st->treeName << " doesn't exist in File "
                  << file->GetName();
            throw error;
         }
         Int_t branchesThisTree = tree->GetNbranches();
         m_logger << DEBUG << branchesThisTree << " branches in tree " << tree->GetName() 
                  << SLogger::endmsg;
         numberOfBranches += branchesThisTree;
      }
      m_logger << DEBUG << numberOfBranches << " branches in total in file " << file->GetName() 
               << SLogger::endmsg;
      file->Close();
      if( file ) delete file;
   }

   m_logger << INFO << "Input type \"" << iD.GetType() << "\" version \"" 
            << iD.GetVersion() << "\" : " << iD.GetEventsTotal() << " events" 
            << SLogger::endmsg;

   return;
}

///////////////////////////////////////////////////////////////////
//                                                               //
//                  The "hidden" function(s):                    //
//                                                               //
///////////////////////////////////////////////////////////////////

void SCycleBase::InitHistogramming( TDirectory* outputFile, const TString& outputFileName ) {

   SCycleBaseHist::InitHistogramming( outputFile, outputFileName );
   return;

}

void SCycleBase::CreateOutputTrees( const SInputData& id, std::vector< TTree* >& trees,
                                    TFile*& file ) throw( SError ) {

   SCycleBaseNTuple::CreateOutputTrees( id, trees, file );
   return;

}

void SCycleBase::LoadInputTrees( const SInputData& id, const std::string& name,
                                 TFile*& file ) throw( SError ) {

   SCycleBaseNTuple::LoadInputTrees( id, name, file );
   return;

}

void SCycleBase::ConnectEVSyncVariable() throw( SError ) {

   SCycleBaseNTuple::ConnectEVSyncVariable();
   return;

}

void SCycleBase::GetEntry( Long64_t entry ) throw( SError ) {

   SCycleBaseNTuple::GetEntry( entry );
   return;

}

void SCycleBase::SyncEVTrees() throw( SError ) {

   SCycleBaseNTuple::SyncEVTrees();
   return;

}

Long64_t SCycleBase::GetNEvents() const {

   return SCycleBaseNTuple::GetNEvents();

}

Double_t SCycleBase::CalculateWeight( const SInputData& inputData, Long64_t entry ) {

   return SCycleBaseNTuple::CalculateWeight( inputData, entry );

}

TFile* SCycleBase::OpenInputFile( const char* filename ) throw( SError ) {

   return SCycleBaseNTuple::OpenInputFile( filename );

}

const char* SCycleBase::GetOutputFileName() const {

   return SCycleBaseNTuple::GetOutputFileName();

}
