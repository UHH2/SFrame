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
#include <TSystem.h>

// Local include(s):
#include "../include/SInputData.h"
#include "../include/SError.h"
#include "../include/SProofManager.h"
#include "../include/STreeTypeDecoder.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SDataSet )
ClassImp( SFile )
ClassImp( STree )
ClassImp( SInputData )
#endif // DOXYGEN_IGNORE

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
     m_cacheable( kFALSE ), m_skipValid( kFALSE ), m_skipLookup( kFALSE ),
     m_entry( 0 ), m_dset( 0 ), m_logger( "SInputData" ) {

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
 * Unfortunately this results in a small memory leak. But since the only
 * solution I see right now is to use Boost (which is not available on all
 * supported platforms by default), I chose to accept this leak for now...
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
 * @param type Type of the TTree. See the definitions in the STreeType namespace
 * @param stree The STree object to add for the specified type
 */
void SInputData::AddTree( Int_t type, const STree& stree ) {

   m_trees[ type ].push_back( stree );
   return;
}

/**
 * This adds a new dataset to the input data, taking care of adding the
 * luminosity of the dataset to the total.
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
 * configuration, and checking how many events they each contain. This
 * information is used at run time to calculate the correct weights of the
 * events.
 *
 * The function is smart enough to load already gathered information from a
 * cache file if it exists. The feature has to be enabled by setting
 * Cacheable="1" in the declaration of the InputData block in the configuration
 * XML.
 *
 * @param pserver Name of the PROOF server to use in the validation
 */
void SInputData::ValidateInput( const char* pserver ) throw( SError ) {

   // Check that the user only specified one type of input:
   if( GetSFileIn().size() && GetDataSets().size() ) {
      REPORT_ERROR( "You cannot use PROOF datasets AND regular input files "
                    "in the" );
      REPORT_ERROR( "same InputData at the moment. Please only use one type!" );
      throw SError( "Trying to use datasets and files in the same ID",
                    SError::SkipInputData );
   }

   // Check that the user did specify some kind of input:
   if( ( ! GetSFileIn().size() ) && ( ! GetDataSets().size() ) ) {
      REPORT_ERROR( "You need to define at least one file or one dataset as "
                    "input" );
      throw SError( "Missing input specification", SError::SkipInputData );
   }

   // Check that the configuration makes sense:
   if( GetSkipValid() && ( ( GetNEventsMax() > 0 ) ||
                           ( GetNEventsSkip() > 0 ) ) ) {
      m_logger << WARNING << "The input file validation can not be skipped "
               << "when running on a subset of events\n"
               << "Turning on the InputData validation for InputData\n"
               << "   Type: " << GetType() << ", Version: " << GetVersion()
               << SLogger::endmsg;
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
         REPORT_ERROR( "PROOF server not specified. Can't validate datasets!" );
         throw SError( "Can't validate PROOF datasets without server name",
                       SError::SkipInputData );
      }
      ValidateInputDataSets( pserver );
   }

   return;
}

/**
 * This function has a slightly different interface than all the other
 * functions. Unforunately I wasn't able to come up with any better ideas on how
 * to signal it to the user when a particular tree type is not available. Which
 * is a pretty normal condition.
 *
 * So now this function returns a null-pointer when there are no trees of the
 * requested type, and returns a pointer to an actual vector when there is at
 * least one such tree.
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

/**
 * @returns <code>kTRUE</code> if there is/are (a) input tree(s) defined,
 *          <code>kFALSE</code> if not
 */
Bool_t SInputData::HasInputTrees() const {

   std::map< Int_t, std::vector< STree > >::const_iterator tree_itr =
      m_trees.begin();
   std::map< Int_t, std::vector< STree > >::const_iterator tree_end =
      m_trees.end();
   for( ; tree_itr != tree_end; ++tree_itr ) {
      std::vector< STree >::const_iterator st_itr = tree_itr->second.begin();
      std::vector< STree >::const_iterator st_end = tree_itr->second.end();
      for( ; st_itr != st_end; ++st_itr ) {
         if( ( st_itr->type & STree::INPUT_TREE ) &&
             ( st_itr->type & STree::EVENT_TREE ) ) {
            return kTRUE;
         }
      }
   }

   return kFALSE;
}

/**
 * @returns The currently available dataset pointer
 */
TDSet* SInputData::GetDSet() const {

   return m_dset;
}

/**
 * @returns The total luminosiy available from the input data
 */
Double_t SInputData::GetTotalLumi() const { 
  
   Double_t return_lumi = 0.;
   // use the given luminosity for this InputData in case it is specified
   if( m_totalLumiGiven ) {
      return_lumi = m_totalLumiGiven;
   }
   // otherwise use the sum of all files/datasets
   else {
      return_lumi = m_totalLumiSum;
   }

   // make sure that the lumi is not zero
   if( ! return_lumi ) {
      REPORT_ERROR( "Total luminosity for "<< GetType() << " is ZERO!" );
   }

   return return_lumi;
}

/**
 * @returns The luminosity that should be used in the luminosity weighting
 */
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
 *
 * @returns <code>kTRUE</code> if the two input data are identical,
 *          <code>kFALSE</code> if they are not
 */
Bool_t SInputData::operator== ( const SInputData& rh ) const {

   if( ( this->m_type == rh.m_type ) &&
       ( this->m_version == rh.m_version ) &&
       ( this->m_totalLumiGiven == rh.m_totalLumiGiven ) &&
       ( this->m_gencuts == rh.m_gencuts ) &&
       ( this->m_sfileIn == rh.m_sfileIn ) &&
       ( this->m_trees == rh.m_trees ) &&
       ( this->m_dataSets == rh.m_dataSets ) &&
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
 *
 * @returns <code>kTRUE</code> if the two input data are different,
 *          <code>kFALSE</code> if they are not
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

   m_logger << INFO
            << " ---------------------------------------------------------"
            << std::endl;
   m_logger << " Type               : " << GetType() << std::endl;
   m_logger << " Version            : " << GetVersion() << std::endl;
   m_logger << " Total luminosity   : " << GetTotalLumi() << "pb-1"
            << std::endl;
   m_logger << " NEventsMax         : " << GetNEventsMax() << std::endl;
   m_logger << " NEventsSkip        : " << GetNEventsSkip() << std::endl;
   m_logger << " Cacheable          : " << ( GetCacheable() ? "Yes" : "No" )
            << std::endl;
   m_logger << " Skip validation    : " << ( GetSkipValid() ? "Yes" : "No" )
            << std::endl;
   m_logger << " Skip file lookup   : " << ( GetSkipLookup() ? "Yes" : "No" )
            << std::endl;

   for( std::vector< SGeneratorCut >::const_iterator gc = m_gencuts.begin();
        gc != m_gencuts.end(); ++gc ) {
      m_logger << " Generator cut      : '" << gc->GetTreeName()
               << "' (tree) | '" << gc->GetFormula() << "' (formula)"
               << std::endl;
   }

   for( std::vector< SDataSet >::const_iterator ds = m_dataSets.begin();
        ds != m_dataSets.end(); ++ds ) {
      m_logger << " Data Set           : '" << ds->name << "' (name) | '"
               << ds->lumi << "' (lumi)" << std::endl;
   }
   for( std::vector< SFile >::const_iterator f = m_sfileIn.begin();
        f != m_sfileIn.end(); ++f ) {
      m_logger << " Input File         : '" << f->file << "' (file) | '"
               << f->lumi << "' (lumi)" << std::endl;
   }

   std::map< Int_t, std::vector< STree > >::const_iterator tree_itr =
      m_trees.begin();
   std::map< Int_t, std::vector< STree > >::const_iterator tree_end =
      m_trees.end();
   for( ; tree_itr != tree_end; ++tree_itr ) {
      for( std::vector< STree >::const_iterator tree = tree_itr->second.begin();
           tree != tree_itr->second.end(); ++tree ) {
         m_logger << " Tree               : '" << tree->treeName
                  << "' (name) | '"
                  << STreeTypeDecoder::Instance()->GetName( tree_itr->first )
                  << "' (type)" << std::endl;
      }
   }

   m_logger << " ---------------------------------------------------------"
            << SLogger::endmsg;

   return;
}

/**
 * This function is used to get an "XML representation" of the configuration
 * stored in this input data object. It is used to archive the cycle
 * configuration into the job's output file.
 *
 * @returns The InputData configuration as an XML string
 */
TString SInputData::GetStringConfig() const {

   // The result string:
   TString result;

   // Compose the "header" of the input data:
   result += TString::Format( "    <InputData Type=\"%s\"\n", m_type.Data() );
   result += TString::Format( "               Version=\"%s\"\n",
                              m_version.Data() );
   result += TString::Format( "               Lumi=\"%g\"\n",
                              m_totalLumiGiven );
   result += TString::Format( "               NEventsMax=\"%lld\"\n",
                              m_neventsmax );
   result += TString::Format( "               NEventsSkip=\"%lld\"\n",
                              m_neventsskip );
   result += TString::Format( "               Cacheable=\"%s\"\n",
                              ( m_cacheable ? "True" : "False" ) );
   result += TString::Format( "               SkipValid=\"%s\"\n",
                              ( m_skipValid ? "True" : "False" ) );
   result += TString::Format( "               SkipLookup=\"%s\">\n\n",
                              ( m_skipLookup ? "True" : "False" ) );

   // Add all the input files:
   std::vector< SFile >::const_iterator f_itr = m_sfileIn.begin();
   std::vector< SFile >::const_iterator f_end = m_sfileIn.end();
   for( ; f_itr != f_end; ++f_itr ) {
      result += TString::Format( "        <In FileName=\"%s\" Lumi=\"%g\"/>\n",
                                 f_itr->file.Data(), f_itr->lumi );
   }

   // Add all the input datasets:
   std::vector< SDataSet >::const_iterator d_itr = m_dataSets.begin();
   std::vector< SDataSet >::const_iterator d_end = m_dataSets.end();
   for( ; d_itr != d_end; ++d_itr ) {
      result += TString::Format( "        <DataSet Name=\"%s\" Lumi=\"%g\"/>\n",
                                 d_itr->name.Data(), d_itr->lumi );
   }

   // Add all the generator cuts:
   std::vector< SGeneratorCut >::const_iterator g_itr = m_gencuts.begin();
   std::vector< SGeneratorCut >::const_iterator g_end = m_gencuts.end();
   for( ; g_itr != g_end; ++g_itr ) {
      result += TString::Format( "        <GeneratorCut Tree=\"%s\" "
                                 "Formula=\"%s\"/>\n",
                                 g_itr->GetTreeName().Data(),
                                 g_itr->GetFormula().Data() );
   }

   // Add all the trees:
   const STreeTypeDecoder* decoder = STreeTypeDecoder::Instance();
   std::map< Int_t, std::vector< STree > >::const_iterator t_itr =
      m_trees.begin();
   std::map< Int_t, std::vector< STree > >::const_iterator t_end =
      m_trees.end();
   for( ; t_itr != t_end; ++t_itr ) {
      std::vector< STree >::const_iterator tt_itr = t_itr->second.begin();
      std::vector< STree >::const_iterator tt_end = t_itr->second.end();
      for( ; tt_itr != tt_end; ++tt_itr ) {
         result += TString::Format( "        <%s Name=\"%s\"/>\n",
                                    decoder->GetXMLName( t_itr->first ).Data(),
                                    tt_itr->treeName.Data() );
      }
   }

   // Close the input data block:
   result += "    </InputData>";

   // Return the constructed string:
   return result;
}

/**
 * This function looks at all the specified input files to make sure that they
 * exist, and to extract information about the trees inside of them.
 */
void SInputData::ValidateInputFiles() throw( SError ) {

   //
   // Set up the connection to the InputData cache if it's asked for:
   //
   TFile* cachefile = 0;
   TFileCollection* filecoll = 0;
   if( m_cacheable && ( ! m_skipValid ) ) {
      // The filename is hardcoded, since this is the only place where it's
      // needed:
      cachefile = TFile::Open( ".sframe." + GetType() + "." + GetVersion() +
                               ".idcache.root", "UPDATE" );
      m_logger << DEBUG << "Opened: " << cachefile->GetName()
               << SLogger::endmsg;
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
         m_logger << DEBUG << "Existing TFileCollection found"
                  << SLogger::endmsg;
      }
   }

   // Flag showing if the cache will have to be saved at the end of the
   // function:
   Bool_t cacheUpdated = kFALSE;
   Int_t  fileInfoInDataset = 0;

   //
   // Loop over all the specified input files:
   //
   std::vector< SFile >::iterator sf_itr = m_sfileIn.begin();
   std::vector< SFile >::iterator sf_end = m_sfileIn.end();
   for( ; sf_itr != sf_end; ++sf_itr ) {

      //
      // If it's a local file, then turn it into a full path name. This makes
      // PROOF-Lite much easier to use, as one can use a relative path in the
      // configuration file to define the input of the job. (Which in this case
      // would be the output of a pervious job.)
      //
      if( ( ! sf_itr->file.Contains( ":/" ) ) &&
          ( sf_itr->file[ 0 ] != '/' ) ) {
         sf_itr->file = gSystem->pwd() + ( "/" + sf_itr->file );
      }

      //
      // Try to load the file's information from the cache. This is *much*
      // faster than querying the file itself...
      //
      if( m_cacheable && LoadInfoOnFile( sf_itr.operator->(), filecoll ) ) {
         ++fileInfoInDataset;
         continue;
      }

      //
      // Open the physical file:
      //
      TFile* file = TFile::Open( sf_itr->file.Data(), "READ" );
      if( ! file || file->IsZombie() ) {
         m_logger << WARNING << "Couldn't open file: " << sf_itr->file.Data()
                  << SLogger::endmsg;
         m_logger << WARNING << "Removing it from the input file list"
                  << SLogger::endmsg;
         // Erasing the file from the file list is a bit tricky actually:
         sf_itr = m_sfileIn.erase( sf_itr );
         // Exit at this point if there are no more files in the dataset:
         if( ! m_sfileIn.size() ) break;
         --sf_itr;
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
            fileinfo = AccessFileInfo( sf_itr.operator->(), filecoll );
         }

         //
         // Investigate the input trees:
         //
         Bool_t firstPassed = kFALSE;
         Long64_t entries = 0;
         Int_t numberOfBranches = 0;
         // Try to find all the input trees in the file info:
         std::map< Int_t, std::vector< STree > >::const_iterator trees_itr =
            m_trees.begin();
         std::map< Int_t, std::vector< STree > >::const_iterator trees_end =
            m_trees.end();
         for( ; trees_itr != trees_end; ++trees_itr ) {

            m_logger << DEBUG << "Investigating \""
                     << STreeTypeDecoder::Instance()->GetName( trees_itr->first )
                     << "\" types" << SLogger::endmsg;

            std::vector< STree >::const_iterator st_itr =
               trees_itr->second.begin();
            std::vector< STree >::const_iterator st_end =
               trees_itr->second.end();
            for( ; st_itr != st_end; ++st_itr ) {

               // Only check the existence of input trees:
               if( ! ( st_itr->type & STree::INPUT_TREE ) ) continue;

               // Try to access the input tree:
               TTree* tree =
                  dynamic_cast< TTree* >( file->Get( st_itr->treeName ) );
               if( ! tree ) {
                  m_logger << WARNING << "Couldn't find tree "
                           << st_itr->treeName << " in file " << sf_itr->file
                           << SLogger::endmsg;
                  m_logger << WARNING
                           << "Removing file from the input file list"
                           << SLogger::endmsg;
                  throw SError( SError::SkipFile );
               }

               // Remember how many branches there are in total in the input:
               const Int_t branchesThisTree = tree->GetNbranches();
               m_logger << DEBUG << branchesThisTree << " branches in tree "
                        << st_itr->treeName << SLogger::endmsg;
               numberOfBranches += branchesThisTree;

               // Check how many events are there in the input:
               if( st_itr->type & STree::EVENT_TREE ) {
                  if( firstPassed && ( tree->GetEntriesFast() != entries ) ) {
                     m_logger << WARNING
                              << "Conflict in number of entries - Tree "
                              << st_itr->treeName << " has "
                              << tree->GetEntriesFast() << " entries, NOT "
                              << entries << SLogger::endmsg;
                     m_logger << WARNING << "Removing " << sf_itr->file
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
                  TFileInfoMeta* tree_info =
                     new TFileInfoMeta( st_itr->treeName, "TTree",
                                        tree->GetEntriesFast() );
                  tree_info->SetName( st_itr->treeName );
                  tree_info->SetTitle( "Meta data info for a TTree" );
                  if( ! fileinfo->AddMetaData( tree_info ) ) {
                     REPORT_ERROR( "There was a problem caching meta-data for "
                                   "TTree: " << st_itr->treeName );
                  } else {
                     REPORT_VERBOSE( "Meta-data cached for TTree: "
                                     << st_itr->treeName );
                  }
               }
            }
         }

         // Update the ID information:
         sf_itr->events = entries;
         AddEvents( entries );

         m_logger << DEBUG << numberOfBranches << " branches in total in file "
                  << file->GetName() << SLogger::endmsg;

      } catch( const SError& ) {
         m_totalLumiSum -= sf_itr->lumi;
         sf_itr = m_sfileIn.erase( sf_itr );
         // Exit at this point if there are no more files in the dataset:
         if( ! m_sfileIn.size() ) break;
         --sf_itr;
      }

      // Close the input file:
      file->Close();
      delete file;
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
         REPORT_VERBOSE( "Writing file collection object to cache" );
         cachefile->cd();
         if( filecoll->Update() == -1 ) {
            REPORT_ERROR( "Failed to update the cached information" );
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
                          " Id Type: " + GetType() + ", Version: " +
                          GetVersion(), SError::StopExecution );
         }

         // Check if the current configuration is likely to be described by this
         // dataset:
         if( fileInfoInDataset == m_dset->GetListOfElements()->GetSize() ) {
            m_logger << DEBUG << "The loaded dataset is up to date"
                     << SLogger::endmsg;
         } else {
            m_logger << DEBUG << "The dataset has to be updated"
                     << SLogger::endmsg;
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
   // Check that the specified maximum number of events and the number of events
   // to skip, make sense:
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

/**
 * This function is used to collect information about each file in the specified
 * input PQ2 dataset(s).
 *
 * @param pserver The PROOF server to use in the validation
 */
void SInputData::ValidateInputDataSets( const char* pserver ) throw( SError ) {

   // Connect to the PROOF server:
   TProof* server = SProofManager::Instance()->Open( pserver );

   // Check the number of defined datasets. It's only possible to use multiple
   // datasets in a single InputData starting from ROOT 5.27/02. In previous
   // releases only the first one can be used.
   if( ( ROOT_VERSION_CODE < ROOT_VERSION( 5, 27, 02 ) ) &&
       // The special PROOF branch of the ROOT development code can also be
       // used:
       ( strcmp( gROOT->GetVersion(), "5.26/00-proof" ) ) &&
       ( m_dataSets.size() > 1 ) ) {

      m_logger << WARNING << "You're currently using ROOT version: "
               << gROOT->GetVersion() << "\n"
               << "This version doesn't yet support defining multiple\n"
               << "datasets per InputData. Only the first one is going to be"
               << "used!" << SLogger::endmsg;
      m_logger << WARNING
               << "To use multiple datasets, upgrate to at least ROOT 5.27/02"
               << SLogger::endmsg;
      m_dataSets.resize( 1 );
      m_totalLumiSum = m_dataSets.front().lumi;
   }

   //
   // Loop over the specified datasets:
   //
   std::vector< SDataSet >::iterator ds_itr = m_dataSets.begin();
   std::vector< SDataSet >::iterator ds_end = m_dataSets.end();
   for( ; ds_itr != ds_end; ++ds_itr ) {

      try {

         //
         // Check if the dataset exists on the server:
         //
         TFileCollection* filecoll = server->GetDataSet( ds_itr->name );
         if( ! filecoll ) {
            REPORT_ERROR( "Dataset \"" << ds_itr->name
                          << "\" doesn't exist on server: " << pserver );
            throw SError( SError::SkipFile );
         }

         //
         // Investigate the input trees:
         //
         Bool_t firstPassed = kFALSE;
         Long64_t entries = 0;
         std::map< Int_t, std::vector< STree > >::const_iterator trees_itr =
            m_trees.begin();
         std::map< Int_t, std::vector< STree > >::const_iterator trees_end =
            m_trees.end();
         for( ; trees_itr != trees_end; ++trees_itr ) {

            m_logger << DEBUG << "Investigating \""
                     << STreeTypeDecoder::Instance()->GetName( trees_itr->first )
                     << "\" types" << SLogger::endmsg;

            std::vector< STree >::const_iterator st_itr =
               trees_itr->second.begin();
            std::vector< STree >::const_iterator st_end =
               trees_itr->second.end();
            for( ; st_itr != st_end; ++st_itr ) {

               // Only check the existence of input trees:
               if( ! ( st_itr->type & STree::INPUT_TREE ) ) continue;

               // Don't check for trees in sub-directories:
               if( st_itr->treeName.Contains( "/" ) ) continue;

               // Try to access information on the input tree:
               const Long64_t tree_entries =
                  filecoll->GetTotalEntries( "/" + st_itr->treeName );
               if( tree_entries == -1 ) {
                  REPORT_ERROR( "Couldn't find tree " << st_itr->treeName
                                << " in dataset " << ds_itr->name );
                  REPORT_ERROR( "Removing dataset from the input list" );
                  throw SError( SError::SkipFile );
               }

               // Check how many events are there in the input:
               if( st_itr->type & STree::EVENT_TREE ) {
                  if( firstPassed && ( tree_entries != entries ) ) {
                     m_logger << WARNING
                              << "Conflict in number of entries - Tree "
                              << st_itr->treeName << " has " << tree_entries
                              << " entries, NOT " << entries << SLogger::endmsg;
                     m_logger << WARNING << "Removing " << ds_itr->name
                              << " from the input dataset list"
                              << SLogger::endmsg;
                     throw SError( SError::SkipFile );
                  } else if( ! firstPassed ) {
                     firstPassed = kTRUE;
                     entries = tree_entries;
                  }
               }
            }
         }

         // Update the ID information:
         ds_itr->events = entries;
         AddEvents( entries );

      } catch( const SError& ) {
         m_totalLumiSum -= ds_itr->lumi;
         ds_itr = m_dataSets.erase( ds_itr );
         --ds_itr;
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

/**
 * This function is used internally to load information on a given
 * file that will be used as input. It only accesses information coming from
 * the metadata about the input files.
 *
 * @param file The file that should be checked
 * @param filecoll The metadata that is available about the input files
 * @returns <code>kTRUE</code> if all information is available, and could be
 *          loaded; <code>kFALSE</code> if not.
 */
Bool_t SInputData::LoadInfoOnFile( SFile* file, TFileCollection* filecoll ) {

   // Retrieve the information about this specific file:
   TObject* tinfo = filecoll->GetList()->FindObject( file->file );
   TFileInfo* fileinfo = dynamic_cast< TFileInfo* >( tinfo );
   if( ! fileinfo ) {
      REPORT_VERBOSE( "File unknown: " << file->file );
      return kFALSE;
   }

   m_logger << DEBUG << "Information found for: " << file->file
            << SLogger::endmsg;

   // Flag showing if we already know the number of entries:
   Bool_t firstPassed = kFALSE;
   // Number of entries in the file:
   Long64_t entries = 0;

   //
   // Check that information is available on all the input trees in the cache:
   //
   std::map< Int_t, std::vector< STree > >::const_iterator trees_itr =
      m_trees.begin();
   std::map< Int_t, std::vector< STree > >::const_iterator trees_end =
      m_trees.end();
   for( ; trees_itr != trees_end; ++trees_itr ) {
      std::vector< STree >::const_iterator st_itr = trees_itr->second.begin();
      std::vector< STree >::const_iterator st_end = trees_itr->second.end();
      for( ; st_itr != st_end; ++st_itr ) {

         // Only check the existence of input trees:
         if( ! ( st_itr->type & STree::INPUT_TREE ) ) continue;

         // Get the tree information:
         TFileInfoMeta* tree_info = fileinfo->GetMetaData( st_itr->treeName );
         if( ! tree_info ) {
            m_logger << DEBUG << "No description found for: "
                     << st_itr->treeName << SLogger::endmsg;
            return kFALSE;
         }

         // Check how many events are there in the input:
         if( st_itr->type & STree::EVENT_TREE ) {
            if( ! firstPassed ) {
               firstPassed = kTRUE;
               entries = tree_info->GetEntries();
            } else if( entries != tree_info->GetEntries() ) {
               m_logger << WARNING << "Inconsistent cached data for: "
                        << file->file << " -> Checking the file again..."
                        << SLogger::endmsg;
               return kFALSE;
            }
         }
      }
   }

   //
   // Update the ID with this information:
   //
   file->events = entries;
   AddEvents( entries );

   // Everything was successful:
   return kTRUE;
}

/**
 * This function is just used internally to access the metadata object
 * describing a given input file. If such metadata doesn't exist yet, the
 * function creates the metadata object.
 *
 * @param file The file to access the metadata about
 * @param filecoll Object holding metadata about all the known input files
 * @returns A pointer to the metadata describing the requested input file
 */
TFileInfo* SInputData::AccessFileInfo( SFile* file,
                                       TFileCollection* filecoll ) {

   // Try to access the metadata object:
   TObject* tresult = filecoll->GetList()->FindObject( file->file );
   TFileInfo* result = dynamic_cast< TFileInfo* >( tresult );

   // Check if we know anything about this file already:
   if( result ) {
      m_logger << DEBUG << "Updating information for " << file->file
               << SLogger::endmsg;
   } else {
      // One has to be very verbose in naming the object, otherwise the stupid
      // ROOT container will not be able to find it afterwards...
      m_logger << DEBUG << "Creating information for " << file->file
               << SLogger::endmsg;
      result = new TFileInfo( file->file );
      result->SetName( file->file );
      result->SetTitle( "Description for: " + file->file );
      filecoll->Add( result );
   }

   // Return the requested object:
   return result;
}

/**
 * This function is used to make a validated dataset object out of the specified
 * input files. This dataset is then used to process the file using PROOF.
 *
 * It can also be persistified into a metadata cache file to speed up the
 * subsequent processing of the same input files.
 *
 * @returns A validated dataset made from the input files
 */
TDSet* SInputData::MakeDataSet() const throw( SError ) {

   // Find the name of the "main" TTree in the files:
   const char* treeName = 0;
   std::map< Int_t, std::vector< STree > >::const_iterator trees_itr =
      m_trees.begin();
   std::map< Int_t, std::vector< STree > >::const_iterator trees_end =
      m_trees.end();
   for( ; trees_itr != trees_end; ++trees_itr ) {
      std::vector< STree >::const_iterator st_itr = trees_itr->second.begin();
      std::vector< STree >::const_iterator st_end = trees_itr->second.end();
      for( ; st_itr != st_end; ++st_itr ) {
         if( ( st_itr->type & STree::INPUT_TREE ) &&
             ( st_itr->type & STree::EVENT_TREE ) ) {
            treeName = st_itr->treeName.Data();
         }
      }
   }
   if( ! treeName ) {
      REPORT_ERROR( "Can't determine input TTree name for InputData with type: "
                    << GetType() << ", version: " << GetVersion() );
      throw SError( "Can't determine input TTree name!",
                    SError::SkipInputData );
   }

   // The dataset is created in two different ways depending on whether we
   // want (mostly XRootD) files to be looked up by ROOT, or their locations
   // should be taken as they were specified in the configuration.
   if( GetSkipLookup() ) {
      // Create the dataset:
      TDSet* result = new TDSet( "DSetCache", treeName );
      std::vector< SFile >::const_iterator file_itr = GetSFileIn().begin();
      std::vector< SFile >::const_iterator file_end = GetSFileIn().end();
      for( ; file_itr != file_end; ++file_itr ) {
         result->Add( file_itr->file );
      }
      result->SetTitle( "Cached dataset for ID Type: " + GetType() +
                        ", Version: " + GetVersion() );
      result->SetLookedUp();
      result->Validate();

      // Return the object:
      return result;
   } else {
      // Create a TChain that will be the basis of the dataset:
      TChain chain( treeName );
      std::vector< SFile >::const_iterator file_itr = GetSFileIn().begin();
      std::vector< SFile >::const_iterator file_end = GetSFileIn().end();
      for( ; file_itr != file_end; ++file_itr ) {
         chain.Add( file_itr->file );
      }

      // Create the dataset:
      TDSet* result = new TDSet( chain );
      result->SetName( "DSetCache" );
      result->SetTitle( "Cached dataset for ID Type: " + GetType() +
                        ", Version: " + GetVersion() );
      result->Validate();

      // Return the object:
      return result;
   }
}

/**
 * This is a very trivial function for hard-coding in a single place how
 * the dataset information is stored by SFrame in the metadata cache files.
 *
 * @param dir Directory in which to look for the dataset object
 * @returns The dataset object pointer if it could be found, or a null pointer
 *          if not
 */
TDSet* SInputData::AccessDataSet( TDirectory* dir ) const {

   return dynamic_cast< TDSet* >( dir->Get( "DSetCache" ) );
}
