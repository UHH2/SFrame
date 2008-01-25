// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBaseExec.h,v 1.1 2008-01-25 14:33:53 krasznaa Exp $
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

#ifndef SFRAME_CORE_SCycleBaseExec_H
#define SFRAME_CORE_SCycleBaseExec_H

// Local include(s):
#include "ISCycleBaseConfig.h"
#include "ISCycleBaseNTuple.h"
#include "ISCycleBaseHist.h"
#include "ISCycleBaseExec.h"
#include "SCycleBaseBase.h"

/**
 *   @short The SCycleBase constituent responsible for running the cycle
 *
 *          The code executing the cycle has been moved to this class.
 *          The idea is that the class only accesses the other parts of
 *          SCycleBase through the interfaces. So various parts
 *          (like SCycleBaseNTuple for ARA) can be exchanged for another
 *          one.
 *
 * @version $Revision: 1.1 $
 */
class SCycleBaseExec : public virtual ISCycleBaseConfig,
                       public virtual ISCycleBaseNTuple,
                       public virtual ISCycleBaseHist,
                       public virtual ISCycleBaseExec,
                       public virtual SCycleBaseBase {

public:
   /// Default constructor
   SCycleBaseExec();
   /// Default destructor
   virtual ~SCycleBaseExec();

   /// Loop over all SInputData
   void ExecuteInputData() throw( SError );

   /// Number of events processed already
   /**
    * The number of processed events is used in a few places,
    * this function tells the framework how many events have
    * already been processed by the cycle.
    */
   Long64_t NumberOfProcessedEvents() const { return m_nProcessedEvents; }

   ///////////////////////////////////////////////////////////////////////////
   //                                                                       //
   //   The following are the functions to be implemented in the derived    //
   //   classes.                                                            //
   //                                                                       //
   ///////////////////////////////////////////////////////////////////////////

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

   // The number of already processed events
   Long64_t m_nProcessedEvents;

   // variable used for the case of multiple InputData objects with
   // the same type, that are written to the same output file
   Bool_t m_keepOutputFile;
   Bool_t m_firstInputDataOfMany;

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleBaseExec, 0 );
#endif // DOXYGEN_IGNORE

}; // class SCycleBaseExec

#endif // SFRAME_CORE_SCycleBaseExec_H
