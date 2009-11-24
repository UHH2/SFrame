// Dear emacs, this is -*- c++ -*-
// $Id$
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

#ifndef SFRAME_CODE_ISCycleBase_H
#define SFRAME_CODE_ISCycleBase_H

// Local include(s):
#include "SCycleBaseExec.h"
#include "ISCycleBaseConfig.h"

/**
 *  @short Main SFrame cycle interface
 *
 *         This interface got revived to make it possible once again to
 *         modify the base classes of the cycles. Since SCycleController
 *         uses this "interface" (it's not really a pure interface actually...),
 *         it's possible to put together a different cycle base class than SCycleBase.
 *
 * @version $Revision$
 */
class ISCycleBase : public SCycleBaseExec,
                    public virtual ISCycleBaseConfig {

public:
   virtual ~ISCycleBase() {}

#ifndef DOXYGEN_IGNORE
   ClassDef( ISCycleBase, 0 );
#endif // DOXYGEN_IGNORE

}; // class ISCycleBase

#endif // SFRAME_CODE_ISCycleBase_H
