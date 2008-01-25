// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBaseExec.h,v 1.1 2008-01-25 14:33:53 krasznaa Exp $
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

#ifndef SFRAME_CORE_ISCycleBaseExec_H
#define SFRAME_CORE_ISCycleBaseExec_H

// Local include(s):
#include "SError.h"

/**
 *   @short Interface class providing the executor functions of the cycle
 *
 *          The SCycleBase class is broken into multiple classes. Some of
 *          these constituents can work independently, not knowing what the
 *          other constituents can do. But some parts (like SCycleBaseExec)
 *          rely on other constituents as well. To make those parts as modular
 *          as possible, they don't rely directly on the concrete implementations
 *          of the other constituents, but on interfaces like this.
 *
 *          This interface provides all the functions that the framework calls
 *          to execute the cycle.
 *
 * @version $Revision: 1.1 $
 */
class ISCycleBaseExec {

public:
   /// Default destructor
   virtual ~ISCycleBaseExec() {}

   /// Loop over all SInputData
   /**
    * This is the main event loop function. It is called by the framework
    * to instruct the cycle to do its analysis on the specified input.
    */
   virtual void ExecuteInputData() throw( SError ) = 0;

   /// Number of events processed already
   /**
    * The number of processed events is used in a few places,
    * this function tells the framework how many events have
    * already been processed by the cycle.
    */
   virtual Long64_t NumberOfProcessedEvents() const = 0;

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
   virtual void EndCycle() throw( SError ) = 0;

}; // class ISCycleBaseExec

#endif // SFRAME_CORE_ISCycleBaseExec_H
