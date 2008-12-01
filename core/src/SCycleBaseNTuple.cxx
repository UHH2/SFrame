// $Id: SCycleBaseNTuple.cxx,v 1.4.2.1 2008-12-01 14:52:56 krasznaa Exp $
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

/*
#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseNTuple );
#endif // DOXYGEN_IGNORE
*/

using namespace std;

static Double_t EPSILON = 1e-15;

/**
 * The constructor is only initialising the base class.
 */
SCycleBaseNTuple::SCycleBaseNTuple()
   : SCycleBaseBase(), m_inputTrees(), m_inputBranches(),
     m_outputTrees(), m_outputVarPointers(), m_output( 0 ) {

   m_logger << VERBOSE << "SCycleBaseNTuple constructed" << SLogger::endmsg;

}

/**
 * Another one of the "I don't do anything" destructors.
 */
SCycleBaseNTuple::~SCycleBaseNTuple() {

   m_logger << VERBOSE << "SCycleBaseNTuple destructed" << SLogger::endmsg;

}

void SCycleBaseNTuple::SetNTupleOutput( TList* output ) {

   m_output = output;
   return;

}

TList* SCycleBaseNTuple::GetNTupleOutput() const {

   return m_output;

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
                                          std::vector< TTree* >& outTrees ) throw( SError ) {

   // sanity checks
   if( outTrees.size() )
      m_logger << WARNING << "Vector of output trees is not empty in "
               << "\"CreateOutputTrees\"!"  << SLogger::endmsg;

   // Clear the vector of output trees:
   m_outputTrees.clear();

   // Clear the vector of output variable pointers:
   m_outputVarPointers.clear();

   const std::vector< STree >& sOutTree = iD.GetOutputTrees();
   // Open output file / create output trees
   gROOT->cd();

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
      TTree::SetBranchStyle( branchStyle );

      outTrees.push_back( tree );
      m_outputTrees.push_back( tree );
      SCycleOutput* out = new SCycleOutput( tree, st->treeName );
      m_output->Add( out );
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
void SCycleBaseNTuple::LoadInputTrees( const SInputData& iD,
                                       TTree* main_tree ) throw( SError ) {

   const std::vector< STree >& sInTree = iD.GetInputTrees();
   Bool_t firstPassed = kFALSE;
   Int_t nEvents = 0;
   m_inputTrees.clear();
   m_inputBranches.clear();

   TFile* file = 0;
   if( GetConfig().GetRunMode() == SCycleConfig::LOCAL ) {
      TChain* chain = dynamic_cast< TChain* >( main_tree );
      if( ! chain ) {
         throw SError( "In LOCAL running the input TTree is not a TChain!",
                       SError::StopExecution );
      }
      file = chain->GetFile();
   } else if( GetConfig().GetRunMode() == SCycleConfig::PROOF ) {
      file = main_tree->GetCurrentFile();
   } else {
      throw SError( "Running mode not recongnised", SError::SkipCycle );
   }
   if( ! file ) {
      throw SError( "Couldn't get the input file pointer!", SError::SkipFile );
      return;
   }

   for( vector< STree >::const_iterator st = sInTree.begin(); st != sInTree.end();
        ++st ) {

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

   // Load the current entry for all the regular input variables:
   for( vector< TBranch* >::const_iterator it = m_inputBranches.begin();
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
   TString type = inputData.GetType();

   Double_t weight = 0.;
   Double_t totlum = 0.;

   if( inputData.GetType() == "data" ) {
      m_logger << VERBOSE << "Data" << SLogger::endmsg;
      weight = 1.;
      return weight;
   }

   //iterate over vector of input data and addup the weight for the type of this input data
   for( vector< SInputData >::const_iterator iD = GetConfig().GetInputData().begin();
        iD != GetConfig().GetInputData().end(); ++iD ) {

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
      weight = ( GetConfig().GetTargetLumi() / totlum );
  
   return weight;
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
TTree* SCycleBaseNTuple::GetInputTree( const std::string& treeName ) throw( SError ) {

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
   // Throw an exception if the tree hasn't been found:
   //
   SError error( SError::SkipFile );
   error << "Couldn't find input TTree with name: " << treeName;
   throw error;

   return 0;
}

TTree* SCycleBaseNTuple::GetOutputTree( const std::string& treeName ) throw( SError ) {

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
   error << "Couldn't find output TTree with name: " << treeName;
   throw error;

   return 0;
}

/**
 * Helper function filling the list of input branches. It is called by the main
 * variable handling functions, not directly by the user.
 */
void SCycleBaseNTuple::RegisterInputBranch( TBranch* br ) throw( SError ) {

   if( find( m_inputBranches.begin(), m_inputBranches.end(), br ) !=
       m_inputBranches.end() ) {
      m_logger << DEBUG << "Branch '" << br->GetName() << "' already registered!"
               << SLogger::endmsg;
   } else {
      m_inputBranches.push_back( br );
   }

   return;
}
