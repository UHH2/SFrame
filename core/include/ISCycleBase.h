// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBase.h,v 1.1 2008-01-25 14:33:53 krasznaa Exp $
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

#ifndef SFRAME_CORE_ISCycleBase_H
#define SFRAME_CORE_ISCycleBase_H

// ROOT include(s):
#include "TObject.h"

// Local include(s):
#include "ISCycleBaseConfig.h"
#include "ISCycleBaseHist.h"
#include "ISCycleBaseNTuple.h"
#include "ISCycleBaseExec.h"

/**
 *   @short Main SFrame cycle interface
 *
 *          To make the implementation of the core SFrame classes more
 *          flexible, the framework calls all functions of the cycles
 *          through interfaces.
 *
 *          This is the main interface through which all functionality
 *          of a user cycle that the framework has to call can be called.
 *
 * @version $Revision: 1.1 $
 */
class ISCycleBase : public virtual ISCycleBaseConfig,
                    public virtual ISCycleBaseHist,
                    public virtual ISCycleBaseNTuple,
                    public virtual ISCycleBaseExec,
                    public virtual TObject {

public:
   virtual ~ISCycleBase() {}

#ifndef DOXYGEN_IGNORE
   ClassDef( ISCycleBase, 0 );
#endif // DOXYGEN_IGNORE

}; // class ISCycleBase

#endif // SFRAME_CORE_ISCycleBase_H
