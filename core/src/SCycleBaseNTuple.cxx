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

// System include(s):
#include <string.h>

// STL include(s):
#include <algorithm>
#include <typeinfo>

// ROOT include(s):
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TBranch.h>
#include <TROOT.h>
#include <TList.h>
#include <TFriendElement.h>
#include <TVirtualIndex.h>
#include <TTreeFormula.h>
#include <TProofOutputFile.h>
#include <TSystem.h>

// Local include(s):
#include "../include/SCycleBaseNTuple.h"
#include "../include/SInputData.h"
#include "../include/SCycleConfig.h"
#include "../include/SCycleOutput.h"
#include "../include/STreeType.h"
#include "../include/SConstants.h"
#include "../include/SOutputFile.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseNTuple )
#endif // DOXYGEN_IGNORE

/**
 * The constructor is only initialising the base class.
 */
SCycleBaseNTuple::SCycleBaseNTuple()
   : SCycleBaseBase(), m_inputTrees(), m_inputBranches(), m_inputVarPointers(),
     m_outputFile( 0 ),
     m_outputTrees(), m_metaInputTrees(), m_outputVarPointers(),
     m_input( 0 ), m_output( 0 ) {

   REPORT_VERBOSE( "SCycleBaseNTuple constructed" );
}

/**
 * The destructor has to make sure that the objects created on the heap for the
 * input variables, are deleted.
 */
SCycleBaseNTuple::~SCycleBaseNTuple() {

   DeleteInputVariables();
   REPORT_VERBOSE( "SCycleBaseNTuple destructed" );
}

void SCycleBaseNTuple::SetNTupleOutput( TList* output ) {

   m_output = output;
   return;
}

TList* SCycleBaseNTuple::GetNTupleOutput() const {

   return m_output;
}

void SCycleBaseNTuple::SetNTupleInput( TList* input ) {

   m_input = input;
   return;
}

TList* SCycleBaseNTuple::GetNTupleInput() const {

   return m_input;
}

/**
 * This function can be used to retrieve both input and output metadata trees.
 * Input metadata trees are TTree-s that don't desribe event level information.
 * The reading of these trees is completely up to the user at this point.
 *
 * Output metadata trees are also completely in the control of the user. Entries
 * in them are only written out on the user's request.
 *
 * @param name Name of the requested metadata tree
 * @returns The pointer to the requested metadata tree
 */
TTree* SCycleBaseNTuple::
GetMetadataTree( const char* name ) const throw( SError ) {

   // The result tree:
   TTree* result = 0;

   // See if this is an input tree:
   try {
      result = GetInputMetadataTree( name );
      return result;
   } catch( const SError& error ) {
      if( error.request() <= SError::SkipFile ) {
         REPORT_VERBOSE( "Input metadata tree with name \"" << name
                         << "\" not found" );
      } else {
         REPORT_ERROR( "Exception message caught with message: "
                       << error.what() );
         throw;
      }
   }

   // See if this is an output tree:
   try {
      result = GetOutputMetadataTree( name );
      return result;
   } catch( const SError& error ) {
      if( error.request() <= SError::SkipFile ) {
         REPORT_VERBOSE( "Output metadata tree with name \"" << name
                         << "\" not found" );
      } else {
         REPORT_ERROR( "Exception message caught with message: "
                       << error.what() );
         throw;
      }
   }

   //
   // Throw an exception if the tree hasn't been found:
   //
   SError error( SError::SkipFile );
   error << "Couldn't find metadata TTree with name: " << name;
   throw error;

   return result;
}

/**
 * This function can be used to retrieve input metadata trees.
 * Input metadata trees are TTree-s that don't desribe event level information.
 * The reading of these trees is completely up to the user at this point.
 *
 * @param name Name of the requested input metadata tree
 * @returns The pointer to the requested metadata tree
 */
TTree* SCycleBaseNTuple::
GetInputMetadataTree( const char* name ) const throw( SError ) {

   //
   // Strip off the directory name from the given tree name:
   //
   TString tname( name );
   if( tname.Contains( "/" ) ) {
      REPORT_VERBOSE( "Tokenizing the metadata tree name: " << tname );
      TObjArray* array = tname.Tokenize( "/" );
      TObjString* real_name = dynamic_cast< TObjString* >( array->Last() );
      tname = real_name->GetString();
      delete array;
   }

   REPORT_VERBOSE( "Looking for input metadata tree with name: " << tname );

   //
   // Look for such a metadata tree:
   //
   for( std::vector< TTree* >::const_iterator it = m_metaInputTrees.begin();
        it != m_metaInputTrees.end(); ++it ) {
      if( *it ) {
         if( tname == ( *it )->GetName() ) {
            REPORT_VERBOSE( "Found input metadata tree with name " << tname 
                            << " at " << ( *it ) );
            return *it;
         }
      }
   }

   //
   // Throw an exception if the tree hasn't been found:
   //
   SError error( SError::SkipFile );
   error << "Couldn't find input metadata TTree with name: " << tname;
   throw error;

   return 0;
}

/**
 * This function can be used to retrieve output metadata trees.
 * Output metadata trees are completely in the control of the user. Entries
 * in them are only written out on the user's request.
 *
 * @param name Name of the requested output metadata tree
 * @returns The pointer to the requested metadata tree
 */
TTree* SCycleBaseNTuple::
GetOutputMetadataTree( const char* name ) const throw( SError ) {

   //
   // Strip off the directory name from the given tree name:
   //
   TString tname( name );
   if( tname.Contains( "/" ) ) {
      REPORT_VERBOSE( "Tokenizing the metadata tree name: " << tname );
      TObjArray* array = tname.Tokenize( "/" );
      TObjString* real_name = dynamic_cast< TObjString* >( array->Last() );
      tname = real_name->GetString();
      delete array;
   }

   REPORT_VERBOSE( "Looking for output metadata tree with name: " << tname );

   //
   // Look for such a metadata tree:
   //
   for( std::vector< TTree* >::const_iterator it = m_metaOutputTrees.begin();
        it != m_metaOutputTrees.end(); ++it ) {
      if( *it ) {
         if( tname == ( *it )->GetName() ) {
            REPORT_VERBOSE( "Found output metadata tree with name " << tname 
                            << " at " << ( *it ) );
            return *it;
         }
      }
   }

   //
   // Throw an exception if the tree hasn't been found:
   //
   SError error( SError::SkipFile );
   error << "Couldn't find input metadata TTree with name: " << tname;
   throw error;

   return 0;
}

/**
 * Function used by a few of the variable handling functions. It finds
 * the tree with a given name among the input trees, or throws an exception
 * if such tree doesn't exist.
 *
 * @param treeName Name of the input TTree to look for
 * @returns A pointer to the requested input tree if successful
 */
TTree* SCycleBaseNTuple::
GetInputTree( const char* treeName ) const throw( SError ) {

   //
   // Look for such input tree:
   //
   TString tname( treeName );
   for( std::vector< TTree* >::const_iterator it = m_inputTrees.begin();
        it != m_inputTrees.end(); ++it ) {
      if( *it ) {
         if( tname == ( *it )->GetName() ) {
            REPORT_VERBOSE( "Found input tree with name " << treeName 
                            << " at " << ( *it ) );
            return *it;
         }
      }
   }

   //
   // Throw an exception if the tree hasn't been found:
   //
   SError error( SError::SkipFile );
   error << "Couldn't find input TTree with name: " << treeName;
   throw error;

   return 0;
}

/**
 * This function can be used to get direct access to one of the output trees
 * of the cycle. This has to be used for instance to pass a TTree to a
 * D3PDReader object.
 *
 * @param treeName Name of the TTree to look for
 * @return The pointer to the TTree if successful
 */
TTree* SCycleBaseNTuple::
GetOutputTree( const char* treeName ) const throw( SError ) {

   //
   // Look for such output tree:
   //
   TString tname( treeName );
   for( std::vector< TTree* >::const_iterator it = m_outputTrees.begin();
        it != m_outputTrees.end(); ++it ) {
      if( *it ) {
         if( tname == ( *it )->GetName() ) {
            REPORT_VERBOSE( "Found output tree with name " << treeName 
                            << " at " << ( *it ) );
            return *it;
         }
      }
   }

   //
   // Throw an exception if the tree hasn't been found:
   //
   SError error( SError::SkipFile );
   error << "Couldn't find output TTree with name: " << treeName;
   throw error;

   return 0;
}

/**
 * This function is used to create/access an output file on demand. For output
 * trees it can actually be known from the configuration whether an output file
 * is needed by the job, but for the in-file histogram merging, this is only
 * discovered at runtime.
 *
 * @return A pointer to the output file's directory if successful, a null
 *         pointer if not
 */
TDirectory* SCycleBaseNTuple::GetOutputFile() throw( SError ) {

   // Return right away if we already have an output file opened:
   if( m_outputFile ) return m_outputFile;

   // A possible PROOF file that is created:
   TProofOutputFile* proofFile = 0;

   // Path name of the temporary directory used in LOCAL mode:
   char* tempDirName = 0;

   // Decide what kind of output file to open:
   TNamed* out =
      dynamic_cast< TNamed* >( m_input->FindObject( SFrame::ProofOutputName ) );
   if( out ) {
      // The path name to use for the file:
      const TString path =
         gSystem->BaseName( TUrl( out->GetTitle() ).GetFile() );
      // This is not the main signature of the TProofOutputFile constructor
      // anymore, but is being kept for backwards compatibility. To avoid using
      // conditional compilation, let's use it for now. If it gets dropped,
      // we'll have to put in some #if statements around here.
      proofFile = new TProofOutputFile( path, "LOCAL" );
      proofFile->SetOutputFileName( out->GetTitle() );
      tempDirName = 0;
      m_output->Add( proofFile );
   } else {
      m_logger << DEBUG << "No PROOF output file specified in configuration -> "
               << "Running in LOCAL mode" << SLogger::endmsg;
      proofFile = 0;
      // Use a more or less POSIX method for creating a unique file name:
      tempDirName = new char[ 300 ];
      if( gSystem->Getenv( "SFRAME_TEMP_DIR" ) ) {
         // Honor the user's preference for the temporary directory
         // location:
         sprintf( tempDirName, "%s/%s",
                  gSystem->Getenv( "SFRAME_TEMP_DIR" ),
                  SFrame::ProofOutputDirName );
      } else {
         sprintf( tempDirName, "%s", SFrame::ProofOutputDirName );
      }
      if( ! mkdtemp( tempDirName ) ) {
         REPORT_FATAL( "Couldn't create temporary directory name from "
                       << "template: " << SFrame::ProofOutputDirName );
         throw SError( "Couldn't create temporary directory for output file",
                       SError::SkipCycle );
         return 0;
      }
      m_output->Add( new SOutputFile( "SFrameOutputFile",
                                      TString( tempDirName ) + "/" +
                                      SFrame::ProofOutputFileName ) );
   }

   // Now actually open the file:
   if( proofFile ) {
      if( ! ( m_outputFile = proofFile->OpenFile( "RECREATE" ) ) ) {
         m_logger << WARNING << "Couldn't open output file: "
                  << proofFile->GetDir() << "/" << proofFile->GetFileName()
                  << SLogger::endmsg;
         m_logger << WARNING << "Saving the ntuples to memory"
                  << SLogger::endmsg;
      } else {
         m_logger << DEBUG << "PROOF temp file opened with name: "
                  << m_outputFile->GetName() << SLogger::endmsg;
      }
   } else {
      if( ! tempDirName ) {
         REPORT_FATAL( "No temporary directory name? There's some serious "
                       "error in the code!" );
         throw SError( "Internal code inconsistency detected",
                       SError::SkipCycle );
         return 0;
      }

      // Open an intermediate file in this temporary directory:
      if( ! ( m_outputFile = TFile::Open( TString( tempDirName ) + "/" +
                                          SFrame::ProofOutputFileName,
                                          "RECREATE" ) ) ) {
         m_logger << WARNING << "Couldn't open output file: "
                  << tempDirName << "/" << SFrame::ProofOutputFileName
                  << SLogger::endmsg;
         m_logger << WARNING << "Saving the ntuples to memory"
                  << SLogger::endmsg;
      } else {
         m_logger << DEBUG << "LOCAL temp file opened with name: "
                  << tempDirName << "/" << SFrame::ProofOutputFileName
                  << SLogger::endmsg;
      }
   }

   // Free up the possibly allocated memory:
   if( tempDirName ) delete[] tempDirName;

   // Return the directory of the output file:
   return m_outputFile;
}

/**
 * This function is called at the end of processing an input data block, to
 * save all the output trees, and close the output file properly.
 */
void SCycleBaseNTuple::CloseOutputFile() throw( SError ) {

   // We only need to do anything if the output file has been made:
   if( m_outputFile ) {

      m_logger << DEBUG << "Closing output file: " << m_outputFile->GetName()
               << SLogger::endmsg;

      // Save all the output trees into the output file. Memory-kept TTrees
      // don't need this.
      this->SaveOutputTrees();

      // Close the output file and reset the variables:
      m_outputFile->SaveSelf( kTRUE );
      m_outputFile->Close();
      delete m_outputFile;
      m_outputFile = 0;
      m_outputTrees.clear();
      m_metaOutputTrees.clear();
   }

   return;
}

/**
 * Function called first when starting to process an InputData object.
 * It opens the output file and creates the output trees defined in the
 * cycle configuration in it. Note, that the created trees are empty,
 * no branches are added to them by default anymore.
 *
 * <strong>The function is used internally by the framework!</strong>
 *
 * @param iD       The input data that we're handling at the moment
 * @param outTrees The collection of output trees that will be created
 */
void SCycleBaseNTuple::
CreateOutputTrees( const SInputData& iD,
                   std::vector< TTree* >& outTrees ) throw( SError ) {

   // sanity checks
   if( outTrees.size() ) {
      m_logger << WARNING << "Vector of output trees is not empty in "
               << "\"CreateOutputTrees\"!"  << SLogger::endmsg;
   }

   // Clear the vector of output trees:
   m_outputTrees.clear();
   m_metaOutputTrees.clear();

   // Clear the vector of output variable pointers:
   m_outputVarPointers.clear();

   // Access all the regular output trees:
   const std::vector< STree >* sOutTree =
      iD.GetTrees( STreeType::OutputSimpleTree );

   // Make sure we're in a generic directory as a start:
   gROOT->cd();

   //
   // Create all the regular output trees, but don't create any branches in them
   // just yet.
   //
   const Int_t branchStyle = 1;
   const Int_t autoSave = 10000000;
   if( sOutTree ) {
      for( std::vector< STree >::const_iterator st = sOutTree->begin();
           st != sOutTree->end(); ++st ) {

         //
         // Split the name into the name of the tree and the name of the
         // directory:
         //
         TString tname( st->treeName ), dirname( "" );
         if( tname.Contains( "/" ) ) {
            REPORT_VERBOSE( "Tokenizing the metadata tree name: " << tname );
            TObjArray* array = tname.Tokenize( "/" );
            TObjString* real_name =
            dynamic_cast< TObjString* >( array->Last() );
            tname = real_name->GetString();
            if( array->GetSize() > 1 ) {
               for( Int_t i = 0; i < array->GetSize() - 1; ++i ) {
                  TObjString* dirletname =
                  dynamic_cast< TObjString* >( array->At( i ) );
                  if( ! dirletname ) continue;
                  if( dirletname->GetString() == "" ) continue;
                  if( dirletname->GetString() == tname ) break;
                  dirname += ( dirname == "" ? dirletname->GetString() :
                               "/" + dirletname->GetString() );
               }
            }
            delete array;
         }

         m_logger << DEBUG << "Creating output event tree with name: "
                  << tname << " in directory: \"" << dirname << "\""
                  << SLogger::endmsg;

         // Create the output TTree:
         TTree* tree = new TTree( tname,
                                  TString( "Format: User" ) +
                                  ", data type: " + iD.GetType() );
         tree->SetAutoSave( autoSave );
         TTree::SetBranchStyle( branchStyle );

         // Store the pointer:
         outTrees.push_back( tree );
         m_outputTrees.push_back( tree );

         // Make sure that an output file is available:
         GetOutputFile();

         // Add it to the output file if available:
         if( m_outputFile ) {
            tree->SetDirectory( MakeSubDirectory( dirname, m_outputFile ) );
            REPORT_VERBOSE( "Attached TTree \"" << st->treeName.Data()
                            << "\" to file: " << m_outputFile->GetName() );
         } else {
            SCycleOutput* out = new SCycleOutput( tree, tname, dirname );
            m_output->Add( out );
            REPORT_VERBOSE( "Keeping TTree \"" << tname
                            << "\" in memory" );
         }
      }
   }

   //
   // Create the metadata output trees. These don't have to be reported back to
   // the caller, since the user will be responsible for filling them.
   //
   const std::vector< STree >* sMetaTrees =
      iD.GetTrees( STreeType::OutputMetaTree );
   if( sMetaTrees ) {
      for( std::vector< STree >::const_iterator mt = sMetaTrees->begin();
           mt != sMetaTrees->end(); ++mt ) {

         //
         // Split the name into the name of the tree and the name of the
         // directory:
         //
         TString tname( mt->treeName ), dirname( "" );
         if( tname.Contains( "/" ) ) {
            REPORT_VERBOSE( "Tokenizing the metadata tree name: " << tname );
            TObjArray* array = tname.Tokenize( "/" );
            TObjString* real_name =
               dynamic_cast< TObjString* >( array->Last() );
            tname = real_name->GetString();
            if( array->GetSize() > 1 ) {
               for( Int_t i = 0; i < array->GetSize() - 1; ++i ) {
                  TObjString* dirletname =
                     dynamic_cast< TObjString* >( array->At( i ) );
                  if( ! dirletname ) continue;
                  if( dirletname->GetString() == "" ) continue;
                  if( dirletname->GetString() == tname ) break;
                  dirname += ( dirname == "" ? dirletname->GetString() :
                               "/" + dirletname->GetString() );
               }
            }
            delete array;
         }

         m_logger << DEBUG << "Creating output metadata tree with name: "
                  << tname  << " in directory: \"" << dirname << "\""
                  << SLogger::endmsg;

         // Create the metadata tree:
         TTree* tree = new TTree( tname, TString( "Format: User" ) +
                                  ", data type: " + iD.GetType() );
         tree->SetAutoSave( autoSave );
         TTree::SetBranchStyle( branchStyle );

         // Remember its pointer:
         m_metaOutputTrees.push_back( tree );

         // Make sure that an output file is available:
         GetOutputFile();

         // Add it to the output file if available:
         if( m_outputFile ) {
            tree->SetDirectory( MakeSubDirectory( dirname, m_outputFile ) );
            REPORT_VERBOSE( "Attached TTree \"" << mt->treeName
                            << "\" to file: " << m_outputFile->GetName() );
         } else {
            SCycleOutput* out = new SCycleOutput( tree, tname, dirname );
            m_output->Add( out );
            REPORT_VERBOSE( "Keeping TTree \"" << mt->treeName
                            << "\" in memory" );
         }
      }
   }

   return;
}

/**
 * This function is used to save all the output trees into the output file when
 * the cycle finishes processing the events from the InputData. It also
 * deletes the TTree objects, so it should really only be called by the
 * framework at the end of processing.
 */
void SCycleBaseNTuple::SaveOutputTrees() throw( SError ) {

   // Remember which directory we were in:
   TDirectory* savedir = gDirectory;

   // Flag stating whether we're running using PROOF:
   const Bool_t isProof = m_input->FindObject( SFrame::ProofOutputName );

   // Save each regular output tree:
   for( std::vector< TTree* >::iterator tree = m_outputTrees.begin();
        tree != m_outputTrees.end(); ++tree ) {
      // Only save the tree if it has entries, or we're not on PROOF:
      if( ( *tree )->GetEntries() || ( ! isProof ) ) {
         TDirectory* dir = ( *tree )->GetDirectory();
         if( dir ) dir->cd();
         ( *tree )->Write();
         ( *tree )->AutoSave();
      } else {
         m_logger << INFO << "Not saving TTree \"" << ( *tree )->GetName()
                  << "\", because it is empty" << SLogger::endmsg;
      }
      // Delete the tree:
      ( *tree )->SetDirectory( 0 );
      delete ( *tree );
   }

   // Save each metadata output tree:
   for( std::vector< TTree* >::iterator tree = m_metaOutputTrees.begin();
        tree != m_metaOutputTrees.end(); ++tree ) {
      // Only save the tree if it has entries, or we're not on PROOF:
      if( ( *tree )->GetEntries() || ( ! isProof ) ) {
         TDirectory* dir = ( *tree )->GetDirectory();
         if( dir ) dir->cd();
         ( *tree )->Write();
         ( *tree )->AutoSave();
      } else {
         m_logger << INFO << "Not saving TTree \"" << ( *tree )->GetName()
                  << "\", because it is empty" << SLogger::endmsg;
      }
      // Delete the tree:
      ( *tree )->SetDirectory( 0 );
      delete ( *tree );
   }

   // Go back to the original directory:
   gDirectory = savedir;

   return;
}

/**
 * Function called first for each new input file. It opens the file, and
 * accesses the trees defined in the cycle configuration. It also starts the
 * book-keeping for the EventView input trees, if such things are defined.
 *
 * <strong>The function is used internally by the framework!</strong>
 *
 * @param iD       The input data that we're handling at the moment
 * @param main_tree Pointer to the main input TTree
 * @param inputFile Pointer to the input file created by the function (output)
 */
void SCycleBaseNTuple::
LoadInputTrees( const SInputData& iD,
                TTree* main_tree,
                TDirectory*& inputFile ) throw( SError ) {

   REPORT_VERBOSE( "Loading/accessing the event-level input trees" );

   //
   // Initialize some variables:
   //
   const std::vector< STree >* sInTree =
      iD.GetTrees( STreeType::InputSimpleTree );
   const std::vector< STree >* sMetaTree =
      iD.GetTrees( STreeType::InputMetaTree );
   Bool_t firstPassed = kFALSE;
   Long64_t nEvents = 0;
   m_inputTrees.clear();
   m_inputBranches.clear();
   DeleteInputVariables();
   m_metaInputTrees.clear();

   //
   // Access the physical file that is currently being opened:
   //
   inputFile = 0;
   if( GetConfig().GetRunMode() == SCycleConfig::LOCAL ) {
      TChain* chain = dynamic_cast< TChain* >( main_tree );
      if( ! chain ) {
         throw SError( "In LOCAL running the input TTree is not a TChain!",
                       SError::StopExecution );
      }
      inputFile = chain->GetFile();
   } else if( GetConfig().GetRunMode() == SCycleConfig::PROOF ) {
      inputFile = main_tree->GetCurrentFile();
   } else {
      throw SError( "Running mode not recongnised", SError::SkipCycle );
      return;
   }
   if( ! inputFile ) {
      throw SError( "Couldn't get the input file pointer!", SError::SkipFile );
      return;
   }
   REPORT_VERBOSE( "Accessed the pointer to the input file: " << inputFile );

   //
   // Handle the regular input trees:
   //
   if( sInTree ) {
      std::vector< STree >::const_iterator st_itr = sInTree->begin();
      std::vector< STree >::const_iterator st_end = sInTree->end();
      for( ; st_itr != st_end; ++st_itr ) {

         REPORT_VERBOSE( "Now trying to access TTree: " << st_itr->treeName );

         TTree* tree =
            dynamic_cast< TTree* >( inputFile->Get( st_itr->treeName ) );
         if( ! tree ) {
            SError error( SError::SkipFile );
            error << "Tree " << st_itr->treeName << " doesn't exist in File "
                  << inputFile->GetName();
            throw error;
         }

         // do we need this at all now that we loop over the branches
         // one-by-one?
         
         // Remove friends if any, for better performance
         bool skipFriends = true; // can be made configurable
         if( skipFriends ) {
            TList* flist = tree->GetListOfFriends();
            TIter nextf( flist );
            TFriendElement* fe = 0;
            while( ( fe = ( TFriendElement* ) nextf() ) ) {
               m_logger << DEBUG << "Remove friend " << fe->GetName()
                        << " from tree " << tree->GetName() << SLogger::endmsg;
               flist->Remove( fe );
               delete fe;
               fe = 0;
            }
         }
         // Delete index if any, for better performance
         bool deleteIndex = true; // can be made configurable
         if( deleteIndex ) {
            if( tree->GetTreeIndex() ) {
               m_logger << DEBUG << "Delete index from tree " 
                        << tree->GetName() << SLogger::endmsg;
               tree->SetTreeIndex( 0 );
               delete tree->GetTreeIndex();
            }
         }

         m_inputTrees.push_back( tree );
         if( firstPassed && tree->GetEntries() != nEvents ) {
            SError error( SError::SkipFile );
            error << "Conflict in number of entries - Tree " << tree->GetName()
                  << " has " << tree->GetEntries() << ", NOT "
                  << nEvents;
            throw error;
         } else if( ! firstPassed ) {
            firstPassed = kTRUE;
            nEvents = tree->GetEntries();
         }
      }
   }

   //
   // Handle the metadata trees:
   //
   if( sMetaTree ) {
      for( std::vector< STree >::const_iterator mt = sMetaTree->begin();
           mt != sMetaTree->end(); ++mt ) {

         TTree* tree = dynamic_cast< TTree* >( inputFile->Get( mt->treeName ) );
         if( ! tree ) {
            SError error( SError::SkipFile );
            error << "Tree " << mt->treeName << " doesn't exist in File "
                  << inputFile->GetName();
            throw error;
         } else {
            m_metaInputTrees.push_back( tree );
         }
      }
   }

   return;
}

/**
 * Function reading in the same entry for each of the connected branches.
 * It is called first for each new event.
 *
 * <strong>The function is used internally by the framework!</strong>
 *
 * @param entry The event number to read in
 */
void SCycleBaseNTuple::GetEvent( Long64_t entry ) throw( SError ) {

   // Tell all trees to update their cache:
   for( std::vector< TTree* >::const_iterator it = m_inputTrees.begin();
        it != m_inputTrees.end(); ++it ) {
      ( *it )->LoadTree( entry );
   }

   // Load the current entry for all the regular input variables:
   for( std::vector< TBranch* >::const_iterator it = m_inputBranches.begin();
        it != m_inputBranches.end(); ++it ) {
      ( *it )->GetEntry( entry );
   }

   return;
}

/**
 * Function calculating the event weight for the MC event for each event.
 *
 * <strong>The function is used internally by the framework!</strong>
 *
 * @param inputData The input data that we're processing at the moment
 * @param entry     The event number
 */
Double_t SCycleBaseNTuple::CalculateWeight( const SInputData& inputData,
                                            Long64_t entry ) const {

   // the type of this input data
   const TString& type    = inputData.GetType();
   const TString& version = inputData.GetVersion();

   Double_t weight = 0.;
   Double_t totlum = 0.;

   // Data events always have a weight of 1.0:
   if( inputData.GetType() == "data" ) {
      return 1.0;
   }

   // Iterate over vector of input data and addup the weight for the type of
   // this input data:
   std::vector< SInputData >::const_iterator id_itr =
      GetConfig().GetInputData().begin();
   std::vector< SInputData >::const_iterator id_end =
      GetConfig().GetInputData().end();
   for( ; id_itr != id_end; ++id_itr ) {

      if( ( id_itr->GetType() == type ) &&
          ( id_itr->GetVersion() == version ) ) {

         const std::vector< SGeneratorCut >& sgencuts =
            id_itr->GetSGeneratorCuts();
         Bool_t inside = kTRUE;

         std::vector< SGeneratorCut >::const_iterator gc_itr = sgencuts.begin();
         std::vector< SGeneratorCut >::const_iterator gc_end = sgencuts.end();
         for( ; gc_itr != gc_end; ++gc_itr ) {

            // loop over the trees
            std::vector< TTree* >::const_iterator tree_itr =
               m_inputTrees.begin();
            std::vector< TTree* >::const_iterator tree_end =
               m_inputTrees.end();
            for( ; tree_itr != tree_end; ++tree_itr ) {

               // consider the one with the correct name
               if( ( *tree_itr )->GetName() == gc_itr->GetTreeName() ) {
                  // check for this entry, if Formula is true
                  TTreeFormula f( "testFormula", gc_itr->GetFormula().Data(),
                                  *tree_itr );
                  // if true for all cuts, then add the Lumi of this input data
                  // to totlum
                  if( ! f.EvalInstance( static_cast< Int_t >( entry ) ) ) {
                     inside = kFALSE;
                  }
                  break;
               }

            }
         }
         if( inside ) totlum += id_itr->GetScaledLumi();
      }
   }

   // Check that the total luminosity is not zero:
   if( totlum > 1e-15 ) {
      weight = ( GetConfig().GetTargetLumi() / totlum );
   }

   return weight;
}

/**
 * This function instructs the object to forget about all the TTree pointers
 * that it collected at the beginning of executing the cycle. It's a security
 * measure for when the cycle is run multiple times.
 */
void SCycleBaseNTuple::ClearCachedTrees() {

   m_inputTrees.clear();
   m_inputBranches.clear();
   m_outputTrees.clear();
   m_metaInputTrees.clear();
   m_metaOutputTrees.clear();

   DeleteInputVariables();

   return;
}

/**
 * This is a tricky one. In SCycleBaseNTuple::DeclareVariable(...) the function
 * automatically detects the type of the variable to be put into the output
 * tree. Since ROOT uses a different naming scheme for the primitives than C++'s
 * typeid call, the typeid names have to be translated for ROOT. This is the
 * function doing that.
 *
 * @warning The translation is probably only valid on various UNIX systems,
 *          probably doesn't work on Windows. (Did anyone ever try SFrame on
 *          Windows anyway???)
 *
 * @param typeid_type Primitive type name in the "typeid" format
 * @returns The primitive type name in ROOT's format
 */
const char*
SCycleBaseNTuple::RootType( const char* typeid_type ) throw( SError ) {

   // Check that we received a reasonable input:
   if( strlen( typeid_type ) != 1 ) {
      SLogger m_logger( "SCycleBaseNTuple" );
      REPORT_ERROR( "Received a complex object description: " << typeid_type );
      throw SError( "SCycleBaseNTuple::RootType received complex object "
                    "description", SError::StopExecution );
   }

   // Do the hard-coded translation:
   switch( typeid_type[ 0 ] ) {

   case 'c':
      return "B";
      break;
   case 'h':
      return "b";
      break;
   case 's':
      return "S";
      break;
   case 't':
      return "s";
      break;
   case 'i':
      return "I";
      break;
   case 'j':
      return "i";
      break;
   case 'f':
      return "F";
      break;
   case 'd':
      return "D";
      break;
   case 'x':
      return "L";
      break;
   case 'y':
      return "l";
      break;
   case 'b':
      return "O";
      break;

   }

   // This is a pretty surprising turn of events:
   SLogger m_logger( "SCycleBaseNTuple" );
   REPORT_ERROR( "Unknown primitive type encountered: " << typeid_type );
   throw SError( "Unknown primitive type encountered: " +
                 TString( typeid_type ), SError::StopExecution );
   return "";
}

/**
 * This function is used internally to check whether the user tries to connect
 * the correct type of primitive to the branches. ROOT can have some trouble
 * identifying such code problems...
 *
 * @warning The implementation might be platform specific!
 *
 * @param root_type ROOT primitive type name
 * @returns The typeid type name for the ROOT primitive type
 */
const char*
SCycleBaseNTuple::TypeidType( const char* root_type ) throw( SError ) {

   // Do the hard-coded translation:
   if( ! strcmp( root_type, "Char_t" ) ) {
      return "c";
   } else if( ! strcmp( root_type, "UChar_t" ) ) {
      return "h";
   } else if( ! strcmp( root_type, "Short_t" ) ) {
      return "s";
   } else if( ! strcmp( root_type, "UShort_t" ) ) {
      return "t";
   } else if( ! strcmp( root_type, "Int_t" ) ) {
      return "i";
   } else if( ! strcmp( root_type, "UInt_t" ) ) {
      return "j";
   } else if( ! strcmp( root_type, "Float_t" ) ) {
      return "f";
   } else if( ! strcmp( root_type, "Double_t" ) ) {
      return "d";
   } else if( ! strcmp( root_type, "Long64_t" ) ) {
      return "x";
   } else if( ! strcmp( root_type, "ULong64_t" ) ) {
      return "y";
   } else if( ! strcmp( root_type, "Bool_t" ) ) {
      return "b";
   }

   // This is a pretty surprising turn of events:
   SLogger m_logger( "SCycleBaseNTuple" );
   REPORT_ERROR( "Unknown ROOT primitive type encountered: " << root_type );
   throw SError( "Unknown ROOT primitive type encountered: " +
                 TString( root_type ), SError::StopExecution );
   return "";
}

/**
 * Helper function filling the list of input branches. It is called by the main
 * variable handling functions, not directly by the user.
 *
 * @param br The branch to remember
 */
void SCycleBaseNTuple::RegisterInputBranch( TBranch* br ) throw( SError ) {

   // This is a bit slow, but still not the worst part of the code...
   if( std::find( m_inputBranches.begin(), m_inputBranches.end(), br ) !=
       m_inputBranches.end() ) {
      m_logger << DEBUG << "Branch '" << br->GetName()
               << "' already registered!" << SLogger::endmsg;
   } else {
      m_inputBranches.push_back( br );
   }

   // Return gracefully:
   return;
}

/**
 * This function deletes the contents of the input variable list. Since the
 * SPointer objects in the list know exactly what kind of object they point to
 * (templating rules...), they're able to delete them when they get deleted.
 * (Even though at this point we delete them through their TObject "interface".)
 */
void SCycleBaseNTuple::DeleteInputVariables() {

   for( std::list< TObject* >::iterator it = m_inputVarPointers.begin();
        it != m_inputVarPointers.end(); ++it ) {
      delete ( *it );
   }
   m_inputVarPointers.clear();

   return;
}

/**
 * This function can create a sub-directory inside an existing directory (a file
 * for instance). It's used to make directories for output trees.
 *
 * @param path The path name that should be created
 * @param dir The parent directory for the sub-directory (structure)
 * @returns A pointer to the inner-most directory
 */
TDirectory*
SCycleBaseNTuple::MakeSubDirectory( const TString& path,
                                    TDirectory* dir ) const throw( SError ) {

   // Return the parent directory if the path name is empty:
   if( ! path.Length() ) return dir;

   TDirectory* result = 0;
   if( ! ( result = dir->GetDirectory( path ) ) ) {

      REPORT_VERBOSE( "Creating directory: "
                      << dir->GetPath() << "/" << path );

      //
      // Break up the path name at the slashes:
      //
      TObjArray* directories = path.Tokenize( "/" );

      //
      // Create each necessary directory:
      //
      result = dir;
      TDirectory* tempDir = 0;
      for( Int_t i = 0; i < directories->GetSize(); ++i ) {

         TObjString* path_element =
            dynamic_cast< TObjString* >( directories->At( i ) );
         if( ! path_element ) continue;
         if( path_element->GetString() == "" ) continue;

         REPORT_VERBOSE( "Accessing directory: " << path_element->GetString() );
         tempDir = result->GetDirectory( path_element->GetString() );
         if( ! tempDir ) {
            REPORT_VERBOSE( "Directory doesn't exist, creating it..." );
            tempDir = result->mkdir( path_element->GetString(), "dummy title" );
            if( ! tempDir ) {
               REPORT_ERROR( "Couldn't create directory: " << path
                             << " in the output file!" );
               SError error( SError::SkipInputData );
               error << "Couldn't create directory: " << path
                     << " in the output file!";
               throw error;
            }
         }
         result = tempDir;

      }

      // Delete the object created by TString::Tokenize(...):
      delete directories;

   }

   // Return the created directory:
   return result;
}
