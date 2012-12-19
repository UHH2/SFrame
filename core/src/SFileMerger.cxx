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

// STL include(s):
#include <set>
#include <string>

// ROOT include(s):
#include <TObject.h>
#include <TString.h>
#include <TFile.h>
#include <TList.h>
#include <TTree.h>
#include <TKey.h>
#include <TSystem.h>
#include <TUUID.h>
#include <TMethodCall.h>

// Local include(s):
#include "../include/SFileMerger.h"

SFileMerger::SFileMerger()
   : m_inputFiles(), m_outputFile( 0 ), m_logger( "SFileMerger" ) {

}

SFileMerger::~SFileMerger() {

   // Close all files before deleting the object
   CloseFiles();
}

/**
 * This function adds a new file as input for the merging.
 *
 * @param fileName The name of the input file
 * @returns <code>kTRUE</code> if everything went correctly,
 *          <code>kFALSE</code> otherwise
 */
Bool_t SFileMerger::AddFile( const TString& fileName ) throw( SError ) {

   //
   // Copy the file locally. This is important when reading an ntuple file
   // from a remote PROOF farm that might be half way around the world...
   //
   TUUID uuid;
   const TString localName =
      TString::Format( "%s/SFRAMEMERGE-%s.root",
                       ( gSystem->Getenv( "SFRAME_TEMP_DIR" ) ?
                         gSystem->Getenv( "SFRAME_TEMP_DIR" ) :
                         gSystem->TempDirectory() ), uuid.AsString() );
   if( ! TFile::Cp( fileName, localName, kTRUE ) ) {
      REPORT_ERROR( "Couldn't create local copy of: " << fileName );
      throw SError( "Couldn't create local copy of: " + fileName,
                    SError::SkipCycle );
      return kFALSE;
   }
   REPORT_VERBOSE( fileName << " copied locally as " << localName );

   //
   // Try to open the specified file. Throw an exception if it wasn't possible.
   //
   TFile* ifile = TFile::Open( localName, "READ" );
   if( ! ifile ) {
      REPORT_ERROR( "Local file could not be opened: " << localName );
      throw SError( "Local file could not be opened: " + fileName,
                    SError::SkipCycle );
      return kFALSE;
   }
   m_inputFiles.push_back( ifile );
   REPORT_VERBOSE( localName << " opened for reading" );

   // Return gracefully:
   return kTRUE;
}

/**
 * This function specifies the name of the output file.
 *
 * @param fileName The name of the output file to be created
 * @param mode Mode to open the output file in
 * @returns <code>kTRUE</code> if everything went correctly,
 *          <code>kFALSE</code> otherwise
 */
Bool_t SFileMerger::OutputFile( const TString& fileName,
                                const TString& mode ) throw( SError ) {

   //
   // Try to open the specified output file. Throw an expection in case of
   // problems.
   //
   TFile* ofile = TFile::Open( fileName, mode );
   if( ! ofile ) {
      REPORT_ERROR( "Couldn't open output file \"" << fileName
                    << "\" in mode \"" << mode << "\"" );
      throw SError( "Output file could not be opened: " + fileName,
                    SError::SkipCycle );
      return kFALSE;
   }
   m_outputFile = ofile;
   REPORT_VERBOSE( fileName << " opened for writing" );

   // Return gracefully:
   return kTRUE;
}

/**
 * This is the main function of this class. It was heavily inspired by the
 * TFileMerger::MergeRecursive function, which in turn is basically a copy of
 * the MergeRootfile function of the hadd executable.
 *
 * @returns <code>kTRUE</code> if the merge was successful, <code>kFALSE</code>
 *          otherwise
 */
Bool_t SFileMerger::Merge() throw( SError ) {

   //
   // Check that we have both input(s) and an output:
   //
   if( ! m_outputFile ) {
      REPORT_ERROR( "Merge(): Output file not specified yet" );
      return kFALSE;
   }
   if( ! m_inputFiles.size() ) {
      m_logger << WARNING
               << "Merge(): No input files specified. Noting to be done..."
               << SLogger::endmsg;
      return kFALSE;
   }

   m_logger << DEBUG << "Running file merging..." << SLogger::endmsg;

   //
   // Loop over all input files:
   //
   for( std::vector< TFile* >::const_iterator ifile = m_inputFiles.begin();
        ifile != m_inputFiles.end(); ++ifile ) {

      REPORT_VERBOSE( "Now processing file: " << ( *ifile )->GetName() );

      // Call the recursive merging function:
      MergeDirectory( *ifile, m_outputFile );
   }

   //
   // Make sure that everything in the output is written out:
   //
   m_outputFile->SaveSelf( kTRUE );
   CloseFiles();

   // Return gracefully:
   return kTRUE;
}

void SFileMerger::CloseFiles() {

   for( std::vector< TFile* >::iterator ifile = m_inputFiles.begin();
        ifile != m_inputFiles.end(); ++ifile ) {
      ( *ifile )->Close();
      // Remove the local copy of the file:
      TString p( ( *ifile )->GetPath() );
      p = p( 0, p.Index( ':', 0 ) );
      REPORT_VERBOSE( "Removing local file: " << p );
      gSystem->Unlink( p );
      delete ( *ifile );
   }
   m_inputFiles.clear();
   if( m_outputFile ) delete m_outputFile;
   m_outputFile = 0;

   return;
}

/**
 * This recursive function is taking care about merging all the TTree-s from one
 * directory into the TTree-s of the output directory. If it finds a directory
 * on the input, it calls itself for that directory.
 *
 * The result should be that all TTree-s from all the sub-directories should get
 * merged into the output.
 *
 * @param input The input directory
 * @param output The output directory
 */
void SFileMerger::MergeDirectory( TDirectory* input,
                                  TDirectory* output ) throw( SError ) {

   // Get a list of all objects in this directory:
   TList* keyList = input->GetListOfKeys();

   //
   // Loop over all keys in the root directory, and select the ones describing
   // a TTree or a TDirectory. Since one single object can appear multiple times
   // in this list (with different "cycles"), keep track of which objectss have
   // already been merged into the output.
   //
   std::set< std::string > processedObjects;
   for( Int_t i = 0; i < keyList->GetSize(); ++i ) {

      // Convert to a TKey:
      TKey* key = dynamic_cast< TKey* >( keyList->At( i ) );
      if( ! key ) {
         REPORT_ERROR( "Couldn't cast to TKey. There is some problem in the "
                       "code" );
         throw SError( "Couldn't cast to TKey. There is some problem in the "
                       "code", SError::StopExecution );
      }

      //
      // Check whether we already processed an object with this name:
      //
      REPORT_VERBOSE( "Processing key with name: " << key->GetName()
                      << ";" << key->GetCycle() );
      if( processedObjects.find( key->GetName() ) != processedObjects.end() ) {
         m_logger << DEBUG << "Object \"" << key->GetName()
                  << "\" has already been processed" << SLogger::endmsg;
         continue;
      }

      //
      // Get the object:
      //
      TObject* obj = input->Get( key->GetName() );
      if( ! obj ) {
         REPORT_ERROR( "Couldn't access object with name '" << key->GetName()
                       << "'" );
         throw SError( "Couldn't access object for which we got a key",
                       SError::StopExecution );
      }

      //
      // Decide how to handle this object:
      //
      if( obj->IsA()->InheritsFrom( "TDirectory" ) ) {

         // Access the input object as a directory:
         TDirectory* indir = dynamic_cast< TDirectory* >( obj );
         if( ! indir ) {
            REPORT_ERROR( "Couldn't cast to object to TDirectory" );
            continue;
         }

         // Check if such a directory already exists in the output:
         TDirectory* outdir =
            dynamic_cast< TDirectory* >( output->Get( key->GetName() ) );
         // If it doesn't let's create it:
         if( ! outdir ) {
            if( ! ( outdir = output->mkdir( key->GetName(),
                                            "dummy title" ) ) ) {
               REPORT_ERROR( "Failed creating subdirectory with name: "
                             << key->GetName() );
               throw SError( "Failed creating subdirectory",
                             SError::SkipInputData );
            }
         }

         // Now call this same function recursively:
         MergeDirectory( indir, outdir );

      } else if( obj->IsA()->InheritsFrom( "TTree" ) ) {

         //
         // See if such a TTree exists in the output already:
         //
         TTree* otree = 0;
         if( ( otree =
               dynamic_cast< TTree* >( output->Get( key->GetName() ) ) ) ) {

            //
            // If it does, then use the TTree:Merge function to copy the
            // contents of the TTree in the input file into the existing TTree
            // in the output file.
            //
            TList itrees;
            itrees.Add( obj );
            if( otree->Merge( &itrees ) ) {
               m_logger << DEBUG << "Merged tree \"" << obj->GetName()
                        << "\" from file: " << input->GetName()
                        << SLogger::endmsg;
               otree->AutoSave();
            } else {
               throw SError( TString( "There was a problem with merging "
                                      "trees \""  ) + obj->GetName() + "\"",
                             SError::SkipCycle );
            }

         } else {

            //
            // If it doesn't exist, then use the TTree::CloneTree function to
            // create a copy of the TTree in the input file. Then save this
            // copy into the output file.
            //
            output->cd();
            TTree* itree = 0;
            if( ! ( itree = dynamic_cast< TTree* >( obj ) ) ) {
               REPORT_ERROR( "Coulnd't dynamic cast object to TTree" );
               continue;
            }

            //
            // TTree::MergeTrees would crash in case the input TTree is empty,
            // so instead let's use TTree::CloneTree
            //
            if( ( otree = itree->CloneTree( -1, "fast" ) ) ) {
               m_logger << DEBUG << "Cloned tree \"" << itree->GetName()
                        << "\" into file: " << m_outputFile->GetName()
                        << SLogger::endmsg;
               otree->SetDirectory( output );
               otree->AutoSave();
            } else {
               throw SError( TString( "Tree \"" ) + itree->GetName() +
                             "\" couldn't be cloned into the output",
                             SError::SkipCycle );
            }

         }

         // Remember that this TTree has already been processed:
         processedObjects.insert( obj->GetName() );

      } else if( obj->IsA()->InheritsFrom( "TObject" ) ) {

         // Check if the object is already in the output:
         TObject* oobj = output->Get( key->GetName() );

         // Decide what to do:
         if( oobj ) {
            // If the object already exists, merge the new object into it:
            MergeObjects( obj, oobj );
            m_logger << DEBUG << "Merged object \"" << obj->GetName()
                     << "\" into file: " << m_outputFile->GetName()
                     << SLogger::endmsg;
         } else {
            // If the object doesn't exist yet, just write this object to the
            // output:
            output->cd();
            obj->Write();
            m_logger << DEBUG << "Cloned object \"" << obj->GetName()
                     << "\" into file: " << m_outputFile->GetName()
                     << SLogger::endmsg;
         }
      }

   }

   return;
}

/**
 * This internal function takes care of merging two objects together.
 * Since TObject doesn't have a Merge(...) function, we have to do it
 * with a bit more code.
 *
 * @param in The input object
 * @param out The object into which the input object should be merged
 */
void SFileMerger::MergeObjects( TObject* in, TObject* out ) throw( SError ) {

   // Put the input object into a list:
   TList list;
   list.Add( in );

   //
   // Make sure that the output object supports merging:
   //
   TMethodCall mergeMethod;
   mergeMethod.InitWithPrototype( out->IsA(), "Merge", "TCollection*" );
   if( ! mergeMethod.IsValid() ) {
      REPORT_ERROR( "Object type \"" << out->ClassName()
                    << "\" doesn't support merging" );
      return;
   }

   //
   // Execute the merging:
   //
   mergeMethod.SetParam( ( Long_t ) &list );
   mergeMethod.Execute( out );

   // Let the user know what we did:
   REPORT_VERBOSE( "Merged objects of type \"" << out->ClassName()
                   << "\" and name: " << out->GetName() );

   // Return gracefully:
   return;
}
