// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBaseNTuple.h,v 1.1.2.2 2009-01-08 16:09:32 krasznaa Exp $

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
class TFile;
class SInputData;

class ISCycleBaseNTuple {

public:
   virtual ~ISCycleBaseNTuple() {}

   virtual void SetNTupleOutput( TList* output ) = 0;
   virtual TList* GetNTupleOutput() const = 0;

protected:
   /// Create the output trees
   virtual void CreateOutputTrees( const SInputData& id,
                                   std::vector< TTree* >& outTrees,
                                   TFile* outputFile = 0 ) throw( SError ) = 0;
   /// Load the input trees
   virtual void LoadInputTrees( const SInputData& id, TTree* main_tree ) throw( SError ) = 0;
   /// Read in the event from the "normal" trees
   virtual void GetEvent( Long64_t entry ) throw( SError ) = 0;
   /// Calculate the weight of the current event
   virtual Double_t CalculateWeight( const SInputData& inputData, Long64_t entry ) = 0;

}; // class ISCycleBaseNTuple

#endif // SFRAME_CORE_ISCycleBaseNTuple_H
