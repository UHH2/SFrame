// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBaseNTuple.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SCycleBaseNTuple_H
#define SFRAME_CORE_SCycleBaseNTuple_H

// STL include(s):
#include <vector>
#include <map>
#include <string>
#include <list>

// Local include(s):
#include "SCycleBaseConfig.h"
#include "SError.h"

// Forward declaration(s):
class TTree;
class TFile;
class TBranch;
class SInputData;

/**
 * This is the most complex constituent of all the SCycleBase
 * classes. It is responsible for handling input and output
 * TTree-s.
 */
class SCycleBaseNTuple : public virtual SCycleBaseConfig {

public:
   SCycleBaseNTuple();
   virtual ~SCycleBaseNTuple();

protected:
   //
   // The functions for connecting input variables:
   //
   template< typename T >
   void ConnectVariable( const char* treeName, const char* branchName,
                         T& variable ) throw ( SError );
   template< typename T >
   const Int_t* ConnectEventViewVariable( const char* baseName, const char* branchName,
                                          std::vector< T >& variables ) throw( SError );

   //
   // This is the function that creates/connects to an output object/variable:
   //
   template< class T >
   TBranch* DeclareVariable( T& obj, const char* name, const char* treeName = 0 ) throw( SError );

   //
   // Functions called by the framework:
   //
   void CreateOutputTrees( const SInputData&, std::vector< TTree* >&, TFile*& ) throw( SError );
   void LoadInputTrees( const SInputData&, const std::string&, TFile*& ) throw( SError );
   void ConnectEVSyncVariable() throw( SError );
   void GetEntry( Long64_t entry ) throw( SError );
   void SyncEVTrees() throw( SError );
   Long64_t GetNEvents() const { return m_nEvents; }
   Double_t CalculateWeight( const SInputData& inputData, Long64_t entry );
   TFile* OpenInputFile( const char* filename ) throw( SError );
   const char* GetOutputFileName() const { return m_outputFileName; }

private:
   static const char* RootType( const char* typeid_type );
   TTree* GetTree( const std::string& treeName ) throw( SError );
   void RegisterInputBranch( TBranch* br ) throw( SError );

   //
   // These are the objects used to handle the input data:
   //
   std::vector< TTree* >             m_inputTrees; // List of input TTree pointers
   std::vector< TTree* >             m_EVinputTrees; // List of input EV TTree pointers
   std::map< TTree*, Int_t >         m_EVInTreeToCounters; // Map storing the last loaded entry for all EV trees
   std::map< TTree*, std::string >   m_EVInTreeToCollTreeName; // Map storing the name of the collection tree
                                                               // for each EV tree
   std::map< TTree*, std::string >   m_EVInTreeToBaseName; // Map storing the base name of the EV trees
   std::map< TTree*, Int_t >         m_EVInTreeToViewNumber; // Map storing the "view number" of the EV trees
   std::map< std::string, Int_t >    m_EVBaseNameToCollVar; // Map associating "view groups" to a collection
                                                            // tree variable

   std::vector< TBranch* > m_inputBranches; // map of input branch pointers registered for the
                                            // current cycle
   std::map< TBranch*, std::string > m_EVInputBranchesToBaseName; // map to hold branch pointers and basename for 
                                                                  // EV variables registered for the current cycle
   std::map< TBranch*, Int_t >       m_EVInputBranchesToViewNumber; // map to hold branch pointers and basename for 
                                                                    // EV variables registered for the current cycle

   // Vector to hold the output trees
   std::vector< TTree* > m_outputTrees;

   // Number of events in the input file:
   Long64_t m_nEvents;

   // Name of the output file:
   TString m_outputFileName;

   // We have to keep the pointers to the output variables defined by the user.
   // ROOT keeps track of the objects by storing pointers to pointers to the
   // objects. Since the user probably wants to use the output objects directly
   // and not through pointers, the base class has to take care of this
   // pointer issue by itself...
   std::list< void* > m_outputVarPointers;

   ClassDef( SCycleBaseNTuple, 0 );

}; // class SCycleBaseNTuple

// Don't include the templated function(s) when we're generating
// a dictionary:
#ifndef __CINT__
#include "SCycleBaseNTuple.icc"
#endif

#endif // SFRAME_CORE_SCycleBaseNTuple_H
