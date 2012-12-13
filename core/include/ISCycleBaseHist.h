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

#ifndef SFRAME_CORE_ISCycleBaseHist_H
#define SFRAME_CORE_ISCycleBaseHist_H

// Forward declaration(s):
class TList;
class TDirectory;

/**
 *   @short Interface providing histogramming capabilities
 *
 *          This interface is used by the higher-level classes when configuring
 *          SCycleBase objects. This way the high-level classes don't directly
 *          depend on SCycleBaseHist.
 *
 *          This interface provides all the functions that the framework needs
 *          to call to make the histogramming in the cycle work.
 *
 * @version $Revision$
 */
class ISCycleBaseHist {

public:
   virtual ~ISCycleBaseHist() {}

   /// Set the PROOF output list
   virtual void SetHistOutput( TList* output ) = 0;
   /// Get the PROOF output list
   virtual TList* GetHistOutput() const = 0;

protected:
   /// Set the current input file
   virtual void SetHistInputFile( TDirectory* file ) = 0;
   /// Get the currently set input file
   virtual TDirectory* GetHistInputFile() const = 0;

   /// Write the objects meant to be merged using the output file
   virtual void WriteHistObjects() = 0;

}; // class ISCycleBaseHist

#endif // SFRAME_CORE_ISCycleBaseHist_H
