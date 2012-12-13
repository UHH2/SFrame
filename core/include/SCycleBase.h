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

#ifndef SFRAME_CORE_SCycleBase_H
#define SFRAME_CORE_SCycleBase_H

// Local include(s):
#include "ISCycleBase.h"
#include "SCycleBaseNTuple.h"
#include "SCycleBaseHist.h"
#include "SCycleBaseConfig.h"

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
 *          SCycleBaseNTuple, SCycleBaseHist SCycleBaseConfig
 *          and SCycleBaseExec classes in such a way, that every
 *          class is instantiated only once inside the SCycleBase
 *          object. (virtual inheritance...)
 *
 * @version $Revision$
 */
class SCycleBase : public ISCycleBase,
                   public SCycleBaseConfig,
                   public SCycleBaseHist,
                   public SCycleBaseNTuple {

public:
   /// Default constructor
   SCycleBase();

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleBase, 0 )
#endif // DOXYGEN_IGNORE

}; // class SCycleBase

/** @example FirstCycle.h
 *           This is an example cycle using SCycleBase. It can be used
 *           out of the box on lxplus or any computer havin access to
 *           /afs/cern.ch/.
 */

/** @example FirstCycle.cxx
 *           This is an example cycle using SCycleBase. It can be used
 *           out of the box on lxplus or any computer havin access to
 *           /afs/cern.ch/.
 */

/** @example SecondCycle.h
 *           This is an example cycle using SCycleBase. It can be used
 *           after running FirstCycle. It demonstrates how to read back
 *           the ntuple produced by a user cycle.
 */

/** @example SecondCycle.cxx
 *           This is an example cycle using SCycleBase. It can be used
 *           after running FirstCycle. It demonstrates how to read back
 *           the ntuple produced by a user cycle.
 */

#endif // SFRAME_CORE_SCycleBase_H
