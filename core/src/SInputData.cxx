// $Id: SInputData.cxx,v 1.4.2.2 2009-01-08 16:09:32 krasznaa Exp $
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

// Local include(s):
#include "../include/SInputData.h"
#include "../include/SError.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SFile );
ClassImp( STree );
ClassImp( SEVTree );
ClassImp( SInputData );
#endif // DOXYGEN_IGNORE

using namespace std;

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
 * It is only necessary for some technical affairs.
 */
SEVTree& SEVTree::operator= ( const SEVTree& parent ) {

   this->treeName = parent.treeName;
   this->treeBaseName = parent.treeBaseName;
   this->viewNumber = parent.viewNumber;
   this->collTreeName = parent.collTreeName;

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
Bool_t SEVTree::operator== ( const SEVTree& rh ) const {

   if( ( this->treeName == rh.treeName ) &&
       ( this->treeBaseName == rh.treeBaseName ) && ( this->viewNumber == rh.viewNumber ) &&
       ( this->collTreeName == rh.collTreeName ) ) {
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
Bool_t SEVTree::operator!= ( const SEVTree& rh ) const {

   return ( ! ( *this == rh ) );

}

/**
 * The constructor initialises all member data to some initial value.
 */
SInputData::SInputData( const char* name )
   : TNamed( name, "SFrame input data object" ), m_type( "unknown" ),
     m_version( 0 ), m_totalLumiGiven( 0 ), m_totalLumiSum( 0 ),
     m_eventsTotal( 0 ), m_neventsmax( -1 ), m_neventsskip( 0 ),
     m_logger( "SInputData" ) {

   m_logger << VERBOSE << "In constructor" << SLogger::endmsg;
}

/**
 * Another one of the "I don't do anything" destructors.
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

void SInputData::ValidateInput() {

   for( std::vector< SFile >::iterator sf = m_sfileIn.begin(); sf != m_sfileIn.end(); ++sf ) {

      TFile* file = TFile::Open( sf->file.Data() );
      if( ! file || file->IsZombie() ) {
         m_logger << WARNING << "Couldn't open file: " << sf->file.Data() << SLogger::endmsg;
         m_logger << WARNING << "Removing it from the input file list" << SLogger::endmsg;
         m_sfileIn.erase( sf );
         continue;
      }

      try {

         Bool_t firstPassed = kFALSE;
         Long64_t entries = 0;
         Int_t numberOfBranches = 0;
         // try to load all the input trees
         for( std::vector< STree >::const_iterator st = m_inputTrees.begin();
              st != m_inputTrees.end(); ++st ) {
            TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
            if( ! tree ) {
               m_logger << WARNING << "Couldn't find tree " << st->treeName
                        << "in file " << sf->file.Data() << SLogger::endmsg;
               m_logger << WARNING << "Removing file from the input file list"
                        << SLogger::endmsg;
               throw SError( SError::SkipFile );
            } else {
               if( firstPassed && tree->GetEntriesFast() != entries ) {
                  m_logger << WARNING << "Conflict in number of entries - Tree "
                           << tree->GetName() << " has " << tree->GetEntries()
                           << ", NOT " << entries << SLogger::endmsg;
                  m_logger << WARNING << "Removing " << sf->file.Data()
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
         }

         // check EV trees
         for( std::vector< SEVTree >::const_iterator st = m_evInputTrees.begin();
              st != m_evInputTrees.end(); ++st ) {
            TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
            if( ! tree ) {
               m_logger << WARNING << "Couldn't find tree " << st->treeName
                        << "in file " << sf->file.Data() << SLogger::endmsg;
               m_logger << WARNING << "Removing file from the input file list"
                        << SLogger::endmsg;
               throw SError( SError::SkipFile );
            }
            Int_t branchesThisTree = tree->GetNbranches();
            m_logger << DEBUG << branchesThisTree << " branches in tree " << tree->GetName() 
                     << SLogger::endmsg;
            numberOfBranches += branchesThisTree;
         }

         // Check the persistent tree(s):
         for( std::vector< STree >::const_iterator st = m_persTrees.begin();
              st != m_persTrees.end(); ++st ) {
            TTree* tree = dynamic_cast< TTree* >( file->Get( st->treeName ) );
            if( ! tree ) {
               m_logger << WARNING << "Couldn't find tree " << st->treeName
                        << "in file " << sf->file.Data() << SLogger::endmsg;
               m_logger << WARNING << "Removing file from the input file list"
                        << SLogger::endmsg;
               throw SError( SError::SkipFile );
            } else {
               if( firstPassed && tree->GetEntriesFast() != entries ) {
                  m_logger << WARNING << "Conflict in number of entries - Tree "
                           << tree->GetName() << " has " << tree->GetEntries()
                           << ", NOT " << entries << SLogger::endmsg;
                  m_logger << WARNING << "Removing " << sf->file.Data()
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
         }

         sf->events = entries;
         AddEvents( entries );

         m_logger << DEBUG << numberOfBranches << " branches in total in file "
                  << file->GetName() << SLogger::endmsg;

      } catch( const SError& ) {
         m_sfileIn.erase( sf );
         continue;
      }

      file->Close();
      if( file ) delete file;
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
            << SLogger::endmsg;

   return;

}

Double_t SInputData::GetTotalLumi() const { 
  
   Double_t return_lumi = 0.;
   // use the given luminosity for this InputData in case it is specified
   if( m_totalLumiGiven ) return_lumi = m_totalLumiGiven;
   // otherwise use the sum of all files
   else return_lumi = m_totalLumiSum;
  
   // make sure that the lumi is not zero
   if( ! return_lumi ) 
      m_logger << FATAL << "total luminosity for "<< GetType() << "is ZERO!"
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
   this->m_evInputTrees = parent.m_evInputTrees;
   this->m_outputTrees = parent.m_outputTrees;
   this->m_totalLumiSum = parent.m_totalLumiSum;
   this->m_eventsTotal = parent.m_eventsTotal;
   this->m_neventsmax = parent.m_neventsmax;
   this->m_neventsskip = parent.m_neventsskip;

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
       ( this->m_evInputTrees == rh.m_evInputTrees ) &&
       ( this->m_outputTrees == rh.m_outputTrees ) &&
       ( this->m_totalLumiSum == rh.m_totalLumiSum ) &&
       ( this->m_eventsTotal == rh.m_eventsTotal ) &&
       ( this->m_neventsmax == rh.m_neventsmax ) &&
       ( this->m_neventsskip == rh.m_neventsskip ) ) {
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

   for( vector< SGeneratorCut >::const_iterator gc = m_gencuts.begin();
        gc != m_gencuts.end(); ++gc )
      m_logger << " Generator cut      : Tree:" << gc->GetTreeName() << " Formula: "
               << gc->GetFormula() << endl;

   for( vector< SFile >::const_iterator f = m_sfileIn.begin(); f != m_sfileIn.end();
        ++f )
      m_logger << " Input SFiles       : " << "'" << f->file << "' (file) | '" << f->lumi
               << "' (lumi) " << endl;
   for( std::vector< STree >::const_iterator t = m_inputTrees.begin();
        t != m_inputTrees.end(); ++t )
      m_logger << " Input tree         : " << "'" << t->treeName << "'" << endl;
   for( std::vector< STree >::const_iterator t = m_persTrees.begin();
        t != m_persTrees.end(); ++t )
      m_logger << " Persistent tree    : " << "'" << t->treeName << "'" << endl;
   for( std::vector< SEVTree >::const_iterator t = m_evInputTrees.begin();
        t != m_evInputTrees.end(); ++t )
      m_logger << " EV Input tree      : " << "'" << t->treeName << "' (tree) | '"
               << t->treeBaseName << "' (base name) | '" << t->collTreeName
               << "' (coll. tree name)" << endl;
   for( std::vector< STree >::const_iterator t = m_outputTrees.begin();
        t != m_outputTrees.end(); ++t )
      m_logger << " Output tree        : " << "'" << t->treeName << "'" << endl;

   m_logger << " ---------------------------------------------------------" << SLogger::endmsg;

   return;

}
