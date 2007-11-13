// Dear emacs, this is -*- c++ -*-
// $Id: SInputData.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Core
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author Andreas Hoecker  <Andreas.Hocker@cern.ch>       - CERN
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

#ifndef SFRAME_CORE_SInputData_H
#define SFRAME_CORE_SInputData_H

// STL include(s):
#include <vector>

// ROOT include(s):
#include "TObject.h"
#include "TString.h"

// Local include(s):
#include "SGeneratorCut.h"
#include "SLogger.h"

/**
 * Class describing an input file to the analysis.
 */
class SFile {

public:
   SFile()
      : file( "" ), lumi( -1 ) , events(0){}
   SFile( TString f )
      : file( f ), lumi( -1 ) , events(0){}
   SFile( TString f, Double_t l )
      : file( f ), lumi( l ) , events(0){}

   //
   // Various basic operators
   //
   SFile& operator=  ( const SFile& parent );
   Bool_t operator== ( const SFile& rh ) const;
   Bool_t operator!= ( const SFile& rh ) const;

   TString  file;
   Double_t lumi;
   Long64_t events;

}; // class SFile

/**
 * Class describing a "simple" input tree in the input file(s).
 */
class STree {

public:
   STree( TString t )
      : treeName( t ) {}

   //
   // Various basic operators
   //
   STree& operator=  ( const STree& parent );
   Bool_t operator== ( const STree& rh ) const;
   Bool_t operator!= ( const STree& rh ) const;

   TString treeName;

}; // class STree

/**
 * Class describing an EventView input tree in the input file(s).
 */
class SEVTree {

public:
   SEVTree( TString t, TString tbasename, Int_t viewnum,
            TString colltname )
      : treeName( t ), treeBaseName( tbasename ),
        viewNumber( viewnum ), collTreeName( colltname ) {}

   //
   // Various basic operators
   //
   SEVTree& operator=  ( const SEVTree& parent );
   Bool_t   operator== ( const SEVTree& rh ) const;
   Bool_t   operator!= ( const SEVTree& rh ) const;

   TString treeName;
   TString treeBaseName;
   Int_t   viewNumber;
   TString collTreeName;

}; // class SEVTree

/**
 * Class describing one kind of input data. (One kind of MC, real data, etc.)
 */
class SInputData : public TObject {

public:
   SInputData();
   ~SInputData();

   //
   // "Setter" methods
   //
   void SetType         ( const TString& type )          { m_type = type; }
   void SetVersion      ( const TString& version )       { m_version     = version; }
   void SetTotalLumi    ( Double_t lumi )                { m_totalLumiGiven = lumi; }
   void SetNEventsMax   ( Long64_t nevents )             { m_neventsmax = nevents; }
   void AddGenCut       ( const SGeneratorCut& gencuts ) { m_gencuts.push_back( gencuts ); }
   void AddSFileIn      ( const SFile& sfile);
   void AddInputSTree   ( const STree& stree )           { m_inputTrees.push_back( stree ); }
   void AddOutputSTree  ( const STree& stree )           { m_outputTrees.push_back( stree ); }
   void AddEVInputSTree ( const SEVTree& stree )         { m_evInputTrees.push_back( stree ); }

   //
   // "Getter" methods
   //
   TString                              GetType() const           { return m_type; }
   TString                              GetVersion() const        { return m_version; }
   const std::vector< SGeneratorCut >&  GetSGeneratorCuts() const { return m_gencuts; }
   const std::vector< SFile >&          GetSFileIn() const        { return m_sfileIn; }
   std::vector< SFile >&                GetSFileIn()              { return m_sfileIn; }
   const std::vector< STree >&          GetInputTrees() const     { return m_inputTrees;}
   const std::vector< STree >&          GetOutputTrees() const    { return m_outputTrees;}
   const std::vector< SEVTree >&        GetEVInputTrees() const   { return m_evInputTrees;}
   Double_t                             GetTotalLumi() const;
   Double_t                             GetScaledLumi() const;
   Long64_t                             GetEventsTotal() const    {return m_eventsTotal;}
   void                                 AddEvents(Long64_t events){m_eventsTotal += events;}
   Long64_t                             GetNEventsMax() const     { return m_neventsmax;}

   //
   // Various basic operators
   //
   SInputData& operator= ( const SInputData& parent );
   Bool_t      operator== ( const SInputData& rh ) const;
   Bool_t      operator!= ( const SInputData& rh ) const;

   void print() const;

private:
   TString                         m_type;
   TString                         m_version;
   Double_t                        m_totalLumiGiven;
   std::vector< SGeneratorCut >    m_gencuts;
   std::vector< SFile >            m_sfileIn;
   std::vector< STree >            m_inputTrees;
   std::vector< SEVTree >          m_evInputTrees;
   std::vector< STree >            m_outputTrees;
   Double_t                        m_totalLumiSum;
   Long64_t                        m_eventsTotal;
   Long64_t                        m_neventsmax ;

   mutable SLogger                 m_logger;

}; // class SInputData

#endif // SFRAME_CORE_SInputData_H
