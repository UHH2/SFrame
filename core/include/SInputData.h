// Dear emacs, this is -*- c++ -*-
// $Id: SInputData.h,v 1.4 2008-10-14 09:45:26 krasznaa Exp $
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

#ifndef SFRAME_CORE_SInputData_H
#define SFRAME_CORE_SInputData_H

// STL include(s):
#include <vector>

// ROOT include(s):
#include "TString.h"

// Local include(s):
#include "SGeneratorCut.h"
#include "SLogger.h"

/**
 *   @short Class describing an input file to the analysis.
 *
 *          It is used to describe one input or output file (as defined
 *          in the configuration XML file) to the framework.
 *
 * @version $Revision: 1.4 $
 */
class SFile {

public:
   /// Default constructor
   SFile()
      : file( "" ), lumi( -1 ) , events( 0 ){}
   /// Constructor with a file name
   SFile( const TString& f )
      : file( f ), lumi( -1 ) , events( 0 ){}
   /// Constructor with a file name and a luminosity
   SFile( const TString& f, Double_t l )
      : file( f ), lumi( l ) , events( 0 ){}

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

}; // class SFile

/**
 *   @short Class describing a "simple" input tree in the input file(s).
 *
 *          This class describes an input or output TTree that is used
 *          by the analysis to the framework. The TTree only has one
 *          property actually, its name. The name of the tree is taken
 *          from the configuration XML file.
 *
 * @version $Revision: 1.4 $
 */
class STree {

public:
   /// Constructor with a tree name
   STree( const TString& t )
      : treeName( t ) {}

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

}; // class STree

/**
 *   @short Class describing an EventView input tree in the input file(s).
 *
 *          This class describes an input EventView tree. (It is not
 *          possible to output such trees with SFrame.) It is only used
 *          when multiple views have been created for the same event.
 *          (For instance by the top reconstruction code in TopView.)
 *          EventView trees have a number of properties besides their
 *          name, all of which are needed to perform synchronisation
 *          between the trees.
 *
 * @version $Revision: 1.4 $
 */
class SEVTree {

public:
   /// Constructor with all the EVTree properties
   SEVTree( const TString& t, const TString& tbasename, Int_t viewnum,
            const TString& colltname )
      : treeName( t ), treeBaseName( tbasename ),
        viewNumber( viewnum ), collTreeName( colltname ) {}

   /// Assignment operator
   SEVTree& operator=  ( const SEVTree& parent );
   /// Equality operator
   Bool_t   operator== ( const SEVTree& rh ) const;
   /// Non-equality operator
   Bool_t   operator!= ( const SEVTree& rh ) const;

   /// Name of the tree
   /**
    * This is the full name of the EventView tree. (For instance "FullRecoAna2",
    * "MyAna3", etc.)
    */
   TString treeName;
   /// "Base name" of the tree
   /**
    * Every view of a given type has the same "base name". This is the name
    * after which a number is put to show the view number. It can be something
    * like: "FullRecoAna", "EV", etc.
    */
   TString treeBaseName;
   /// Number of the view
   /**
    * Every view is assigned a "view number". This number is put after the
    * base name to form the full tree name. It's useful to store this
    * as a separate variable for the tree synchronisation.
    */
   Int_t   viewNumber;
   /// Name of the "collection tree"
   /**
    * The EventView trees are synchronised using variables from a so called
    * collection tree. This tree holds the variable(s) showing how many views
    * were created for a given event. The name is practically always
    * "CollectionTree", but it was better to keep it as a variable for
    * bigger flexibility.
    */
   TString collTreeName;

}; // class SEVTree

/**
 *   @short Class describing one kind of input data.
 *
 *          This class is used to describe all the properties (files, trees
 *          in the files, etc.) if an input data type. It is created
 *          by the framework from the configuration values put in the
 *          configuration XML file.
 *
 * @version $Revision: 1.4 $
 */
class SInputData {

public:
   /// Default constructor
   SInputData();
   /// Default desctructor
   ~SInputData();

   /// Set the name of the input data type
   void SetType         ( const TString& type )          { m_type = type; }
   /// Set the "version" of the input data
   void SetVersion      ( const TString& version )       { m_version = version; }
   /// Set the total luminosity of the input data
   void SetTotalLumi    ( Double_t lumi )                { m_totalLumiGiven = lumi; }
   /// Set the maximal number of events to process from the input data
   void SetNEventsMax   ( Long64_t nevents )             { m_neventsmax = nevents; }
   /// Set the number of events to skip at the beginning of the input data
   void SetNEventsSkip  ( Long64_t nevents )             { m_neventsskip = nevents; }

   /// Add a new generator cut to the input data
   void AddGenCut       ( const SGeneratorCut& gencuts ) { m_gencuts.push_back( gencuts ); }
   /// Add a new input file to the input data
   void AddSFileIn      ( const SFile& sfile );
   /// Add a new input tree to the input data
   void AddInputSTree   ( const STree& stree )           { m_inputTrees.push_back( stree ); }
   /// Add a new persistent tree to the input data
   void AddPersSTree    ( const STree& ptree )           { m_persTrees.push_back( ptree ); }
   /// Add a new output tree to the input data
   void AddOutputSTree  ( const STree& stree )           { m_outputTrees.push_back( stree ); }
   /// Add a new input EventView tree to the input data
   void AddEVInputSTree ( const SEVTree& stree )         { m_evInputTrees.push_back( stree ); }
   /// Add some number of events to the input data
   void AddEvents       ( Long64_t events )              { m_eventsTotal += events; }

   /// Get the name of the input data type
   const TString&                       GetType() const           { return m_type; }
   /// Get the version of the input data type
   const TString&                       GetVersion() const        { return m_version; }
   /// Get all the defined generator cuts
   const std::vector< SGeneratorCut >&  GetSGeneratorCuts() const { return m_gencuts; }
   /// Get all the defined input files
   const std::vector< SFile >&          GetSFileIn() const        { return m_sfileIn; }
   /// Get all the defined input files
   std::vector< SFile >&                GetSFileIn()              { return m_sfileIn; }
   /// Get all the defined input trees
   const std::vector< STree >&          GetInputTrees() const     { return m_inputTrees; }
   /// Get all the defined "persistent" trees
   const std::vector< STree >&          GetPersTrees() const      { return m_persTrees; }
   /// Get all the defined output trees
   const std::vector< STree >&          GetOutputTrees() const    { return m_outputTrees; }
   /// Get all the defined input EventView trees
   const std::vector< SEVTree >&        GetEVInputTrees() const   { return m_evInputTrees; }
   /// Get the total luminosity of the input data
   Double_t                             GetTotalLumi() const;
   /// Get the total luminosity scaled to the number of events to process
   Double_t                             GetScaledLumi() const;
   /// Get the total number of events in the input data files
   Long64_t                             GetEventsTotal() const    { return m_eventsTotal; }
   /// Get the maximal number of events to process from the input data
   Long64_t                             GetNEventsMax() const     { return m_neventsmax; }
   /// Get the number of events to skip at the beginning of the input data
   Long64_t                             GetNEventsSkip() const    { return m_neventsskip; }

   /// Assignment operator
   SInputData& operator=  ( const SInputData& parent );
   /// Equality operator
   Bool_t      operator== ( const SInputData& rh ) const;
   /// Non-equality operator
   Bool_t      operator!= ( const SInputData& rh ) const;

   /// Function printing the contents of the object
   void print() const;

private:
   TString                         m_type;
   TString                         m_version;
   Double_t                        m_totalLumiGiven;
   std::vector< SGeneratorCut >    m_gencuts;
   std::vector< SFile >            m_sfileIn;
   std::vector< STree >            m_inputTrees;
   std::vector< SEVTree >          m_evInputTrees;
   std::vector< STree >            m_persTrees;
   std::vector< STree >            m_outputTrees;
   Double_t                        m_totalLumiSum;
   Long64_t                        m_eventsTotal;
   Long64_t                        m_neventsmax;
   Long64_t                        m_neventsskip;

   mutable SLogger                 m_logger;

}; // class SInputData

#endif // SFRAME_CORE_SInputData_H
