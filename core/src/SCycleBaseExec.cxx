// $Id: SCycleBaseExec.cxx,v 1.3 2008-02-08 16:21:10 krasznaa Exp $
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

// ROOT include(s):
#include <TFile.h>
#include <TTree.h>
#include <TStopwatch.h>
#include <TSystem.h>

// Local include(s):
#include "../include/SCycleBaseExec.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseExec );
#endif // DOXYGEN_IGNORE

/**
 * The constructor just initialises some member variable(s).
 */
SCycleBaseExec::SCycleBaseExec()
   : m_nProcessedEvents( 0 ) {

   m_logger << VERBOSE << "SCycleBaseExec constructed" << SLogger::endmsg;

}

/**
 * The destructor doesn't do anything.
 */
SCycleBaseExec::~SCycleBaseExec() {

   m_logger << VERBOSE << "SCycleBaseExec destructed" << SLogger::endmsg;

}

/**
 * This is the main "execute" function of the cycle. Once the cycle
 * has been correctly configured, the framework calls this function
 * which loops over all configured inputs and executes the cycle.
 */
void SCycleBaseExec::ExecuteInputData() throw( SError ) {

   //
   // "Check" the input files:
   //
   for( std::vector< SInputData >::iterator iD = m_inputData.begin();
        iD != m_inputData.end(); ++iD ) {
      this->CheckInputFiles( *iD );
   }

   std::vector< TTree* > outputTrees;
   TFile* outputFile = 0;

   // This timer is used to calculate the time spent analysing the
   // events:
   TStopwatch timer;
   timer.ResetCpuTime();
   timer.ResetRealTime();

   // iterate over SInputData objects
   for( std::vector< SInputData >::const_iterator iD = m_inputData.begin();
        iD != m_inputData.end(); ++iD ) {

      m_logger << DEBUG << "ExecuteInputData: process InputData object of type \""  
               << iD->GetType() << "\"" << SLogger::endmsg;

      // check whether this InputData object is of the same type as
      // the previous one, in which case output trees and other
      // objects will be kept and not recreated
      std::vector< SInputData >::const_iterator previous = iD;
      if( previous == m_inputData.begin() ) m_firstInputDataOfMany = true;
      else {
         --previous;
         if( previous->GetType() != iD->GetType() ) {
            m_firstInputDataOfMany = true;
            //sanity check
            if( m_keepOutputFile == true ) {
               SError error( SError::SkipInputData );
               error << "Previous InputData object kept the output file open" 
                     << ", but the current InputData object is supposed to be the very first"
                     << " of its kind !";
               throw error;
            }
         } else {
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
         Long64_t nProcessedEventsForThisInputData = 0;
         Long64_t nProcessedEventsForThisInputFile = 0;

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

         if( m_firstInputDataOfMany ) {
            // open output file and create output trees therein
            outputTrees.clear();
            outputFile = 0;
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
            nProcessedEventsForThisInputFile = 0;

            try { // For catching input file level problems...

               // stop the loop if number of processed events exceeds maxevents
               if( nProcessedEventsForThisInputData >= evToProcess ) break;

               this->LoadInputTrees( *iD, sf->file.Data(), file );
               if( ! InputDataIsInitialised ) {
                  this->BeginInputData( *iD );
                  InputDataIsInitialised = true;
               }
               this->BeginInputFile( *iD );

               // Measure used memory before the event loop:
               ProcInfo_t mem_before;
               gSystem->GetProcInfo( &mem_before );

               // Start the timer:
               timer.Start( kFALSE );

               // loop over all entries
               for( Long64_t currentEvent = 0; currentEvent < GetNEvents();
                    ++currentEvent ) {

                  try { // For catching event level problems...

                     // stop the loop if number of processed events exceeds maxevents
                     if( nProcessedEventsForThisInputData >= evToProcess ) break;

                     // count the number of processed events in this SInputData
                     nProcessedEventsForThisInputData++;

                     // count the number of processed events in the input file
                     nProcessedEventsForThisInputFile++;

                     // count the number of processed events in this cycle
                     m_nProcessedEvents++;

                     if( ( currentEvent % 1000 ) == 0 ) {
                        m_logger << INFO << "Processed events: " << currentEvent << " / "
                                 << GetNEvents() << SLogger::endmsg;
                     }

                     this->GetEntry( currentEvent );

                     this->ExecuteEvent( *iD, this->CalculateWeight( *iD, currentEvent ) );

                     // if ExecuteEvent returns gracefully, fill output trees
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

               // Stop the timer:
               timer.Stop();

               // Measure used memory after the event loop:
               ProcInfo_t mem_after;
               gSystem->GetProcInfo( &mem_after );

               m_logger << DEBUG << "Memory leaks while processing file:" << SLogger::endmsg;
               m_logger << DEBUG << "   Resident mem.: " << std::setw( 6 )
                        << ( mem_after.fMemResident - mem_before.fMemResident )
                        << " kB; " << std::setw( 7 )
                        << ( ( mem_after.fMemResident - mem_before.fMemResident ) /
                             static_cast< double >( nProcessedEventsForThisInputFile ) )
                        << " kB / event" << SLogger::endmsg;
               m_logger << DEBUG << "   Virtual mem. : " << std::setw( 6 )
                        << ( mem_after.fMemVirtual - mem_before.fMemVirtual )
                        << " kB; " << std::setw( 7 )
                        << ( ( mem_after.fMemVirtual - mem_before.fMemVirtual ) /
                             static_cast< double >( nProcessedEventsForThisInputFile ) )
                        << " kB / event" << SLogger::endmsg;

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
         for( std::vector< TTree* >::iterator tree = outputTrees.begin();
              tree != outputTrees.end(); ++tree ) {
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

   m_logger << INFO << "Finished processing all input data" << SLogger::endmsg;

   //
   // Print statistics on the pure event processing speed:
   //
   m_logger << INFO << "Pure event processing statistics:" << SLogger::endmsg;
   Double_t nev = static_cast< Double_t >( NumberOfProcessedEvents() );
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

/**
 * This function calculates the number of events in the input TTree-s
 * to be able to correctly calculate the event weights if only a subset
 * of all the events are processed.
 */
void SCycleBaseExec::CheckInputFiles( SInputData& iD ) throw( SError ) {

   std::vector< SFile >& sfile = iD.GetSFileIn();
   for( std::vector< SFile >::iterator sf = sfile.begin(); sf != sfile.end(); ++sf ) {

      TFile* file = this->OpenInputFile( sf->file.Data() );

      const std::vector< STree >& sInTree = iD.GetInputTrees();
      Bool_t firstPassed = kFALSE;
      Long64_t entries = 0;
      Int_t numberOfBranches = 0;
      // try to load all the input trees
      for( std::vector< STree >::const_iterator st = sInTree.begin();
           st != sInTree.end(); ++st ) {
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

      // check EV trees
      const std::vector< SEVTree >& sEVInTree = iD.GetEVInputTrees();
      for( std::vector< SEVTree >::const_iterator st = sEVInTree.begin();
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

      // Check the persistent tree(s):
      const std::vector< STree >& sPersTree = iD.GetPersTrees();
      for( std::vector< STree >::const_iterator st = sPersTree.begin();
           st != sPersTree.end(); ++st ) {
         TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
         if( ! tree ) {
            SError error( SError::SkipFile );
            error << "Tree " << st->treeName << " doesn't exist in File "
                  << file->GetName();
            throw error;
         } else {
            if( firstPassed && tree->GetEntriesFast() != entries ) {
               SError error( SError::SkipFile );
               error << "Conflict in number of entries - Tree " << tree->GetName()
                     << " has " << tree->GetEntries() << ", NOT "
                     << entries;
               throw error;
            } else if( ! firstPassed ) {
               firstPassed = kTRUE;
               entries = tree->GetEntriesFast();
            }
         }
         Int_t branchesThisTree = tree->GetNbranches();
         m_logger << DEBUG << branchesThisTree << " branches in tree " << tree->GetName() 
                  << SLogger::endmsg;
         numberOfBranches += branchesThisTree;
      }

      sf->events = entries;
      iD.AddEvents( entries );

      m_logger << DEBUG << numberOfBranches << " branches in total in file "
               << file->GetName() << SLogger::endmsg;
      file->Close();
      if( file ) delete file;
   }

   m_logger << INFO << "Input type \"" << iD.GetType() << "\" version \"" 
            << iD.GetVersion() << "\" : " << iD.GetEventsTotal() << " events" 
            << SLogger::endmsg;

   return;
}
