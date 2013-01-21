// Dear emacs, this is -*- c++ -*-
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

#ifndef SFRAME_CORE_SInputData_H
#define SFRAME_CORE_SInputData_H

// STL include(s):
#include <vector>
#include <map>

// ROOT include(s):
#include <TObject.h>
#include <TNamed.h>

// Local include(s):
#include "SGeneratorCut.h"
#include "SLogger.h"
#include "SError.h"

// Forward declaration(s):
class TFileCollection;
class TFileInfo;
class TDirectory;
class TDSet;

/**
 *   @short Class describing one input PROOF dataset for a cycle
 *
 *          This class is used internally to handle datasets as inputs to the
 *          analysis cycles. Use the "sframe_dset_*" functions to find out the
 *          exact names of the datasets.
 *
 * @version $Revision$
 */
class SDataSet : public TObject {

public:
   /// Default constructor
   SDataSet( const TString& n = "", Double_t l = 1.0 )
      : name( n ), lumi( l ), events( 0 ) {}

   /// Assignment operator
   SDataSet& operator= ( const SDataSet& parent );
   /// Equality operator
   Bool_t operator== ( const SDataSet& rh ) const;
   /// Non-equality operator
   Bool_t operator!= ( const SDataSet& rh ) const;

   /// Dataset name
   /**
    * This should be the fully qualified name of the dataset.
    */
   TString name;
   /// Luminosity of the dataset
   /**
    * Every dataset is assigned a luminosity. This is used to calculate
    * the correct event weights for SCycleBase::ExecuteEvent to normalise
    * the different Monte Carlos correctly to each other.
    */
   Double_t lumi;
   /// Number of events in the dataset
   /**
    * This property is calculated by the framework in
    * SCycleBase::CheckInputFiles. It is used for calculating the correct
    * weights when only a specified number of events should be processed
    * from a dataset. (So the luminosity of the dataset has to be weighted.)
    */
   Long64_t events;

#ifndef DOXYGEN_IGNORE
   ClassDef( SDataSet, 1 )
#endif // DOXYGEN_IGNORE

}; // class SDataSet

/**
 *   @short Class describing an input file to the analysis.
 *
 *          It is used to describe one input or output file (as defined
 *          in the configuration XML file) to the framework.
 *
 * @version $Revision$
 */
class SFile : public TObject {

public:
   /// Default constructor
   SFile()
      : file( "" ), lumi( -1 ), events( 0 ){}
   /// Constructor with a file name
   SFile( const TString& f )
      : file( f ), lumi( -1 ), events( 0 ){}
   /// Constructor with a file name and a luminosity
   SFile( const TString& f, Double_t l )
      : file( f ), lumi( l ), events( 0 ){}

   /// Assignment operator
   SFile& operator=  ( const SFile& parent );
   /// Equality operator
   Bool_t operator== ( const SFile& rh ) const;
   /// Non-equality operator
   Bool_t operator!= ( const SFile& rh ) const;

   /// The file name
   /**
    * Not much to say about this, this is the full name of the input file.
    */
   TString  file;
   /// Luminosity of the file
   /**
    * Every input file is assigned a luminosity. This is used to calculate
    * the correct event weights for SCycleBase::ExecuteEvent to normalise
    * the different Monte Carlos correctly to each other.
    */
   Double_t lumi;
   /// Number of events in the file
   /**
    * This property is calculated by the framework in
    * SCycleBase::CheckInputFiles. It is used for calculating the correct
    * weights when only a specified number of events should be processed
    * from a dataset. (So the luminosity of the dataset has to be weighted.)
    */
   Long64_t events;

#ifndef DOXYGEN_IGNORE
   ClassDef( SFile, 1 )
#endif // DOXYGEN_IGNORE

}; // class SFile

/**
 *   @short Class describing a "simple" input tree in the input file(s).
 *
 *          This class describes an input or output TTree that is used
 *          by the analysis to the framework. The TTree only has one
 *          property actually, its name. The name of the tree is taken
 *          from the configuration XML file.
 *
 * @version $Revision$
 */
class STree : public TObject {

public:
   /// Constructor with a tree name
   STree( const TString& name = "", Int_t typ = 0 )
      : treeName( name ), type( typ ) {}

   /// Assignment operator
   STree& operator=  ( const STree& parent );
   /// Equality operator
   Bool_t operator== ( const STree& rh ) const;
   /// Non-equality operator
   Bool_t operator!= ( const STree& rh ) const;

   /// Name of the tree
   /**
    * The only thing that has to be specified for an input or output tree
    * in the configuration XML is its name. This is under which an input
    * tree will be looked for, or under which a new output tree will
    * be created.
    */
   TString treeName;

   static const Int_t INPUT_TREE; ///< This is an input tree
   static const Int_t OUTPUT_TREE; ///< This is an output tree
   static const Int_t EVENT_TREE; ///< This tree has one entry per event

   /// Type of this tree
   /**
    * This bitmask is used internally to descibe all the trees that SFrame
    * can handle. All SFrame needs to know at this point is if a tree is
    * input or output, and whether it descibes event lever data or not.
    */
   Int_t type;

#ifndef DOXYGEN_IGNORE
   ClassDef( STree, 1 )
#endif // DOXYGEN_IGNORE

}; // class STree

/**
 *   @short Class describing one kind of input data.
 *
 *          This class is used to describe all the properties (files, trees
 *          in the files, etc.) if an input data type. It is created
 *          by the framework from the configuration values put in the
 *          configuration XML file.
 *
 * @version $Revision$
 */
class SInputData : public TNamed {

public:
   /// Default constructor
   SInputData( const char* name = "SInputData" );
   /// Default desctructor
   virtual ~SInputData();

   /// Set the name of the input data type
   void SetType         ( const TString& type )    { m_type = type; }
   /// Set the "version" of the input data
   void SetVersion      ( const TString& version ) { m_version = version; }
   /// Set the total luminosity of the input data
   void SetTotalLumi    ( Double_t lumi )          { m_totalLumiGiven = lumi; }
   /// Set the maximal number of events to process from the input data
   void SetNEventsMax   ( Long64_t nevents )       { m_neventsmax = nevents; }
   /// Set the number of events to skip at the beginning of the input data
   void SetNEventsSkip  ( Long64_t nevents )       { m_neventsskip = nevents; }

   /// Set whether the file properties can be cached
   void SetCacheable( Bool_t flag = kTRUE )        { m_cacheable = flag; }
   /// Get whether the file properties can be caches
   Bool_t GetCacheable() const                     { return m_cacheable; }

   /// Set whether the file/dataset validation can be skipped
   void SetSkipValid( Bool_t flag = kTRUE )        { m_skipValid = flag; }
   /// Get whether the file/dataset validation can be skipped
   Bool_t GetSkipValid() const                     { return m_skipValid; }

   /// Set whether the file lookup during dataset validation can be skipped
   void SetSkipLookup( Bool_t flag = kTRUE )       { m_skipLookup = flag; }
   /// Get whether the file lookup during dataset validation can be skipped
   Bool_t GetSkipLookup() const                    { return m_skipLookup; }

   /// Set the current entry which is being read from the input
   void SetEventTreeEntry( Long64_t entry )        { m_entry = entry; }
   /// Get the current entry which is being read from the input
   Long64_t GetEventTreeEntry() const              { return m_entry; }

   /// Add a new generator cut to the input data
   void AddGenCut( const SGeneratorCut& gencuts ) {
      m_gencuts.push_back( gencuts );
   }
   /// Add a new input file to the input data
   void AddSFileIn( const SFile& sfile );
   /// Add a new tree to the input data
   void AddTree( Int_t type, const STree& stree );
   /// Add a new dataset to the input data
   void AddDataSet( const SDataSet& dset );

   /// Add some number of events to the input data
   void AddEvents( Long64_t events ) { m_eventsTotal += events; }

   /// Collect information about the input files (needed before running)
   void ValidateInput( const char* pserver = 0 ) throw( SError );

   /// Get the name of the input data type
   const TString& GetType() const { return m_type; }
   /// Get the version of the input data type
   const TString& GetVersion() const { return m_version; }
   /// Get all the defined generator cuts
   const std::vector< SGeneratorCut >& GetSGeneratorCuts() const {
      return m_gencuts;
   }
   /// Get all the defined input files
   const std::vector< SFile >& GetSFileIn() const { return m_sfileIn; }
   /// Get all the defined input files
   std::vector< SFile >& GetSFileIn() { return m_sfileIn; }
   /// Get all the defined trees of a given type
   const std::vector< STree >* GetTrees( Int_t type ) const;
   /// Get all the defined trees
   const std::map< Int_t, std::vector< STree > >& GetTrees() const {
      return m_trees;
   }
   /// Get all the defined input datasets
   const std::vector< SDataSet >& GetDataSets() const { return m_dataSets; }

   /// Simple function answering whether there are any input trees in the configuration
   Bool_t HasInputTrees() const;

   /// Get the dataset representing all the input files
   TDSet* GetDSet() const;

   /// Get the total luminosity of the input data
   Double_t GetTotalLumi() const;
   /// Get the total luminosity scaled to the number of events to process
   Double_t GetScaledLumi() const;
   /// Get the total number of events in the input data files
   Long64_t GetEventsTotal() const { return m_eventsTotal; }
   /// Get the maximal number of events to process from the input data
   Long64_t GetNEventsMax() const  { return m_neventsmax; }
   /// Get the number of events to skip at the beginning of the input data
   Long64_t GetNEventsSkip() const { return m_neventsskip; }

   /// Assignment operator
   SInputData& operator=  ( const SInputData& parent );
   /// Equality operator
   Bool_t      operator== ( const SInputData& rh ) const;
   /// Non-equality operator
   Bool_t      operator!= ( const SInputData& rh ) const;

   /// Function printing the contents of the object
   void Print( const Option_t* opt = 0 ) const;

   /// Get the input data configuration as a TString object
   TString GetStringConfig() const;

private:
   /// This function validates the input when files are specified
   void ValidateInputFiles() throw( SError );
   /// This function validates the input when PQ2 datasets are specified
   void ValidateInputDataSets( const char* pserver ) throw( SError );
   /// Function loading all information about a given input file
   Bool_t LoadInfoOnFile( SFile* file, TFileCollection* filecoll );
   /// Function accessing the metadata about a given input file
   TFileInfo* AccessFileInfo( SFile* file, TFileCollection* filecoll );
   /// Function creating a new dataset object for this input data object
   TDSet* MakeDataSet() const throw( SError );
   /// Function trying to access the dataset object in a given directory
   TDSet* AccessDataSet( TDirectory* dir ) const;

   TString m_type; ///< Type of the input data
   TString m_version; ///< Version of the input data
   Double_t m_totalLumiGiven; ///< The total specified luminosity
   std::vector< SGeneratorCut > m_gencuts; ///< The specified generator cuts
   std::vector< SFile > m_sfileIn; ///< The specified input files
   /// The specified TTree-s to handle
   std::map< Int_t, std::vector< STree > > m_trees;
   std::vector< SDataSet > m_dataSets; ///< The specified input PROOF datasets
   Double_t m_totalLumiSum; ///< The total luminosity from files/datasets
   Long64_t m_eventsTotal; ///< The total number of events in the input
   Long64_t m_neventsmax; ///< The maximum number of events to process
   Long64_t m_neventsskip; ///< The number of events to skip
   Bool_t m_cacheable; ///< Flag showing whether to cache the ID info
   Bool_t m_skipValid; ///< Flag showing whether to skip the ID validation
   /// Flag showing whether to skip the file lookup during dataset validation
   Bool_t m_skipLookup;
   Long64_t m_entry; ///< Current entry read from the input

   TDSet* m_dset; //! Transient dataset representation of input files

   mutable SLogger m_logger; //! Transient logger object

#ifndef DOXYGEN_IGNORE
   ClassDef( SInputData, 1 )
#endif // DOXYGEN_IGNORE

}; // class SInputData

#endif // SFRAME_CORE_SInputData_H
