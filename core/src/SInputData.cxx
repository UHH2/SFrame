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

// ROOT include(s):
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TFileCollection.h>
#include <TFileInfo.h>
#include <THashList.h>
#include <TDSet.h>
#include <TProof.h>
#include <TROOT.h>

// Local include(s):
#include "../include/SInputData.h"
#include "../include/SError.h"
#include "../include/SProofManager.h"
#include "../include/STreeTypeDecoder.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SDataSet );
ClassImp( SFile );
ClassImp( STree );
ClassImp( SInputData );
#endif // DOXYGEN_IGNORE

using namespace std;

// Define the constants:
const Int_t STree::INPUT_TREE  = 0x1;
const Int_t STree::OUTPUT_TREE = 0x2;
const Int_t STree::EVENT_TREE  = 0x4;

/**
 * It is only necessary for some technical affairs.
 */
SDataSet& SDataSet::operator= ( const SDataSet& parent ) {

   this->name = parent.name;

   return *this;
}

/**
 * The equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 == inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t SDataSet::operator== ( const SDataSet& rh ) const {

   if( this->name == rh.name ) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

/**
 * The non-equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 != inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t SDataSet::operator!= ( const SDataSet& rh ) const {

   return ( ! ( *this == rh ) );
}

/**
 * It is only necessary for some technical affairs.
 */
SFile& SFile::operator= ( const SFile& parent ) {

   this->file = parent.file;
   this->lumi = parent.lumi;
   this->events = parent.events;

   return *this;
}

/**
 * The equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 == inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t SFile::operator== ( const SFile& rh ) const {

   if( ( this->file == rh.file ) && ( this->lumi == rh.lumi ) &&
       ( this->events == rh.events ) ) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

/**
 * The non-equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 != inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t SFile::operator!= ( const SFile& rh ) const {

   return ( ! ( *this == rh ) );
}

/**
 * It is only necessary for some technical affairs.
 */
STree& STree::operator= ( const STree& parent ) {

   this->treeName = parent.treeName;
   this->type     = parent.type;

   return *this;
}

/**
 * The equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 == inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t STree::operator== ( const STree& rh ) const {

   if( ( this->treeName == rh.treeName ) &&
       ( this->type     == rh.type ) ) {
      return kTRUE;
   } else {
      return kFALSE;
   }
}

/**
 * The non-equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 != inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t STree::operator!= ( const STree& rh ) const {

   return ( ! ( *this == rh ) );
}

/**
 * The constructor initialises all member data to some initial value.
 */
SInputData::SInputData( const char* name )
   : TNamed( name, "SFrame input data object" ), m_type( "unknown" ),
     m_version( 0 ), m_totalLumiGiven( 0 ), m_totalLumiSum( 0 ),
     m_eventsTotal( 0 ), m_neventsmax( -1 ), m_neventsskip( 0 ),
     m_cacheable( kFALSE ), m_skipValid( kFALSE ), m_entry( 0 ),
     m_dset( 0 ), m_logger( "SInputData" ) {

   REPORT_VERBOSE( "In constructor" );
}

/**
 * Another one of the "I don't do anything" destructors. Notice that I'm not
 * deleting the TDSet object. This is basically because TDSet's copy-constructor
 * is private... Since the whole point in storing a TDSet object is to have a
 * *validated* TDSet object, I can't create new objects every time I copy the
 * SInputData object. So I just copy the pointer to the TDSet object, and let
 * all the SInputData instances use the same TDSet.
 *
 * Unfortunately this results in a small memory leak. But since the only solution
 * I see right now is to use Boost (which is not available on all supported platforms
 * by default), I chose to accept this leak for now...
 */
SInputData::~SInputData() {

   REPORT_VERBOSE( "In destructor" );
}

/**
 * The function adds a new input file to the input data, correctly adding
 * the luminosity of the file to the total luminosity sum of the input
 * data.
 *
 * @param sfile The file to add to the input data
 */
void SInputData::AddSFileIn( const SFile& sfile ) {

   m_sfileIn.push_back( sfile );
   m_totalLumiSum += sfile.lumi;
   return;
}

/**
 * This is a generic function for adding a new TTree that is to be handled by
 * SFrame in this input data. Now there can be any number of types of TTrees,
 * which makes adding new functionality quite a bit easier.
 *
 * @param type  Type of the TTree. See the definitions in the STreeType namespace
 * @param stree The STree object to add for the specified type
 */
void SInputData::AddTree( Int_t type, const STree& stree ) {

   m_trees[ type ].push_back( stree );
   return;
}

/**
 * This adds a new dataset to the input data, taking care of adding the luminosity
 * of the dataset to the total.
 *
 * @param dset The dataset to be added to the input data
 */
void SInputData::AddDataSet( const SDataSet& dset ) {

   m_dataSets.push_back( dset );
   m_totalLumiSum += dset.lumi;
   return;
}

/**
 * This function takes care of investigating all the input files defined in the
 * configuration, and checking how many events they each contain. This information
 * is used at run time to calculate the correct weights of the events.
 *
 * The function is smart enough to load already gathered information from a cache file
 * if it exists. The feature has to be enabled by setting Cacheable="1" in the declaration
 * of the InputData block in the configuration XML.
 */
void SInputData::ValidateInput( const char* pserver ) throw( SError ) {

   // Check that the user only specified one type of input:
   if( GetSFileIn().size() && GetDataSets().size() ) {
      m_logger << ERROR << "You cannot use PROOF datasets AND regular input files in the"
               << SLogger::endmsg;
      m_logger << ERROR << "same InputData at the moment. Please only use one type!"
               << SLogger::endmsg;
      throw SError( "Trying to use datasets and files in the same ID",
                    SError::SkipInputData );
   }

   // Check that the user did specify some kind of input:
   if( ( ! GetSFileIn().size() ) && ( ! GetDataSets().size() ) ) {
      m_logger << ERROR << "You need to define at least one file or one dataset as input"
               << SLogger::endmsg;
      throw SError( "Missing input specification", SError::SkipInputData );
   }

   // Check that the configuration makes sense:
   if( GetSkipValid() && ( ( GetNEventsMax() > 0 ) || ( GetNEventsSkip() > 0 ) ) ) {
      m_logger << WARNING << "The input file validation can not be skipped when running on "
               << "a subset of events\n"
               << "Turning on the InputData validation for InputData\n"
               << "   Type: " << GetType() << ", Version: " << GetVersion() << SLogger::endmsg;
      SetSkipValid( kFALSE );
   }

   // Return at this point if the validation can be skipped:
   if( GetSkipValid() ) {
      m_logger << INFO << "Input type \"" << GetType() << "\" version \"" 
               << GetVersion() << "\" : Validation skipped" << SLogger::endmsg;
      return;
   }

   // Now do the actual validation:
   if( GetSFileIn().size() ) {
      ValidateInputFiles();
   } else if( GetDataSets().size() ) {
      if( ! pserver ) {
         m_logger << ERROR << "PROOF server not specified. Can't validate datasets!"
                  << SLogger::endmsg;
         throw SError( "Can't validate PROOF datasets without server name",
                       SError::SkipInputData );
      }
      ValidateInputDataSets( pserver );
   }

   return;

}

/**
 * This function has a slightly different interface than all the other functions.
 * Unforunately I wasn't able to come up with any better ideas on how to signal it to
 * the user when a particular tree type is not available. Which is a pretty normal
 * condition.
 *
 * So now this function returns a null-pointer when there are no trees of the requested
 * type, and returns a pointer to an actual vector when there is at least one
 * such tree.
 *
 * @param type Type of the tree(s)
 * @returns null-pointer if requested trees don't exist, pointer to valid vector
 *          otherwise
 */
const std::vector< STree >* SInputData::GetTrees( Int_t type ) const {

   std::map< Int_t, std::vector< STree > >::const_iterator itr;
   if( ( itr = m_trees.find( type ) ) == m_trees.end() ) {
      return 0;
   } else {
      return &( itr->second );
   }
}

Bool_t SInputData::HasInputTrees() const {

   for( std::map< Int_t, std::vector< STree > >::const_iterator trees = m_trees.begin();
        trees != m_trees.end(); ++trees ) {
      for( std::vector< STree >::const_iterator st = trees->second.begin();
           st != trees->second.end(); ++st ) {
         if( ( st->type & STree::INPUT_TREE ) && ( st->type & STree::EVENT_TREE ) ) {
            return kTRUE;
         }
      }
   }

   return kFALSE;
}

TDSet* SInputData::GetDSet() const {

   return m_dset;
}

Double_t SInputData::GetTotalLumi() const { 
  
   Double_t return_lumi = 0.;
   // use the given luminosity for this InputData in case it is specified
   if( m_totalLumiGiven ) return_lumi = m_totalLumiGiven;
   // otherwise use the sum of all files
   else return_lumi = m_totalLumiSum;
  
   // make sure that the lumi is not zero
   if( ! return_lumi ) 
      m_logger << FATAL << "total luminosity for "<< GetType() << " is ZERO!"
               << SLogger::endmsg;

   return return_lumi;
}

Double_t SInputData::GetScaledLumi() const { 
  
   Double_t scaled_lumi = 0.;

   if( m_neventsmax > -1. ) {
      scaled_lumi = GetTotalLumi() * m_neventsmax / m_eventsTotal;
   } else {
      scaled_lumi = GetTotalLumi();
   }

   return scaled_lumi;
}

/**
 * It is only necessary for some technical affairs.
 */
SInputData& SInputData::operator= ( const SInputData& parent ) {

   this->m_type = parent.m_type;
   this->m_version = parent.m_version;
   this->m_totalLumiGiven = parent.m_totalLumiGiven;
   this->m_gencuts = parent.m_gencuts;
   this->m_sfileIn = parent.m_sfileIn;
   this->m_trees = parent.m_trees;
   this->m_dataSets = parent.m_dataSets;
   this->m_totalLumiSum = parent.m_totalLumiSum;
   this->m_eventsTotal = parent.m_eventsTotal;
   this->m_neventsmax = parent.m_neventsmax;
   this->m_neventsskip = parent.m_neventsskip;
   this->m_cacheable = parent.m_cacheable;
   this->m_skipValid = parent.m_skipValid;
   this->m_entry = parent.m_entry;

   this->m_dset = parent.m_dset;

   return *this;

}

/**
 * The equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 == inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t SInputData::operator== ( const SInputData& rh ) const {

   if( ( this->m_type == rh.m_type ) && ( this->m_version == rh.m_version ) &&
       ( this->m_totalLumiGiven == rh.m_totalLumiGiven ) &&
       ( this->m_gencuts == rh.m_gencuts ) && ( this->m_sfileIn == rh.m_sfileIn ) &&
       ( this->m_trees == rh.m_trees ) &&
       ( this->m_totalLumiSum == rh.m_totalLumiSum ) &&
       ( this->m_eventsTotal == rh.m_eventsTotal ) &&
       ( this->m_neventsmax == rh.m_neventsmax ) &&
       ( this->m_neventsskip == rh.m_neventsskip ) &&
       ( this->m_cacheable == rh.m_cacheable ) &&
       ( this->m_skipValid == rh.m_skipValid ) &&
       ( this->m_dset->IsEqual( rh.m_dset ) ) ) {
      return kTRUE;
   } else {
      return kFALSE;
   }

}

/**
 * The non-equality operator is put in to make code such as
 *
 * <code>
 *    if( inputData1 != inputData2 ) ...
 * </code>
 *
 * possible.
 */
Bool_t SInputData::operator!= ( const SInputData& rh ) const {

   return ( ! ( *this == rh ) );

}

/**
 * At initialisation the cycles print the configuration of the input data
 * which was configured in the XML file. This function is used to print
 * the configuration of a given input data object.
 */
void SInputData::Print( const Option_t* ) const {

   m_logger << INFO << " ---------------------------------------------------------" << endl;
   m_logger << " Type               : " << GetType() << endl;
   m_logger << " Version            : " << GetVersion() << endl;
   m_logger << " Total luminosity   : " << GetTotalLumi() << "pb-1" << endl;
   m_logger << " NEventsMax         : " << GetNEventsMax() << endl;
   m_logger << " NEventsSkip        : " << GetNEventsSkip() << endl;
   m_logger << " Cacheable          : " << ( GetCacheable() ? "Yes" : "No" ) << endl;
   m_logger << " Skip validation    : " << ( GetSkipValid() ? "Yes" : "No" ) << endl;

   for( vector< SGeneratorCut >::const_iterator gc = m_gencuts.begin();
        gc != m_gencuts.end(); ++gc )
      m_logger << " Generator cut      : '" << gc->GetTreeName() << "' (tree) | '"
               << gc->GetFormula() << "' (formula)" << endl;

   for( vector< SDataSet >::const_iterator ds = m_dataSets.begin();
        ds != m_dataSets.end(); ++ds )
      m_logger << " Data Set           : '" << ds->name << "' (name) | '" << ds->lumi
               << "' (lumi)" << endl;
   for( vector< SFile >::const_iterator f = m_sfileIn.begin(); f != m_sfileIn.end();
        ++f )
      m_logger << " Input File         : '" << f->file << "' (file) | '" << f->lumi
               << "' (lumi)" << endl;

   for( std::map< Int_t, std::vector< STree > >::const_iterator trees = m_trees.begin();
        trees != m_trees.end(); ++trees ) {
      for( std::vector< STree >::const_iterator tree = trees->second.begin();
           tree != trees->second.end(); ++tree ) {
         m_logger << " Tree               : '" << tree->treeName << "' (name) | '"
                  << STreeTypeDecoder::Instance()->GetName( trees->first )
                  << "' (type)" << endl;
      }
   }

   m_logger << " ---------------------------------------------------------" << SLogger::endmsg;

   return;
}

void SInputData::ValidateInputFiles() throw( SError ) {

   //
   // Set up the connection to the InputData cache if it's asked for:
   //
   TFile* cachefile = 0;
   TFileCollection* filecoll = 0;
   if( m_cacheable && ( ! m_skipValid ) ) {
      // The filename is hardcoded, since this is the only place where it's needed:
      cachefile = TFile::Open( ".sframe." + GetType() + "." + GetVersion() + ".idcache.root",
                               "UPDATE" );
      m_logger << DEBUG << "Opened: " << cachefile->GetName() << SLogger::endmsg;
      // Try to access the ID information:
      filecoll = ( TFileCollection* ) cachefile->Get( "IDCache" );
      if( ! filecoll ) {
         // Create a new object. This is needed when a new cache is created.
         m_logger << DEBUG << "Creating new TFileCollection" << SLogger::endmsg;
         cachefile->cd();
         filecoll = new TFileCollection( "IDCache", "InputData cache data" );
         cachefile->Append( filecoll );
      } else {
         // The cache already exists:
         m_logger << DEBUG << "Existing TFileCollection found" << SLogger::endmsg;
      }
   }

   // Flag showing if the cache will have to be saved at the end of the function:
   Bool_t cacheUpdated = kFALSE;
   Int_t  fileInfoInDataset = 0;

   //
   // Loop over all the specified input files:
   //
   for( std::vector< SFile >::iterator sf = m_sfileIn.begin(); sf != m_sfileIn.end(); ++sf ) {

      //
      // Try to load the file's information from the cache. This is *much* faster than
      // querying the file itself...
      //
      if( m_cacheable && LoadInfoOnFile( sf, filecoll ) ) {
         ++fileInfoInDataset;
         continue;
      }

      //
      // Open the physical file:
      //
      TFile* file = TFile::Open( sf->file.Data() );
      if( ! file || file->IsZombie() ) {
         m_logger << WARNING << "Couldn't open file: " << sf->file.Data() << SLogger::endmsg;
         m_logger << WARNING << "Removing it from the input file list" << SLogger::endmsg;
         // Erasing the file from the file list is a bit tricky actually:
         sf = m_sfileIn.erase( sf );
         --sf;
         continue;
      }

      try {

         // If any of the files had to be opened, then the cache will need to be
         // updated in the ROOT file:
         cacheUpdated = kTRUE;

         //
         // Create/retrieve the object storing the information about the file:
         //
         TFileInfo* fileinfo = 0;
         if( m_cacheable ) {
            fileinfo = AccessFileInfo( sf, filecoll );
         }

         //
         // Investigate the input trees:
         //
         Bool_t firstPassed = kFALSE;
         Long64_t entries = 0;
         Int_t numberOfBranches = 0;
         // try to load all the input trees
         for( std::map< Int_t, std::vector< STree > >::const_iterator trees = m_trees.begin();
              trees != m_trees.end(); ++trees ) {

            m_logger << DEBUG << "Investigating \""
                     << STreeTypeDecoder::Instance()->GetName( trees->first )
                     << "\" types" << SLogger::endmsg;
            for( std::vector< STree >::const_iterator st = trees->second.begin();
                 st != trees->second.end(); ++st ) {

               // Only check the existence of input trees:
               if( ! ( st->type & STree::INPUT_TREE ) ) continue;

               // Try to access the input tree:
               TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
               if( ! tree ) {
                  m_logger << WARNING << "Couldn't find tree " << st->treeName
                           << " in file " << sf->file << SLogger::endmsg;
                  m_logger << WARNING << "Removing file from the input file list"
                           << SLogger::endmsg;
                  throw SError( SError::SkipFile );
               }

               // Remember how many branches there are in total in the input:
               Int_t branchesThisTree = tree->GetNbranches();
               m_logger << DEBUG << branchesThisTree << " branches in tree " << tree->GetName() 
                        << SLogger::endmsg;
               numberOfBranches += branchesThisTree;

               // Check how many events are there in the input:
               if( st->type & STree::EVENT_TREE ) {
                  if( firstPassed && ( tree->GetEntriesFast() != entries ) ) {
                     m_logger << WARNING << "Conflict in number of entries - Tree "
                              << tree->GetName() << " has " << tree->GetEntriesFast()
                              << " entries, NOT " << entries << SLogger::endmsg;
                     m_logger << WARNING << "Removing " << sf->file
                              << " from the input file list" << SLogger::endmsg;
                     throw SError( SError::SkipFile );
                  } else if( ! firstPassed ) {
                     firstPassed = kTRUE;
                     entries = tree->GetEntriesFast();
                  }
               }

               //
               // Save the information about this tree into the cache:
               //
               if( m_cacheable ) {
                  TFileInfoMeta* tree_info = new TFileInfoMeta( tree->GetName(), "TTree",
                                                                tree->GetEntriesFast() );
                  tree_info->SetName( tree->GetName() );
                  tree_info->SetTitle( "Meta data info for a TTree" );
                  if( ! fileinfo->AddMetaData( tree_info ) ) {
                     m_logger << ERROR << "There was a problem caching meta-data for TTree: "
                              << tree->GetName() << SLogger::endmsg;
                  } else {
                     m_logger << VERBOSE << "Meta-data cached for TTree: " << tree->GetName()
                              << SLogger::endmsg;
                  }
               }
            }
         }

         // Update the ID information:
         sf->events = entries;
         AddEvents( entries );

         m_logger << DEBUG << numberOfBranches << " branches in total in file "
                  << file->GetName() << SLogger::endmsg;

      } catch( const SError& ) {
         m_totalLumiSum -= sf->lumi;
         sf = m_sfileIn.erase( sf );
         --sf;
      }

      // Close the input file:
      file->Close();
      if( file ) delete file;
   }

   //
   // Save/close the cache file if it needs to be saved/closed:
   //
   if( m_dset ) delete m_dset;
   if( m_cacheable ) {

      //
      // Take care of the TFileCollection object:
      //
      if( cacheUpdated ) {
         m_logger << VERBOSE << "Writing file collection object to cache" << SLogger::endmsg;
         cachefile->cd();
         if( filecoll->Update() == -1 ) {
            m_logger << ERROR << "Failed to update the cached information" << SLogger::endmsg;
         }
         filecoll->Write();

         //
         // Create a new dataset and write it to the cache file:
         //
         m_dset = MakeDataSet();
         cachefile->cd();
         m_dset->Write();
      } else {
         // Load the cached dataset:
         m_dset = AccessDataSet( cachefile );
         if( ! m_dset ) {
            throw SError( "There was a logical error in the cache handling.\n"
                          " Id Type: " + GetType() + ", Version: " + GetVersion(),
                          SError::StopExecution );
         }

         // Check if the current configuration is likely to be described by this dataset:
         if( fileInfoInDataset == m_dset->GetListOfElements()->GetSize() ) {
            m_logger << DEBUG << "The loaded dataset is up to date" << SLogger::endmsg;
         } else {
            m_logger << DEBUG << "The dataset has to be updated" << SLogger::endmsg;
            delete m_dset;
            m_dset = MakeDataSet();
            cachefile->cd();
            m_dset->Write();
         }
      }

      cachefile->Close();
      delete cachefile;

   } else {

      m_dset = MakeDataSet();

   }

   //
   // Check that the specified maximum number of events and the number of events to
   // skip, make sense:
   //
   if( GetNEventsSkip() + GetNEventsMax() > GetEventsTotal() ) {
      if( GetNEventsSkip() >= GetEventsTotal() ) {
         SetNEventsMax( 0 );
      } else {
         SetNEventsMax( GetEventsTotal() - GetNEventsSkip() );
      }
   }

   //
   // Print some status:
   //
   m_logger << INFO << "Input type \"" << GetType() << "\" version \"" 
            << GetVersion() << "\" : " << GetEventsTotal() << " events" 
            << ( ( m_cacheable && ( ! cacheUpdated ) ) ? " (cached)" : "" )
            << SLogger::endmsg;

   return;
}

void SInputData::ValidateInputDataSets( const char* pserver ) throw( SError ) {

   // Connect to the PROOF server:
   TProof* server = SProofManager::Instance()->Open( pserver );

   // Check the number of defined datasets. It's only possible to use multiple datasets
   // in a single InputData starting from ROOT 5.27/02. In previous releases only the
   // first one can be used.
   if( ( ROOT_VERSION_CODE < ROOT_VERSION( 5, 27, 02 ) ) &&
       // The special PROOF branch of the ROOT development code can also be used:
       ( strcmp( gROOT->GetVersion(), "5.26/00-proof" ) ) &&
       ( m_dataSets.size() > 1 ) ) {

      m_logger << WARNING << "You're currently using ROOT version: "
               << gROOT->GetVersion() << "\n"
               << "This version doesn't yet support defining multiple\n"
               << "datasets per InputData. Only the first one is going to be used!"
               << SLogger::endmsg;
      m_logger << WARNING << "To use multiple datasets, upgrate to at least ROOT 5.27/02"
               << SLogger::endmsg;
      m_dataSets.resize( 1 );
      m_totalLumiSum = m_dataSets.front().lumi;
   }

   //
   // Loop over the specified datasets:
   //
   for( std::vector< SDataSet >::iterator ds = m_dataSets.begin();
        ds != m_dataSets.end(); ++ds ) {

      try {

         //
         // Check if the dataset exists on the server:
         //
         TFileCollection* filecoll = server->GetDataSet( ds->name );
         if( ! filecoll ) {
            m_logger << ERROR << "Dataset \"" << ds->name << "\" doesn't exist on server: "
                     << pserver << SLogger::endmsg;
            throw SError( SError::SkipFile );
         }

         //
         // Investigate the input trees:
         //
         Bool_t firstPassed = kFALSE;
         Long64_t entries = 0;
         for( std::map< Int_t, std::vector< STree > >::const_iterator trees = m_trees.begin();
              trees != m_trees.end(); ++trees ) {

            m_logger << DEBUG << "Investigating \""
                     << STreeTypeDecoder::Instance()->GetName( trees->first )
                     << "\" types" << SLogger::endmsg;
            for( std::vector< STree >::const_iterator st = trees->second.begin();
                 st != trees->second.end(); ++st ) {

               // Only check the existence of input trees:
               if( ! ( st->type & STree::INPUT_TREE ) ) continue;

               // Don't check for trees in sub-directories:
               if( st->treeName.Contains( "/" ) ) continue;

               // Try to access information on the input tree:
               Long64_t tree_entries = filecoll->GetTotalEntries( "/" + st->treeName );
               if( tree_entries == -1 ) {
                  m_logger << ERROR << "Couldn't find tree " << st->treeName << " in dataset "
                           << ds->name << SLogger::endmsg;
                  m_logger << ERROR << "Removing dataset from the input list"
                           << SLogger::endmsg;
                  throw SError( SError::SkipFile );
               }

               // Check how many events are there in the input:
               if( st->type & STree::EVENT_TREE ) {
                  if( firstPassed && ( tree_entries != entries ) ) {
                     m_logger << WARNING << "Conflict in number of entries - Tree "
                              << st->treeName << " has " << tree_entries
                              << " entries, NOT " << entries << SLogger::endmsg;
                     m_logger << WARNING << "Removing " << ds->name
                              << " from the input dataset list" << SLogger::endmsg;
                     throw SError( SError::SkipFile );
                  } else if( ! firstPassed ) {
                     firstPassed = kTRUE;
                     entries = tree_entries;
                  }
               }
            }
         }

         // Update the ID information:
         ds->events = entries;
         AddEvents( entries );

      } catch( const SError& ) {
         m_totalLumiSum -= ds->lumi;
         ds = m_dataSets.erase( ds );
         --ds;
      }

   }

   //
   // Print some status:
   //
   m_logger << INFO << "Input type \"" << GetType() << "\" version \"" 
            << GetVersion() << "\" : " << GetEventsTotal() << " events" 
            << SLogger::endmsg;

   return;
}

Bool_t SInputData::LoadInfoOnFile( std::vector< SFile >::iterator& file_itr,
                                   TFileCollection* filecoll ) {

   // Retrieve the information about this specific file:
   TFileInfo* fileinfo = ( TFileInfo* ) filecoll->GetList()->FindObject( file_itr->file );
   if( ! fileinfo ) {
      m_logger << VERBOSE << "File unknown: " << file_itr->file << SLogger::endmsg;
      return kFALSE;
   }

   m_logger << DEBUG << "Information found for: " << file_itr->file << SLogger::endmsg;

   Bool_t firstPassed = kFALSE; // Flag showing if we already know the number of entries
   Long64_t entries = 0; // Number of entries in the file

   //
   // Check that information is available on all the input trees in the cache:
   //
   for( std::map< Int_t, std::vector< STree > >::const_iterator trees = m_trees.begin();
        trees != m_trees.end(); ++trees ) {
      for( std::vector< STree >::const_iterator st = trees->second.begin();
           st != trees->second.end(); ++st ) {

         // Only check the existence of input trees:
         if( ! ( st->type & STree::INPUT_TREE ) ) continue;

         // Get the tree information:
         TFileInfoMeta* tree_info = fileinfo->GetMetaData( st->treeName );
         if( ! tree_info ) {
            m_logger << DEBUG << "No description found for: " << st->treeName
                     << SLogger::endmsg;
            return kFALSE;
         }

         // Check how many events are there in the input:
         if( st->type & STree::EVENT_TREE ) {
            if( ! firstPassed ) {
               firstPassed = kTRUE;
               entries = tree_info->GetEntries();
            } else if( entries != tree_info->GetEntries() ) {
               m_logger << WARNING << "Inconsistent cached data for: "
                        << file_itr->file << " -> Checking the file again..."
                        << SLogger::endmsg;
               return kFALSE;
            }
         }
      }
   }

   //
   // Update the ID with this information:
   //
   file_itr->events = entries;
   AddEvents( entries );

   return kTRUE;
}

TFileInfo* SInputData::AccessFileInfo( std::vector< SFile >::iterator& file_itr,
                                       TFileCollection* filecoll ) {

   TFileInfo* result = 0;

   // Check if we know anything about this file already:
   if( ( result = ( TFileInfo* ) filecoll->GetList()->FindObject( file_itr->file ) ) ) {
      m_logger << DEBUG << "Updating information for " << file_itr->file << SLogger::endmsg;
   } else {
      // One has to be very verbose in naming the object, otherwise the stupid
      // ROOT container will not be able to find it afterwards...
      m_logger << DEBUG << "Creating information for " << file_itr->file << SLogger::endmsg;
      result = new TFileInfo( file_itr->file );
      result->SetName( file_itr->file );
      result->SetTitle( "Description for: " + file_itr->file );
      filecoll->Add( result );
   }

   return result;
}

TDSet* SInputData::MakeDataSet() throw( SError ) {

   // Find the name of the "main" TTree in the files:
   const char* treeName = 0;
   for( std::map< Int_t, std::vector< STree > >::const_iterator trees = m_trees.begin();
        trees != m_trees.end(); ++trees ) {
      for( std::vector< STree >::const_iterator st = trees->second.begin();
           st != trees->second.end(); ++st ) {
         if( ( st->type & STree::INPUT_TREE ) && ( st->type & STree::EVENT_TREE ) ) {
            treeName = st->treeName.Data();
         }
      }
   }
   if( ! treeName ) {
      throw SError( "Can't determine input TTree name!", SError::SkipInputData );
   }

   // Create a TChain that will be the basis of the dataset:
   TChain chain( treeName );
   for( std::vector< SFile >::const_iterator file = GetSFileIn().begin();
        file != GetSFileIn().end(); ++file ) {
      chain.Add( file->file );
   }

   // Create the dataset:
   TDSet* result = new TDSet( chain );
   result->SetName( "DSetCache" );
   result->SetTitle( "Cached dataset for ID Type: " + GetType() + ", Version: " +
                     GetVersion() );
   result->Validate();

   return result;
}

TDSet* SInputData::AccessDataSet( TDirectory* dir ) {

   return dynamic_cast< TDSet* >( dir->Get( "DSetCache" ) );
}
