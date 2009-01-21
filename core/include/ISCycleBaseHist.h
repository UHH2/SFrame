// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBaseHist.h,v 1.1.2.2 2009-01-21 14:34:54 krasznaa Exp $
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
class TList;

/**
 *   @short Interface providing histogramming capabilities
 *
 *          This interface is used by the higher-level classes when configuring
 *          SCycleBase objects. This way the high-level classes don't directly
 *          depend on SCycleBaseHist.
 *
 * @version $Revision: 1.1.2.2 $
 */
class ISCycleBaseHist {

public:
   virtual ~ISCycleBaseHist() {}

   /// Set the PROOF output list
   virtual void SetHistOutput( TList* output ) = 0;
   /// Get the PROOF output list
   virtual TList* GetHistOutput() const = 0;

}; // class ISCycleBaseHist

#endif // SFRAME_CORE_ISCycleBaseHist_H
