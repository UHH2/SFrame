// $Id: SCycleBaseNTuple.cxx,v 1.3 2008-01-25 14:33:54 krasznaa Exp $
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
#include <TBranch.h>
#include <TROOT.h>
#include <TList.h>
#include <TFriendElement.h>
#include <TVirtualIndex.h>
#include <TTreeFormula.h>

// Local include(s):
#include "../include/SCycleBaseNTuple.h"
#include "../include/SInputData.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseNTuple );
#endif // DOXYGEN_IGNORE

using namespace std;

static Double_t EPSILON = 1e-15;

/**
 * The constructor is only initialising the base class.
 */
SCycleBaseNTuple::SCycleBaseNTuple()
   : SCycleBaseConfig() {

   m_logger << VERBOSE << "SCycleBaseNTuple constructed" << SLogger::endmsg;

}

/**
 * Another one of the "I don't do anything" destructors.
 */
SCycleBaseNTuple::~SCycleBaseNTuple() {

   m_logger << VERBOSE << "SCycleBaseNTuple destructed" << SLogger::endmsg;

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
 * @param fileOut  Pointer to the output file that the function opens
 */
void SCycleBaseNTuple::CreateOutputTrees( const SInputData& iD,
                                          std::vector< TTree* >& outTrees,
                                          TFile*& fileOut ) throw( SError ) {

   // sanity checks
   if( outTrees.size() )
      m_logger << WARNING << "Vector of output trees is not empty in \"CreateOutputTrees\"!" 
               << SLogger::endmsg;
   if( fileOut )
      m_logger << WARNING << "Pointer to output file is not 0 in \"CreateOutputTrees\"!" 
               << SLogger::endmsg;

   // Clear the vector of output trees:
   m_outputTrees.clear();

   // Clear the vector of output variable pointers:
   m_outputVarPointers.clear();

   const std::vector< STree >& sOutTree = iD.GetOutputTrees();
   // Open output file / create output trees
   gROOT->cd();

   // Construct output file name
   m_outputFileName = GetOutputDirectory() + GetName() + "." + iD.GetType() +
      GetPostFix() + ".root";
   // Replace "::" by "." to make nice file names for cycles defined
   // in a namespace:
   m_outputFileName.ReplaceAll( "::", "." );

   m_logger << INFO << "Opening: "<< m_outputFileName << SLogger::endmsg;

   fileOut = new TFile( m_outputFileName, "RECREATE" );
   if( ! fileOut || fileOut->IsZombie() ) {
      SError error( SError::SkipInputData );
      error << "Cannot open output file " << m_outputFileName;
      throw error;
   }

   //
   // Create all the output trees, but don't create any branches in them just yet.
   //
   const Int_t branchStyle = 1;
   const Int_t autoSave = 10000000;
   for( std::vector< STree >::const_iterator st = sOutTree.begin();
        st != sOutTree.end(); ++st ) {

      TTree* tree = new TTree( st->treeName.Data(), TString( "Format: User" ) +
                               ", data type: " + iD.GetType() );

      tree->SetAutoSave( autoSave );
      tree->SetDirectory( fileOut );
      TTree::SetBranchStyle( branchStyle );

      outTrees.push_back( tree );
      m_outputTrees.push_back( tree );
   }

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
void SCycleBaseNTuple::LoadInputTrees( const SInputData& iD, const std::string& filename,
                                       TFile*& file ) throw( SError ) {

   m_logger << INFO << "Opening: " << filename << SLogger::endmsg;
   // If you want to read files from castor...
   // if(filename.find("rfio:") != std::string::npos) file = new TRFIOFile( filename.c_str(), "READ" );
   // else file = new TFile( filename.c_str(), "READ" );

   file = this->OpenInputFile( filename.c_str() ); // This will throw an exception if unsuccessful...

   const std::vector< STree >& sInTree = iD.GetInputTrees();
   Bool_t firstPassed = kFALSE;
   m_nEvents = 0;
   m_inputTrees.clear();
   m_inputBranches.clear();

   for( vector< STree >::const_iterator st = sInTree.begin(); st != sInTree.end(); ++st ) {

      TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
      if( ! tree ) {
         SError error( SError::SkipFile );
         error << "Tree " << st->treeName << " doesn't exist in File "
               << file->GetName();
         throw error;
      } else {

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

         m_inputTrees.push_back( tree );
         if( firstPassed && tree->GetEntries() != m_nEvents ) {
            SError error( SError::SkipFile );
            error << "Conflict in number of entries - Tree " << tree->GetName()
                  << " has " << tree->GetEntries() << ", NOT "
                  << m_nEvents;
            throw error;
         } else if( ! firstPassed ) {
            firstPassed = kTRUE;
            m_nEvents = tree->GetEntries();
         }
      }
   }

   //
   // Now initialise the EV trees, if they're defined
   //
   m_EVinputTrees.clear();
   m_EVInTreeToCounters.clear();
   m_EVInTreeToCollTreeName.clear();
   m_EVInTreeToBaseName.clear();
   m_EVInTreeToViewNumber.clear();
   m_EVInputBranchesToBaseName.clear();
   m_EVInputBranchesToViewNumber.clear();

   const std::vector< SEVTree >& sEVInTree = iD.GetEVInputTrees();
   for( vector< SEVTree >::const_iterator st = sEVInTree.begin();
        st != sEVInTree.end(); ++st ) {
      TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
      if( ! tree ) {
         SError error( SError::SkipFile );
         error << "Tree " << st->treeName << " doesn't exist in File "
               << file->GetName();
         throw error;
      } else {
         // store pointers to EventView input trees, and initialise one
         // counter for each tree for the bookkeeping
         m_EVinputTrees.push_back( tree );
         m_EVInTreeToCounters[ tree ] = 0;
         m_EVInTreeToCollTreeName[ tree ] = st->collTreeName;
         m_EVInTreeToBaseName[ tree ] = st->treeBaseName;
         m_EVInTreeToViewNumber[ tree ] = st->viewNumber;
      }
   }

   //
   // Finally finish the EV tree initialisation by connecting the
   // synchronisation variables:
   //
   this->ConnectEVSyncVariable();

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
void SCycleBaseNTuple::GetEntry( Long64_t entry ) throw( SError ) {

   // Load the current entry for all the regular input variables:
   for( vector< TBranch* >::const_iterator it = m_inputBranches.begin();
        it != m_inputBranches.end(); ++it ) {
      ( *it )->GetEntry( entry );
   }

   // Now synchronise the EV input trees:
   this->SyncEVTrees();

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
Double_t SCycleBaseNTuple::CalculateWeight( const SInputData& inputData,  Long64_t entry ) {

   // the type of this input data
   TString type = inputData.GetType();

   Double_t weight = 0.;
   Double_t totlum = 0.;

   if( inputData.GetType() == "data" ) {
      m_logger << DEBUG << "Data" << SLogger::endmsg;
      weight = 1.;
      return weight;
   }

   //iterate over vector of input data and addup the weight for the type of this input data
   for( vector< SInputData >::const_iterator iD = m_inputData.begin();
        iD != m_inputData.end(); ++iD ) {

      if( iD->GetType() == type ) {

         const std::vector< SGeneratorCut >& sgencuts = iD->GetSGeneratorCuts();
         Bool_t inside = kTRUE;

         for( std::vector< SGeneratorCut >::const_iterator sgc = sgencuts.begin();
              sgc != sgencuts.end(); ++sgc ) {
			 
            // loop over the trees 
            for( vector< TTree* >::const_iterator it = m_inputTrees.begin();
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

   if( totlum > EPSILON) 
      weight = ( GetTargetLumi() / totlum );
  
   return weight;
}

/**
 * This function is used to open the input files for reading. It has the nice
 * property of automatically checking whether the file has been found and
 * correctly opened.
 *
 * <strong>The function is used internally by the framework!</strong>
 *
 * @param filename Name of the input file to open
 */
TFile* SCycleBaseNTuple::OpenInputFile( const char* filename ) throw( SError ) {

   TFile* file = new TFile( filename );
   if( ! file || file->IsZombie() ) {
      SError error( SError::SkipFile );
      error << "Failed to open input file " << filename;
      throw error;
   }

   return file;
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
const char* SCycleBaseNTuple::RootType( const char* typeid_type ) {

   if( strlen( typeid_type ) != 1 ) {
      SError error( SError::StopExecution );
      error << "SCycleBaseNTuple::RootType received complex object description";
      throw error;
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

   SError error( SError::StopExecution );
   error << "Unknown primitive type encountered: " << typeid_type;
   throw error;

   return "";

}

/**
 * Function used by a few of the variable handling functions. It finds
 * the tree with a given name among the input and output trees (in this
 * order), or throws an exception if such tree doesn't exist.
 */
TTree* SCycleBaseNTuple::GetTree( const std::string& treeName ) throw( SError ) {

   //
   // Look for such input tree:
   //
   for( vector< TTree* >::iterator it = m_inputTrees.begin();
        it != m_inputTrees.end(); ++it ) {
      if( *it ) {
         if( treeName == ( *it )->GetName() ) {
            m_logger << VERBOSE << "Found input tree with name " << treeName 
                     << " at " << ( *it ) << SLogger::endmsg;
            return *it;
         }
      }
   }

   //
   // Look for such output tree:
   //
   for( vector< TTree* >::iterator it = m_outputTrees.begin();
        it != m_outputTrees.end(); ++it ) {
      if( *it ) {
         if( treeName == ( *it )->GetName() ) {
            m_logger << VERBOSE << "Found output tree with name " << treeName 
                     << " at " << ( *it ) << SLogger::endmsg;
            return *it;
         }
      }
   }

   //
   // Throw an exception if the tree hasn't been found:
   //
   SError error( SError::SkipFile );
   error << "Couldn't find TTree with name: " << treeName;
   throw error;

   return 0;
}

/**
 * Helper function filling the list of input branches. It is called by the main
 * variable handling functions, not directly by the user.
 */
void SCycleBaseNTuple::RegisterInputBranch( TBranch* br ) throw( SError ) {

   if( find( m_inputBranches.begin(), m_inputBranches.end(), br ) != m_inputBranches.end() ) {
      m_logger << DEBUG << "Branch '" << br->GetName() << "' already registered!"
               << SLogger::endmsg;
   } else {
      m_inputBranches.push_back( br );
   }

   return;
}

/**
 * This function is called after SCycleBaseNTuple::LoadInputTrees(...).
 * The EventView trees hold different number of events. These events can
 * be synchronised through additional variables found in separate
 * (collection) trees.
 *
 * For each view, this function tries to find the variable in the collection
 * tree that can be used for the synchronisation.
 *
 * <strong>The function is used internally by the framework!</strong>
 */
void SCycleBaseNTuple::ConnectEVSyncVariable() throw( SError ) {

   if( ! m_inputTrees.size() ) {
      m_logger << DEBUG << "ConnectEVSyncVariable> No input trees defined"
               << SLogger::endmsg;
      return;
   }
   if( ! m_EVinputTrees.size() ) {
      return;
   }

   // Clear the map(s) helping in synchronizing the EV trees:
   m_EVBaseNameToCollVar.clear();

   // now connect EV trees if they're defined
   for( vector< TTree* >::iterator it = m_EVinputTrees.begin();
        it != m_EVinputTrees.end(); ++it ) {

      //
      // Get the info to find the variable to synchronize this tree with:
      //
      map< TTree*, string >::const_iterator collTreeName =
         m_EVInTreeToCollTreeName.find( *it );
      if( collTreeName == m_EVInTreeToCollTreeName.end() ) {
         SError error( SError::SkipInputData );
         error << "Collection tree name for " << ( *it )->GetName() << " not found";
         throw error;
      }
      map< TTree*, string >::const_iterator baseName = m_EVInTreeToBaseName.find( *it );
      if( baseName == m_EVInTreeToBaseName.end() ) {
         SError error( SError::SkipInputData );
         error << "Base name for " << ( *it )->GetName() << " not found";
         throw error;
      }

      //
      // Find the collection tree for this EV tree:
      //
      TTree* collTree = 0;
      for( vector< TTree* >::iterator coll_it = m_inputTrees.begin();
           coll_it != m_inputTrees.end(); ++coll_it ) {
         if( ( *coll_it )->GetName() == collTreeName->second ) {
            collTree = *coll_it;
            break;
         }
      }
      if( ! collTree ) {
         SError error( SError::SkipInputData );
         error << "Couldn't find collection tree ('" << collTreeName->second
               << "') for EV tree '" << ( *it )->GetName();
         throw error;
      }

      //
      // Connect to the variable allowing the synchronization of this tree:
      //
      m_EVBaseNameToCollVar[ baseName->second ] = 0;
      std::ostringstream varname;
      varname << baseName->second << "NInstance";
      collTree->SetBranchAddress( varname.str().c_str(),
                                  &m_EVBaseNameToCollVar[ baseName->second ] );
   }

   return;
}

/**
 * Function synchronising the branches of the EventView trees. It is quite
 * complicated, so optimisation is always welcome. It is called for each event
 * from SCycleBaseNTuple::GetEntry(...).
 *
 * <strong>The function is used internally by the framework!</strong>
 */
void SCycleBaseNTuple::SyncEVTrees() throw( SError ) {

   m_logger << VERBOSE << "In SyncEVTrees()" << SLogger::endmsg;

   //
   // Return right away if there are no EV trees defined:
   //
   if( ! m_EVInputBranchesToBaseName.size() ) return;

   //
   // Loop over all the EV Trees, and find the ones for which a new entry should be loaded:
   //
   for( vector< TTree* >::const_iterator evtree = m_EVinputTrees.begin();
        evtree != m_EVinputTrees.end(); ++evtree ) {

      // Find the "view number" of this EV tree:
      map< TTree*, Int_t >::const_iterator viewNumber = m_EVInTreeToViewNumber.find( *evtree );
      if( viewNumber == m_EVInTreeToViewNumber.end() ) {
         SError error( SError::SkipInputData );
         error << "SyncEVTrees> View number not found for tree '" << ( *evtree )->GetName()
               << "'";
         throw error;
      } else {
         m_logger << VERBOSE << "SyncEVTrees> ViewNumber for tree '" << ( *evtree )->GetName() << "' is "
                  << viewNumber->second << SLogger::endmsg;
      }

      // Find the "base name" of this EV tree:
      map< TTree*, string >::const_iterator baseName = m_EVInTreeToBaseName.find( *evtree );
      if( baseName == m_EVInTreeToBaseName.end() ) {
         SError error( SError::SkipInputData );
         error << "SyncEVTrees> Base name for '" << ( *evtree )->GetName() << "' not found";
         throw error;
      } else {
         m_logger << VERBOSE << "SyncEVTrees> Base name for '" << ( *evtree )->GetName() << "' is '"
                  << baseName->second << "'" << SLogger::endmsg;
      }

      // Find the variable describing how many views of this type are there for this event:
      map< string, Int_t >::const_iterator viewsInEvent =
         m_EVBaseNameToCollVar.find( baseName->second );
      if( viewsInEvent == m_EVBaseNameToCollVar.end() ) {
         SError error( SError::SkipInputData );
         error << "SyncEVTrees> Collection tree variable not found for tree '"
               << ( *evtree )->GetName() << "'";
         throw error;
      } else {
         m_logger << VERBOSE << "SyncEVTrees> Number of views in current event is "
                  << viewsInEvent->second << SLogger::endmsg;
      }

      //
      // Continue only, if the tree has to be updated:
      //
      if( viewNumber->second < viewsInEvent->second ) {

         m_logger << VERBOSE << "SyncEVTrees> A new entry is to be loaded for tree '"
                  << ( *evtree )->GetName() << "'" << SLogger::endmsg;

         // Find the entry number to load:
         map< TTree*, Int_t >::iterator entryNumber =
            m_EVInTreeToCounters.find( *evtree );
         if( entryNumber == m_EVInTreeToCounters.end() ) {
            SError error( SError::SkipInputData );
            error << "SyncEVTrees> Entry counter not found for tree '" << ( *evtree )->GetName()
                  << "'";
            throw error;
         } else {
            m_logger << VERBOSE << "SyncEVTrees> The entry to load for tree '" << ( *evtree )->GetName()
                     << "' is " << entryNumber->second << SLogger::endmsg;
         }

         // Check if this is a valid entry number:
         if( entryNumber->second >= ( *evtree )->GetEntries() ) {
            SError error( SError::SkipEvent );
            error << "SyncEVTrees> Entry " << entryNumber->second
                  << " requested for tree '" << ( *evtree )->GetName()
                  << "' (" << ( *evtree )->GetEntries() << " entries) in current event";
            throw error;
         } else {
            m_logger << VERBOSE << "SyncEVTrees> This is a valid entry" << SLogger::endmsg;
         }

         //
         // Load the correct entry, and increment the counter:
         //
         //
         //  The previous method was actually fine, since the elements in both maps were
         //  ordered in the same way. But I found it fragile to depend on this. Hence, here
         //  is a new, (hopefully) more robust implementation.
         //
         for( vector< TBranch* >::iterator branch = m_inputBranches.begin();
              branch != m_inputBranches.end(); ++branch ) {

            map< TBranch*, string >::const_iterator brBaseName =
               m_EVInputBranchesToBaseName.find( *branch );
            map< TBranch*, Int_t >::const_iterator viewNumber =
               m_EVInputBranchesToViewNumber.find( *branch );
            if( ( brBaseName == m_EVInputBranchesToBaseName.end() ) ||
                ( viewNumber == m_EVInputBranchesToViewNumber.end() ) ) {
               // This is not an EV branch...
               continue;
            }
            if( ( brBaseName->second != baseName->second ) ||
                ( viewNumber->second >= viewsInEvent->second ) ) {
               // This branch doesn't have to be updated...
               continue;
            }

            // Load the correct entry for this branch:
            m_logger << VERBOSE << "GetEntry(" << entryNumber->second 
                     << ") for tree '" << ( *evtree )->GetName()
                     << "' (view " << viewNumber->second 
                     << "), branch '" << ( *branch )->GetName()
                     << "'" << SLogger::endmsg;
            ( *branch )->GetEntry( entryNumber->second );

         }

         // Now that all branches are loaded, increment the counter
         ++( entryNumber->second );
      }

   } // End of loop over EV trees

   return;
}
