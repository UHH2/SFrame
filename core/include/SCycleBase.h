// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBase.h,v 1.2 2007-11-22 18:19:24 krasznaa Exp $
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

#ifndef SFRAME_CORE_SCycleBase_H
#define SFRAME_CORE_SCycleBase_H

// Local include(s):
#include "SCycleBaseNTuple.h"
#include "SCycleBaseHist.h"
#include "SCycleBaseConfig.h"
#include "SError.h"

// Forward declaration(s):
class SInputData;

/**
 *   @short Base class for the user analysis cycles
 *
 *          All user analysis cycles have to inherit from this
 *          class, and implement all of its "pure virtual"
 *          functions. The class provides functions for handling
 *          input and output TTree-s, easily writing histograms,
 *          reading the cycle's configuration from the XML
 *          configuration file, etc.
 *
 *          The class has a non-trivial inheritance tree, because
 *          of this Doxygen is having problems sorting out its member
 *          functions. (Some of them appear duplicated.) This is
 *          a problem with Doxygen, not with the code.
 *
 *          The "dot" tool on the other hand (producing the inheritance
 *          graphs) is very good. The inheritance graphs look exactly
 *          as they should. The class is composed from the
 *          SCycleBaseNTuple, SCycleBaseHist
 *          and SCycleBaseConfig classes in such a way, that every
 *          class is instantiated only once inside the SCycleBase
 *          object. (virtual inheritance...)
 *
 * @version $Revision: 1.2 $
 */
class SCycleBase : public virtual SCycleBaseNTuple,
                   public virtual SCycleBaseHist,
                   public virtual SCycleBaseConfig {

public:
   /// Default constructor
   SCycleBase();
   /// Default destructor
   virtual ~SCycleBase();

   /// Loop over all SInputData
   void ExecuteInputData() throw( SError );

   /// Number of events processed already
   /**
    * The number of processed events is used in a few places,
    * this function tells the framework how many events have
    * already been processed by the cycle.
    */
   Long64_t NumberOfProcessedEvents() { return m_nProcessedEvents; }

   ///////////////////////////////////////////////////////////////////////////
   //                                                                       //
   //   The following are the functions to be implemented in the derived    //
   //   classes.                                                            //
   //                                                                       //
   ///////////////////////////////////////////////////////////////////////////

   /// Initialisation called at the beginning of a full cycle
   /**
    * Analysis-wide configurations, like the setup of some reconstruction
    * algorithm based on properties configured in XML should be done here.
    */
   virtual void BeginCycle() throw( SError ) = 0;
   /// Finalisation called at the end of a full cycle
   /**
    * This is the last function called after an analysis run, so it
    * could be a good place to print some statistics about the running,
    * maybe close helper files (not input or output files!) used in
    * the analysis.
    */
   virtual void EndCycle()   throw( SError ) = 0;

   /// Initialisation called for each input data type
   /**
    * This is the place to declare the output variables for the output
    * TTree(s). This is also the earliest point where histograms can
    * be created.
    */
   virtual void BeginInputData( const SInputData& ) throw( SError ) = 0;
   /// Finalisation called for each input data type
   /**
    * Mainly used for printing input data statistics, or normalising
    * efficiency histograms by hand.
    */
   virtual void EndInputData  ( const SInputData& ) throw( SError ) = 0;

   /// Initialisation called for each input file
   /**
    * This is the place to connect the input variables to the branches
    * of the input tree(s).
    */
   virtual void BeginInputFile( const SInputData& ) throw( SError ) = 0;

   /// Function called for every event
   /**
    * This is the function where the main analysis should be done. By the
    * time it is called, all the input variables are filled with the
    * contents of the actual event.
    */
   virtual void ExecuteEvent( const SInputData&, Double_t weight ) throw( SError ) = 0;

private:
   void CheckInputFiles( SInputData& ) throw( SError );

   //
   // Functions from the base classes that should be hidden from the user.
   // While it's maybe not that nice to duplicate the definitions of the functions
   // here, it prevents the user from calling functions that he's not supposed to.
   //
   void InitHistogramming( TDirectory* outputFile, const TString& outputFileName );
   void CreateOutputTrees( const SInputData&, std::vector< TTree* >&, TFile*& ) throw( SError );
   void LoadInputTrees( const SInputData&, const std::string&, TFile*& ) throw( SError );
   void ConnectEVSyncVariable() throw( SError );
   void GetEntry( Long64_t entry ) throw( SError );
   void SyncEVTrees() throw( SError );
   Long64_t GetNEvents() const;
   Double_t CalculateWeight( const SInputData& inputData, Long64_t entry );
   TFile* OpenInputFile( const char* filename ) throw( SError );
   const char* GetOutputFileName() const;

   // The number of already processed events
   Long64_t m_nProcessedEvents;

   // variable used for the case of multiple InputData objects with
   // the same type, that are written to the same output file
   Bool_t m_keepOutputFile;
   Bool_t m_firstInputDataOfMany;

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleBase, 0 );
#endif // DOXYGEN_IGNORE

}; // class SCycleBase

#endif // SFRAME_CORE_SCycleBase_H
