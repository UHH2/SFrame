// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBase.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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
 *  Abstract base class for all cycle algorithms.
 *  From this class the template class is derived which
 *  is then used as a base class for all cycle classes.
 */
class SCycleBase : public virtual SCycleBaseNTuple,
                   public virtual SCycleBaseHist,
                   public virtual SCycleBaseConfig {

public:
   SCycleBase();
   virtual ~SCycleBase();

   /// Loops over all SInputData:
   void ExecuteInputData() throw( SError );

   /// For easy access:
   Long64_t NumberOfProcessedEvents() { return m_nProcessedEvents; }

   ///////////////////////////////////////////////////////////////////////////
   //                                                                       //
   //   The following are the functions to be implemented in the derived    //
   //   classes.                                                            //
   //                                                                       //
   ///////////////////////////////////////////////////////////////////////////

   // Special initialisation of cycle called at begin and end of full cycle
   virtual void BeginCycle() throw( SError ) = 0;
   virtual void EndCycle()   throw( SError ) = 0;

   // called at begin and end of each SInputData within SInputData loop
   virtual void BeginInputData( const SInputData& ) throw( SError ) = 0;
   virtual void EndInputData  ( const SInputData& ) throw( SError ) = 0;

   // loops over all events
   virtual void ExecuteEvent( const SInputData&, Double_t weight ) throw( SError ) = 0;

   // called before new input file is processed
   virtual void BeginInputFile( const SInputData& ) throw( SError ) = 0;

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

   /// variable used for the case of multiple InputData objects with
   /// the same type, that are written to the same output file
   Bool_t m_keepOutputFile;
   Bool_t m_firstInputDataOfMany;

   ClassDef( SCycleBase, 0 );

}; // class SCycleBase

#endif // SFRAME_CORE_SCycleBase_H
