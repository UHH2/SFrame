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

#ifndef DOXYGEN_IGNORE
ClassImp( SDataSet );
ClassImp( SFile );
ClassImp( STree );
ClassImp( SInputData );
#endif // DOXYGEN_IGNORE

using namespace std;

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

   if( this->treeName == rh.treeName ) {
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
     m_cacheable( kFALSE ), m_dset( 0 ), m_logger( "SInputData" ) {

   m_logger << VERBOSE << "In constructor" << SLogger::endmsg;
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

   m_logger << VERBOSE << "In destructor" << SLogger::endmsg;

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
   if( m_sfileIn.size() && m_dataSets.size() ) {
      m_logger << ERROR << "You cannot use PROOF datasets AND regular input files in the"
               << SLogger::endmsg;
      m_logger << ERROR << "same InputData at the moment. Please only use one type!"
               << SLogger::endmsg;
      throw SError( "Trying to use datasets and files in the same ID",
                    SError::SkipInputData );
   }

   // Check that the user did specify some kind of input:
   if( ( ! m_sfileIn.size() ) && ( ! m_dataSets.size() ) ) {
      m_logger << ERROR << "You need to define at least one file or one dataset as input"
               << SLogger::endmsg;
      throw SError( "Missing input specification", SError::SkipInputData );
   }

   // Now do the actual validation:
   if( m_sfileIn.size() ) {
      ValidateInputFiles();
   } else if( m_dataSets.size() ) {
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
   } else
      scaled_lumi=GetTotalLumi();

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
   this->m_inputTrees = parent.m_inputTrees;
   this->m_persTrees = parent.m_persTrees;
   this->m_outputTrees = parent.m_outputTrees;
   this->m_totalLumiSum = parent.m_totalLumiSum;
   this->m_eventsTotal = parent.m_eventsTotal;
   this->m_neventsmax = parent.m_neventsmax;
   this->m_neventsskip = parent.m_neventsskip;
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
       ( this->m_inputTrees == rh.m_inputTrees ) &&
       ( this->m_persTrees == rh.m_persTrees ) &&
       ( this->m_outputTrees == rh.m_outputTrees ) &&
       ( this->m_totalLumiSum == rh.m_totalLumiSum ) &&
       ( this->m_eventsTotal == rh.m_eventsTotal ) &&
       ( this->m_neventsmax == rh.m_neventsmax ) &&
       ( this->m_neventsskip == rh.m_neventsskip ) &&
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
void SInputData::print() const {

   m_logger << INFO << " ---------------------------------------------------------" << endl;
   m_logger << " Type               : " << GetType() << endl;
   m_logger << " Version            : " << GetVersion() << endl;
   m_logger << " Total luminosity   : " << GetTotalLumi() << "pb-1" << endl;
   m_logger << " NEventsMax         : " << GetNEventsMax() << endl;
   m_logger << " NEventsSkip        : " << GetNEventsSkip() << endl;
   m_logger << " Cacheable          : " << ( GetCacheable() ? "Yes" : "No" ) << endl;

   for( vector< SGeneratorCut >::const_iterator gc = m_gencuts.begin();
        gc != m_gencuts.end(); ++gc )
      m_logger << " Generator cut      : Tree:" << gc->GetTreeName() << " Formula: "
               << gc->GetFormula() << endl;

   for( vector< SDataSet >::const_iterator ds = m_dataSets.begin();
        ds != m_dataSets.end(); ++ds )
      m_logger << " Data Set           : '" << ds->name << "' (name) | '" << ds->lumi
               << "' (lumi)" << endl;
   for( vector< SFile >::const_iterator f = m_sfileIn.begin(); f != m_sfileIn.end();
        ++f )
      m_logger << " Input SFile        : " << "'" << f->file << "' (file) | '" << f->lumi
               << "' (lumi)" << endl;
   for( std::vector< STree >::const_iterator t = m_inputTrees.begin();
        t != m_inputTrees.end(); ++t )
      m_logger << " Input tree         : " << "'" << t->treeName << "'" << endl;
   for( std::vector< STree >::const_iterator t = m_persTrees.begin();
        t != m_persTrees.end(); ++t )
      m_logger << " Persistent tree    : " << "'" << t->treeName << "'" << endl;
   for( std::vector< STree >::const_iterator t = m_outputTrees.begin();
        t != m_outputTrees.end(); ++t )
      m_logger << " Output tree        : " << "'" << t->treeName << "'" << endl;
   for( std::vector< STree >::const_iterator t = m_metaTrees.begin();
        t != m_metaTrees.end(); ++t )
      m_logger << " Metadata tree      : " << "'" << t->treeName << "'" << endl;

   m_logger << " ---------------------------------------------------------" << SLogger::endmsg;

   return;

}

void SInputData::ValidateInputFiles() throw( SError ) {

   //
   // Set up the connection to the InputData cache if it's asked for:
   //
   TFile* cachefile = 0;
   TFileCollection* filecoll = 0;
   if( m_cacheable ) {
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
         // Investigate the "regular" trees:
         //
         Bool_t firstPassed = kFALSE;
         Long64_t entries = 0;
         Int_t numberOfBranches = 0;
         // try to load all the input trees
         for( std::vector< STree >::const_iterator st = m_inputTrees.begin();
              st != m_inputTrees.end(); ++st ) {
            TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
            if( ! tree ) {
               m_logger << WARNING << "Couldn't find tree " << st->treeName
                        << " in file " << sf->file << SLogger::endmsg;
               m_logger << WARNING << "Removing file from the input file list"
                        << SLogger::endmsg;
               throw SError( SError::SkipFile );
            } else {
               if( firstPassed && tree->GetEntriesFast() != entries ) {
                  m_logger << WARNING << "Conflict in number of entries - Tree "
                           << tree->GetName() << " has " << tree->GetEntries()
                           << ", NOT " << entries << SLogger::endmsg;
                  m_logger << WARNING << "Removing " << sf->file
                           << " from the input file list" << SLogger::endmsg;
                  throw SError( SError::SkipFile );
               } else if( ! firstPassed ) {
                  firstPassed = kTRUE;
                  entries = tree->GetEntries();
               }
               Int_t branchesThisTree = tree->GetNbranches();
               m_logger << DEBUG << branchesThisTree << " branches in tree " << tree->GetName() 
                        << SLogger::endmsg;
               numberOfBranches += branchesThisTree;
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

         //
         // Check the persistent tree(s):
         //
         for( std::vector< STree >::const_iterator st = m_persTrees.begin();
              st != m_persTrees.end(); ++st ) {
            TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
            if( ! tree ) {
               m_logger << WARNING << "Couldn't find tree " << st->treeName
                        << " in file " << sf->file << SLogger::endmsg;
               m_logger << WARNING << "Removing file from the input file list"
                        << SLogger::endmsg;
               throw SError( SError::SkipFile );
            } else {
               if( firstPassed && tree->GetEntriesFast() != entries ) {
                  m_logger << WARNING << "Conflict in number of entries - Tree "
                           << tree->GetName() << " has " << tree->GetEntries()
                           << ", NOT " << entries << SLogger::endmsg;
                  m_logger << WARNING << "Removing " << sf->file
                           << " from the input file list" << SLogger::endmsg;
                  throw SError( SError::SkipFile );
               } else if( ! firstPassed ) {
                  firstPassed = kTRUE;
                  entries = tree->GetEntriesFast();
               }
            }
            Int_t branchesThisTree = tree->GetNbranches();
            m_logger << DEBUG << branchesThisTree << " branches in tree " << tree->GetName() 
                     << SLogger::endmsg;
            numberOfBranches += branchesThisTree;
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

         // Update the ID information:
         sf->events = entries;
         AddEvents( entries );

         //
         // Check the metadata tree(s):
         //
         for( std::vector< STree >::const_iterator mt = m_metaTrees.begin();
              mt != m_metaTrees.end(); ++mt ) {
            TTree* tree = dynamic_cast< TTree* >( file->Get( mt->treeName ) );
            if( ! tree ) {
               m_logger << WARNING << "Couldn't find metadata tree \"" << mt->treeName
                        << " in file " << sf->file << SLogger::endmsg;
               m_logger << WARNING << "Removing file from the input file list"
                        << SLogger::endmsg;
               throw SError( SError::SkipFile );
            }
            Int_t branchesThisTree = tree->GetNbranches();
            m_logger << DEBUG << branchesThisTree << " branches in tree " << mt->treeName
                     << SLogger::endmsg;
            //
            // Save the information about this tree into the cache:
            //
            if( m_cacheable ) {
               TFileInfoMeta* tree_info = new TFileInfoMeta( mt->treeName, "TTree",
                                                             tree->GetEntriesFast() );
               tree_info->SetName( mt->treeName );
               tree_info->SetTitle( "Meta data info for a TTree" );
               if( ! fileinfo->AddMetaData( tree_info ) ) {
                  m_logger << ERROR << "There was a problem caching meta-data for TTree: "
                           << mt->treeName << SLogger::endmsg;
               } else {
                  m_logger << VERBOSE << "Meta-data cached for TTree: " << mt->treeName
                           << SLogger::endmsg;
               }
            }
         }

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
            throw SError( "There was a logical error in the cache handling\n."
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
         // Investigate the "regular" trees:
         //
         Bool_t firstPassed = kFALSE;
         Long64_t entries = 0;
         for( std::vector< STree >::const_iterator st = m_inputTrees.begin();
              st != m_inputTrees.end(); ++st ) {
            Long64_t tree_entries = filecoll->GetTotalEntries( "/" + st->treeName );
            if( tree_entries == -1 ) {
               m_logger << ERROR << "Couldn't find tree " << st->treeName << " in dataset "
                        << ds->name << SLogger::endmsg;
               m_logger << ERROR << "Removing dataset from the input list" << SLogger::endmsg;
               throw SError( SError::SkipFile );
            }
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

         //
         // Check the persistent tree(s):
         //
         for( std::vector< STree >::const_iterator st = m_persTrees.begin();
              st != m_persTrees.end(); ++st ) {
            Long64_t tree_entries = filecoll->GetTotalEntries( "/" + st->treeName );
            if( tree_entries == -1 ) {
               m_logger << ERROR << "Couldn't find tree " << st->treeName << " in dataset "
                        << ds->name << SLogger::endmsg;
               m_logger << ERROR << "Removing dataset from the input list" << SLogger::endmsg;
               throw SError( SError::SkipFile );
            }
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

         // Update the ID information:
         ds->events = entries;
         AddEvents( entries );

         //
         // Check the metadata tree(s):
         //
         for( std::vector< STree >::const_iterator mt = m_metaTrees.begin();
              mt != m_metaTrees.end(); ++mt ) {
            Long64_t tree_entries = filecoll->GetTotalEntries( "/" + mt->treeName );
            if( tree_entries == -1 ) {
               m_logger << ERROR << "Couldn't find tree " << mt->treeName << " in dataset "
                        << ds->name << SLogger::endmsg;
               m_logger << ERROR << "Removing dataset from the input list" << SLogger::endmsg;
               throw SError( SError::SkipFile );
            }
         }

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

   Bool_t firstTree = kTRUE; // Flag showing if this is the first tree investigated
   Long64_t entries = 0; // Number of entries in the file

   //
   // Check that information is available on all the "regular" trees in the cache:
   //
   for( std::vector< STree >::const_iterator st = m_inputTrees.begin();
        st != m_inputTrees.end(); ++st ) {
      TFileInfoMeta* tree_info = fileinfo->GetMetaData( st->treeName );
      if( ! tree_info ) {
         m_logger << DEBUG << "No description found for: " << st->treeName
                  << SLogger::endmsg;
         return kFALSE;
      }

      // If the information is available, check that the tree contains the same
      // number of entries as all the other trees:
      if( firstTree ) {
         entries = tree_info->GetEntries();
         firstTree = kFALSE;
      } else if( entries != tree_info->GetEntries() ) {
         m_logger << WARNING << "Inconsistent cached data for: "
                  << file_itr->file << " -> Checking the file again..." << SLogger::endmsg;
         return kFALSE;
      }
   }

   //
   // Check that information is available on all the persistent trees in the cache:
   //
   for( std::vector< STree >::const_iterator st = m_persTrees.begin();
        st != m_persTrees.end(); ++st ) {
      TFileInfoMeta* tree_info = fileinfo->GetMetaData( st->treeName );
      if( ! tree_info ) {
         m_logger << DEBUG << "No description found for: " << st->treeName
                  << SLogger::endmsg;
         return kFALSE;
      }

      // If the information is available, check that the tree contains the same
      // number of entries as all the other trees:
      if( firstTree ) {
         entries = tree_info->GetEntries();
         firstTree = kFALSE;
      } else if( entries != tree_info->GetEntries() ) {
         m_logger << WARNING << "Inconsistent cached data for: "
                  << file_itr->file << " -> Checking the file again..." << SLogger::endmsg;
         return kFALSE;
      }
   }

   //
   // Check that information is available on all the metadata trees in the cache:
   //
   for( std::vector< STree >::const_iterator mt = m_metaTrees.begin();
        mt != m_metaTrees.end(); ++mt ) {
      TFileInfoMeta* tree_info = fileinfo->GetMetaData( mt->treeName );
      if( ! tree_info ) {
         m_logger << DEBUG << "No description found for: " << mt->treeName
                  << SLogger::endmsg;
         return kFALSE;
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
   if( GetInputTrees().size() ) {
      treeName = GetInputTrees().front().treeName.Data();
   } else if( GetPersTrees().size() ) {
      treeName = GetPersTrees().front().treeName.Data();
   }
   if( ! treeName ) {
      throw SError( "Can't determine input TTree name!", SError::SkipInputData );
   }

   TChain chain( treeName );
   for( std::vector< SFile >::const_iterator file = GetSFileIn().begin();
        file != GetSFileIn().end(); ++file ) {
      chain.Add( file->file );
   }

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
