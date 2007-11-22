// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBaseNTuple.h,v 1.2 2007-11-22 18:19:25 krasznaa Exp $
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
 *   @short NTuple handling part of SCycleBase
 *
 *          This is the most complex constituent of all the
 *          SCycleBase classes. It is responsible for handling
 *          input and output TTree-s. It has quite a number of
 *          protected functions which are used by SCycleBase, and
 *          are hidden from the user by that class. (A little
 *          C++ magic...)
 *
 * @version $Revision: 1.2 $
 */
class SCycleBaseNTuple : public virtual SCycleBaseConfig {

public:
   /// Default constructor
   SCycleBaseNTuple();
   /// Default destructor
   virtual ~SCycleBaseNTuple();

protected:
   /// Connect an input variable
   template< typename T >
   void ConnectVariable( const char* treeName, const char* branchName,
                         T& variable ) throw ( SError );
   /// Connect an input variable that is available in multiple views
   template< typename T >
   const Int_t* ConnectEventViewVariable( const char* baseName, const char* branchName,
                                          std::vector< T >& variables ) throw( SError );

   /// Declare an output variable
   template< class T >
   TBranch* DeclareVariable( T& obj, const char* name, const char* treeName = 0 ) throw( SError );

   //////////////////////////////////////////////////////////
   //                                                      //
   //          Functions called by the framework:          //
   //                                                      //
   //////////////////////////////////////////////////////////

   /// Open the output file and create the output trees
   void CreateOutputTrees( const SInputData&, std::vector< TTree* >&, TFile*& ) throw( SError );
   /// Open the input file and load the input trees
   void LoadInputTrees( const SInputData&, const std::string&, TFile*& ) throw( SError );
   /// Initialise the EventView tree synchronisation
   void ConnectEVSyncVariable() throw( SError );
   /// Read in the event from the "normal" trees
   void GetEntry( Long64_t entry ) throw( SError );
   /// Synchronise the EventView trees to the current event
   void SyncEVTrees() throw( SError );
   /// Return the number of events (???)
   Long64_t GetNEvents() const { return m_nEvents; }
   /// Calculate the weight of the current event
   Double_t CalculateWeight( const SInputData& inputData, Long64_t entry );
   /// Open an input file
   TFile* OpenInputFile( const char* filename ) throw( SError );
   /// Return the name of the active output file
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

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleBaseNTuple, 0 );
#endif // DOXYGEN_IGNORE

}; // class SCycleBaseNTuple

// Don't include the templated function(s) when we're generating
// a dictionary:
#ifndef __CINT__
#include "SCycleBaseNTuple.icc"
#endif

#endif // SFRAME_CORE_SCycleBaseNTuple_H
