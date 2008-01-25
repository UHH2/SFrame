// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBaseHist.h,v 1.1 2008-01-25 14:33:53 krasznaa Exp $
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

#ifndef SFRAME_CORE_ISCycleBaseHist_H
#define SFRAME_CORE_ISCycleBaseHist_H

// Forward declaration(s):
class TDirectory;
class TString;

/**
 *   @short Interface class providing the histogramming setup of the cycle
 *
 *          The SCycleBase class is broken into multiple classes. Some of
 *          these constituents can work independently, not knowing what the
 *          other constituents can do. But some parts (like SCycleBaseExec)
 *          rely on other constituents as well. To make those parts as modular
 *          as possible, they don't rely directly on the concrete implementations
 *          of the other constituents, but on interfaces like this.
 *
 *          This interface provides all the functions that the framework needs
 *          to call to make the histogramming in the cycle work.
 *
 * @version $Revision: 1.1 $
 */
class ISCycleBaseHist {

public:
   /// Default destructor
   virtual ~ISCycleBaseHist() {}

protected:
   /// Function initialising the histogramming
   /**
    * This function is needed because of the implementation. The constituent
    * that handles the output histograms needs to know the output file name
    * and have access to the output file directly. This function is called
    * by the framework every time a new output file is opened.
    */
   virtual void InitHistogramming( TDirectory* outputFile,
                                   const TString& outputFileName ) = 0;

}; // class ISCycleBaseHist

#endif // SFRAME_CORE_ISCycleBaseHist_H
