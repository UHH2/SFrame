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

// STL include(s):
#include <set>
#include <string>

// ROOT include(s):
#include <TObject.h>
#include <TFile.h>
#include <TList.h>
#include <TTree.h>
#include <TKey.h>

// Local include(s):
#include "../include/SFileMerger.h"

SFileMerger::SFileMerger()
   : m_inputFiles(), m_outputFile( 0 ), m_logger( "SFileMerger" ) {

}

SFileMerger::~SFileMerger() {

   // Close all files before deleting the object
   CloseFiles();
}

void SFileMerger::AddInput( const TString& fileName ) throw( SError ) {

   //
   // Try to open the specified file. Throw an exception if it wasn't possible.
   //
   TFile* ifile = TFile::Open( fileName, "READ" );
   if( ! ifile ) {
      m_logger << ERROR << "Specified input file does not exist: "
               << fileName << SLogger::endmsg;
      throw SError( "Input file does not exist: " + fileName,
                    SError::SkipCycle );
   }
   m_inputFiles.push_back( ifile );
   m_logger << VERBOSE << fileName << " opened for reading" << SLogger::endmsg;

   return;
}

void SFileMerger::SetOutput( const TString& fileName ) throw( SError ) {

   //
   // Try to open the specified output file. Throw an expection in case of problems.
   //
   TFile* ofile = TFile::Open( fileName, "UPDATE" );
   if( ! ofile ) {
      m_logger << ERROR << "Couldn't open output file: "
               << fileName << SLogger::endmsg;
      throw SError( "Output file could not be opened: " + fileName,
                    SError::SkipCycle );
   }
   m_outputFile = ofile;
   m_logger << VERBOSE << fileName << " opened for writing" << SLogger::endmsg;

   return;
}

/**
 * This is the main function of this class. It was heavily inspired by the
 * TFileMerger::MergeRecursive function, which in turn is basically a copy of
 * the MergeRootfile function of the hadd executable.
 */
void SFileMerger::Merge() throw( SError ) {

   //
   // Check that we have both input(s) and an output:
   //
   if( ! m_outputFile ) {
      m_logger << ERROR << "Merge(): Output file not specified yet" << SLogger::endmsg;
      return;
   }
   if( ! m_inputFiles.size() ) {
      m_logger << WARNING << "Merge(): No input files specified. Noting to be done..."
               << SLogger::endmsg;
      return;
   }

   m_logger << DEBUG << "Running file merging..." << SLogger::endmsg;

   //
   // Loop over all input files:
   //
   for( std::vector< TFile* >::const_iterator ifile = m_inputFiles.begin();
        ifile != m_inputFiles.end(); ++ifile ) {

      m_logger << VERBOSE << "Now processing file: " << ( *ifile )->GetName()
               << SLogger::endmsg;

      //
      // I don't go into any sub-directories. SFrame only puts TTree-s in the root
      // directory of output files, so I only look for them there:
      //
      TList* keyList = ( *ifile )->GetListOfKeys();

      //
      // Loop over all keys in the root directory, and select the ones describing
      // a TTree. Since one single TTree can appear multiple times in this list
      // (with different "cycles"), keep track of which TTree-s have already been
      // merged into the output.
      //
      std::set< std::string > processedTrees;
      for( Int_t i = 0; i < keyList->GetSize(); ++i ) {
         TKey* key = dynamic_cast< TKey* >( keyList->At( i ) );
         if( ! key ) {
            throw SError( "Couldn't cast to TKey. There is some problem in the code",
                          SError::StopExecution );
         }

         //
         // Check whether we already processed an object with this name:
         //
         m_logger << VERBOSE << "Processing key with name: " << key->GetName()
                  << ";" << key->GetCycle() << SLogger::endmsg;
         if( processedTrees.find( key->GetName() ) != processedTrees.end() ) {
            m_logger << DEBUG << "Object \"" << key->GetName() << "\" has already been processed"
                     << SLogger::endmsg;
            continue;
         }

         TObject* obj = ( *ifile )->Get( key->GetName() );
         if( obj->IsA()->InheritsFrom( "TTree" ) ) {

            //
            // See if such a TTree exists in the output already:
            //
            TTree* otree = 0;
            if( ( otree = dynamic_cast< TTree* >( m_outputFile->Get( key->GetName() ) ) ) ) {

               //
               // If it does, then use the TTree:Merge function to copy the contents
               // of the TTree in the input file into the existing TTree in the output
               // file.
               //
               TList itrees;
               itrees.Add( obj );
               if( otree->Merge( &itrees ) ) {
                  m_logger << DEBUG << "Merged tree \"" << obj->GetName() << "\" from file: "
                           << ( *ifile )->GetName() << SLogger::endmsg;
                  otree->AutoSave();
               } else {
                  throw SError( TString( "There was a problem with merging trees \""  ) +
                                obj->GetName() + "\"", SError::SkipCycle );
               }

            } else {

               //
               // If it doesn't exist, then use the TTree::CloneTree function to
               // create a copy of the TTree in the input file. Then save this copy into
               // the output file.
               //
               m_outputFile->cd();
               TTree* itree = 0;
               if( ! ( itree = dynamic_cast< TTree* >( obj ) ) ) {
                  m_logger << ERROR << "Coulnd't dynamic cast object to TTree" << SLogger::endmsg;
                  continue;
               }

               //
               // TTree::MergeTrees would crash in case the input TTree is empty,
               // so instead let's use TTree::CloneTree
               //
               if( ( otree = itree->CloneTree( -1, "fast" ) ) ) {
                  m_logger << DEBUG << "Cloned tree \"" << itree->GetName()
                           << "\" into file: " << m_outputFile->GetName() << SLogger::endmsg;
                  otree->SetDirectory( m_outputFile );
                  otree->AutoSave();
               } else {
                  throw SError( TString( "Tree \"" ) + itree->GetName() +
                                "\" couldn't be cloned into the output", SError::SkipCycle );
               }

            }

            // Remember that this TTree has already been processed:
            processedTrees.insert( obj->GetName() );
         }

      }

   }

   //
   // Make sure that everything in the output is written out:
   //
   m_outputFile->SaveSelf( kTRUE );
   CloseFiles();

   return;
}

void SFileMerger::CloseFiles() {

   for( std::vector< TFile* >::iterator ifile = m_inputFiles.begin();
        ifile != m_inputFiles.end(); ++ifile ) {
      ( *ifile )->Close();
      delete ( *ifile );
   }
   m_inputFiles.clear();
   if( m_outputFile ) delete m_outputFile;
   m_outputFile = 0;

   return;
}
