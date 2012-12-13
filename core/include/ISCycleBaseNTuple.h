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

#ifndef SFRAME_CORE_ISCycleBaseNTuple_H
#define SFRAME_CORE_ISCycleBaseNTuple_H

// STL include(s):
#include <vector>

// ROOT include(s):
#include <Rtypes.h>

// Local include(s):
#include "SError.h"

// Forward declaration(s):
class TTree;
class TList;
class TDirectory;
class SInputData;

/**
 *   @short Interface providing ntuple handling capabilities
 *
 *          This interface is used by the higher-level classes when setting up
 *          SCycleBase objects. This way the high-level classes don't directly
 *          depend on SCycleBaseNTuple.
 *
 *          This interface provides all the functions that the framework uses
 *          to set up reading/writing of ntuples.
 *
 * @version $Revision$
 */
class ISCycleBaseNTuple {

public:
   /// Virtual destructor, needed to make vtable happy...
   virtual ~ISCycleBaseNTuple() {}

   /// Set the PROOF output list
   virtual void SetNTupleOutput( TList* output ) = 0;
   /// Get the PROOF output list
   virtual TList* GetNTupleOutput() const = 0;
   /// Set the object list used for NTuple input
   virtual void SetNTupleInput( TList* input ) = 0;
   /// Get the object list used for NTuple input
   virtual TList* GetNTupleInput() const = 0;

protected:
   /// Function creating an output file on demand
   virtual TDirectory* GetOutputFile() throw( SError ) = 0;
   /// Function closing a potentially open output file
   virtual void CloseOutputFile() throw( SError ) = 0;
   /// Create the output trees
   virtual void
   CreateOutputTrees( const SInputData& id,
                      std::vector< TTree* >& outTrees ) throw( SError ) = 0;
   /// Save all the created output trees in the output
   virtual void SaveOutputTrees() throw( SError ) = 0;
   /// Load the input trees
   virtual void LoadInputTrees( const SInputData& id, TTree* main_tree,
                                TDirectory*& inputFile ) throw( SError ) = 0;
   /// Read in the event from the "normal" trees
   virtual void GetEvent( Long64_t entry ) throw( SError ) = 0;
   /// Calculate the weight of the current event
   virtual Double_t CalculateWeight( const SInputData& inputData,
                                     Long64_t entry ) const = 0;
   /// Forget about the internally cached TTree pointers
   virtual void ClearCachedTrees() = 0;

}; // class ISCycleBaseNTuple

#endif // SFRAME_CORE_ISCycleBaseNTuple_H
