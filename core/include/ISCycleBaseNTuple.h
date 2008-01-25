// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBaseNTuple.h,v 1.1 2008-01-25 14:33:53 krasznaa Exp $
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

#ifndef SFRAME_CORE_ISCycleBaseNTuple_H
#define SFRAME_CORE_ISCycleBaseNTuple_H

// Local include(s):
#include "SError.h"

// Forward declaration(s):
class TTree;
class TFile;
class SInputData;

/**
 *   @short Interface class providing the ntuple access to the cycle
 *
 *          The SCycleBase class is broken into multiple classes. Some of
 *          these constituents can work independently, not knowing what the
 *          other constituents can do. But some parts (like SCycleBaseExec)
 *          rely on other constituents as well. To make those parts as modular
 *          as possible, they don't rely directly on the concrete implementations
 *          of the other constituents, but on interfaces like this.
 *
 *          This interface provides all the functions that the framework uses
 *          to set up reading/writing ntuples.
 *
 * @version $Revision: 1.1 $
 */
class ISCycleBaseNTuple {

public:
   /// Default destructor
   virtual ~ISCycleBaseNTuple() {}

protected:
   /// Open the output file and create the output trees
   virtual void CreateOutputTrees( const SInputData&,
                                   std::vector< TTree* >&, TFile*& ) throw( SError ) = 0;
   /// Open the input file and load the input trees
   virtual void LoadInputTrees( const SInputData&,
                                const std::string&, TFile*& ) throw( SError ) = 0;
   /// Read in the event from the "normal" trees
   virtual void GetEntry( Long64_t entry ) throw( SError ) = 0;
   /// Return the number of events (???)
   virtual Long64_t GetNEvents() const = 0;
   /// Calculate the weight of the current event
   virtual Double_t CalculateWeight( const SInputData& inputData, Long64_t entry ) = 0;
   /// Open an input file
   virtual TFile* OpenInputFile( const char* filename ) throw( SError ) = 0;
   /// Return the name of the active output file
   virtual const char* GetOutputFileName() const = 0;

}; // class ISCycleBaseNTuple

#endif // SFRAME_CORE_ISCycleBaseNTuple_H
