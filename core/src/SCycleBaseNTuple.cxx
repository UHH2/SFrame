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

// Local include(s):
#include "../include/SCycleBaseNTuple.h"
#include "../include/SInputData.h"
#include "../include/SCycleConfig.h"
#include "../include/SCycleOutput.h"
#include "../include/STreeType.h"

static const Double_t EPSILON = 1e-15;

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseNTuple );
#endif // DOXYGEN_IGNORE

/**
 * The constructor is only initialising the base class.
 */
SCycleBaseNTuple::SCycleBaseNTuple()
   : SCycleBaseBase(), m_inputTrees(), m_inputBranches(),
     m_outputTrees(), m_metaInputTrees(), m_outputVarPointers(), m_output( 0 ) {

   REPORT_VERBOSE( "SCycleBaseNTuple constructed" );
}

/**
 * Another one of the "I don't do anything" destructors.
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

/**
 * This function can be used to retrieve both input and output metadata trees.
 * Input metadata trees are TTree-s that don't desribe event level information.
 * The reading of these trees is completely up to the user at this point.
 *
 * Output metadata trees are also completely in the control of the user. Entries
 * in them are only written out on the users request.
 *
 * @param name Name of the requested metadata tree
 * @returns The pointer to the requested metadata tree
 */
TTree* SCycleBaseNTuple::GetMetadataTree( const char* name ) const throw( SError ) {

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

   m_logger << DEBUG << "Looking for metadata tree with name: " << tname << SLogger::endmsg;

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
 */
TTree* SCycleBaseNTuple::GetInputTree( const char* treeName ) const throw( SError ) {

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

TTree* SCycleBaseNTuple::GetOutputTree( const char* treeName ) const throw( SError ) {

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
void SCycleBaseNTuple::CreateOutputTrees( const SInputData& iD,
                                          std::vector< TTree* >& outTrees,
                                          TFile* outputFile ) throw( SError ) {

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

   const std::vector< STree >* sOutTree = iD.GetTrees( STreeType::OutputSimpleTree );

   // Open output file / create output trees
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

         m_logger << DEBUG << "Creating output event tree with name: "
                  << st->treeName << SLogger::endmsg;

         TTree* tree = new TTree( st->treeName.Data(), TString( "Format: User" ) +
                                  ", data type: " + iD.GetType() );

         tree->SetAutoSave( autoSave );
         TTree::SetBranchStyle( branchStyle );

         outTrees.push_back( tree );
         m_outputTrees.push_back( tree );

         if( outputFile ) {
            tree->SetDirectory( outputFile );
            REPORT_VERBOSE( "Attached TTree \"" << st->treeName.Data()
                            << "\" to file: " << outputFile->GetName() );
         } else {
            SCycleOutput* out = new SCycleOutput( tree, st->treeName );
            m_output->Add( out );
            REPORT_VERBOSE( "Keeping TTree \"" << st->treeName.Data()
                            << "\" in memory" );
         }
      }
   }

   //
   // Create the metadata output trees. These don't have to be reported back to the
   // caller, since the user will be responsible for filling them.
   //
   const std::vector< STree >* sMetaTrees = iD.GetTrees( STreeType::OutputMetaTree );
   if( sMetaTrees ) {
      for( std::vector< STree >::const_iterator mt = sMetaTrees->begin();
           mt != sMetaTrees->end(); ++mt ) {

         m_logger << DEBUG << "Creating output metadata tree with name: "
                  << mt->treeName << SLogger::endmsg;

         TTree* tree = new TTree( mt->treeName, TString( "Format: User" ) +
                                  ", data type: " + iD.GetType() );

         tree->SetAutoSave( autoSave );
         TTree::SetBranchStyle( branchStyle );

         m_metaOutputTrees.push_back( tree );

         if( outputFile ) {
            tree->SetDirectory( outputFile );
            REPORT_VERBOSE( "Attached TTree \"" << mt->treeName
                            << "\" to file: " << outputFile->GetName() );
         } else {
            SCycleOutput* out = new SCycleOutput( tree, mt->treeName );
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
 * deletes the TTree objects, so it should really only be called by the framework
 * at the end of processing.
 *
 * @param output The directory where the trees have to be written (Usually a TFile)
 */
void SCycleBaseNTuple::SaveOutputTrees( TDirectory* output ) throw( SError ) {

   // Remember which directory we were in:
   TDirectory* savedir = gDirectory;
   output->cd();

   // Save each regular output tree:
   for( std::vector< TTree* >::iterator tree = m_outputTrees.begin();
        tree != m_outputTrees.end(); ++tree ) {
      ( *tree )->Write();
      ( *tree )->SetDirectory( 0 );
      delete ( *tree );
   }

   // Save each metadata output tree:
   for( std::vector< TTree* >::iterator tree = m_metaOutputTrees.begin();
        tree != m_metaOutputTrees.end(); ++tree ) {
      ( *tree )->Write();
      ( *tree )->SetDirectory( 0 );
      delete ( *tree );
   }

   // Go back to the original directory:
   gDirectory = savedir;

   return;
}

/**
 * Function called first for each new input file. It opens the file, and accesses
 * the trees defined in the cycle configuration. It also starts the book-keeping
 * for the EventView input trees, if such things are defined.
 *
 * <strong>The function is used internally by the framework!</strong>
 *
 * @param iD       The input data that we're handling at the moment
 * @param filename The full name of the input file
 * @param file     Pointer to the input file that the function opens
 */
void SCycleBaseNTuple::LoadInputTrees( const SInputData& iD,
                                       TTree* main_tree,
                                       TFile*& inputFile ) throw( SError ) {

   REPORT_VERBOSE( "Loading/accessing the event-level input trees" );

   //
   // Initialize some variables:
   //
   const std::vector< STree >* sInTree = iD.GetTrees( STreeType::InputSimpleTree );
   const std::vector< STree >* sMetaTree = iD.GetTrees( STreeType::InputMetaTree );
   Bool_t firstPassed = kFALSE;
   Int_t nEvents = 0;
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
      for( std::vector< STree >::const_iterator st = sInTree->begin(); st != sInTree->end();
           ++st ) {

         REPORT_VERBOSE( "Now trying to access TTree: " << st->treeName );

         TTree* tree = dynamic_cast< TTree* >( inputFile->Get( st->treeName ) );
         if( ! tree ) {
            SError error( SError::SkipFile );
            error << "Tree " << st->treeName << " doesn't exist in File "
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
               m_logger << DEBUG << "Remove friend " << fe->GetName() << " from tree " 
                        << tree->GetName() << SLogger::endmsg;
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

         // Disable reading all the branches. (We do it explicitly by hand.)
         tree->SetBranchStatus( "*", 0 );

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
 * Function telling the input trees that all branches that need to be cached, have
 * now been declared. The code will not try to be smart about the caching from
 * here on, but just cache all the branches that were explicitly requested.
 *
 * <strong>The function is used internally by the framework!</strong>
 */
void SCycleBaseNTuple::SetInputCacheConfigured() throw( SError ) {

   // This feature is only available in new ROOT versions...
#if ROOT_VERSION_CODE >= ROOT_VERSION( 5, 26, 0 )
   // Tell all input trees that their cache is now configured:
   for( std::vector< TTree* >::const_iterator it = m_inputTrees.begin();
        it != m_inputTrees.end(); ++it ) {
      ( *it )->StopCacheLearningPhase();
   }
#endif // ROOT_VERSION...

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
                                            Long64_t entry ) {

   // the type of this input data
   const TString& type    = inputData.GetType();
   const TString& version = inputData.GetVersion();

   Double_t weight = 0.;
   Double_t totlum = 0.;

   if( inputData.GetType() == "data" ) {
      REPORT_VERBOSE( "Data" );
      weight = 1.;
      return weight;
   }

   //iterate over vector of input data and addup the weight for the type of this input data
   for( std::vector< SInputData >::const_iterator iD = GetConfig().GetInputData().begin();
        iD != GetConfig().GetInputData().end(); ++iD ) {

      if( ( iD->GetType() == type ) && ( iD->GetVersion() == version ) ) {

         const std::vector< SGeneratorCut >& sgencuts = iD->GetSGeneratorCuts();
         Bool_t inside = kTRUE;

         for( std::vector< SGeneratorCut >::const_iterator sgc = sgencuts.begin();
              sgc != sgencuts.end(); ++sgc ) {
			 
            // loop over the trees 
            for( std::vector< TTree* >::const_iterator it = m_inputTrees.begin();
                 it != m_inputTrees.end(); ++it ) {

               // consider the one with the correct name
               if( ( *it )->GetName() == sgc->GetTreeName() ) {
                  // check for this entry, if Formula is true	
                  TString teststring = sgc->GetFormula();
                  TTreeFormula f( "testFormula", teststring.Data(), *it );
                  // if true for all cuts, then add the Lumi of this input data to totlum
                  if( ! f.EvalInstance( entry ) ) inside = kFALSE;
                  break;
               }

            }
         }
         if( inside ) totlum += iD->GetScaledLumi();
      }
   }

   if( totlum > EPSILON ) 
      weight = ( GetConfig().GetTargetLumi() / totlum );
  
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
 * automatically detects the type of the variable to be put into the output tree.
 * Since ROOT uses a different naming scheme for the primitives than C++'s typeid
 * call, the typeid names have to be translated for ROOT. This is the function
 * doing that.
 *
 * Note: The translation is probably only valid on various UNIX systems, probably
 * doesn't work on Windows. (Did anyone ever try SFrame on Windows anyway???)
 */
const char* SCycleBaseNTuple::RootType( const char* typeid_type ) throw( SError ) {

   if( strlen( typeid_type ) != 1 ) {
      throw SError( "SCycleBaseNTuple::RootType received complex object description",
                    SError::StopExecution );
   }

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

   throw SError( "Unknown primitive type encountered: " + TString( typeid_type ),
                 SError::StopExecution );
   return "";
}

/**
 * This function is used internally to check whether the user tries to connect
 * the correct type of primitive to the branches. ROOT can have some trouble
 * identifying such code problems...
 *
 * Note: The implementation might be platform specific!
 */
const char* SCycleBaseNTuple::TypeidType( const char* root_type ) throw( SError ) {

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

   throw SError( "Unknown ROOT primitive type encountered: " + TString( root_type ),
                 SError::StopExecution );
   return "";
}

/**
 * Helper function filling the list of input branches. It is called by the main
 * variable handling functions, not directly by the user.
 */
void SCycleBaseNTuple::RegisterInputBranch( TBranch* br ) throw( SError ) {

   if( std::find( m_inputBranches.begin(), m_inputBranches.end(), br ) !=
       m_inputBranches.end() ) {
      m_logger << DEBUG << "Branch '" << br->GetName() << "' already registered!"
               << SLogger::endmsg;
   } else {
      m_inputBranches.push_back( br );
   }

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
