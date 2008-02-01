// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBaseBase.h,v 1.3 2008-02-01 14:22:01 krasznaa Exp $
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

#ifndef SFRAME_CORE_SCycleBaseBase_H
#define SFRAME_CORE_SCycleBaseBase_H

// ROOT include(s):
#include "TObject.h"

// Local include(s):
#include "SLogger.h"

/**
 *   @short Absolute base of all analysis cycles
 *
 *          Absolute base object in the SCycleBase hierarchy. All "parts"
 *          of SCycleBase have to inherit from this with "public virtual",
 *          so that it only gets created once in memory for each cycle...
 *
 * @version $Revision: 1.3 $
 */
class SCycleBaseBase : public virtual TObject {

public:
   /// Default constructor
   SCycleBaseBase();
   /// Default destructor
   virtual ~SCycleBaseBase();

protected:
   /// Object used for output messages to the terminal
   /**
    * In the whole SFrame framework, terminal messages are printed
    * using the SLogger class. This makes it possible to print nice
    * formatted messages to the teminal. The user should preferably
    * use this object to write messages instead of using std::cout
    * directly.
    */
   mutable SLogger m_logger;

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleBaseBase, 0 );
#endif // DOXYGEN_IGNORE

}; // class SCycleBaseBase

/**  @class TObject
 *   @short ROOT base class
 *
 *          This is the base ROOT object used as base class for a few
 *          classes in the code. For more documentation, go to
 *          http://root.cern.ch/root/html518/TObject.html.
 */

#endif // SFRAME_CORE_SCycleBaseBase_H
