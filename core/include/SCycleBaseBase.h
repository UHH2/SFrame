// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBaseBase.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SCycleBaseBase_H
#define SFRAME_CORE_SCycleBaseBase_H

// ROOT include(s):
#include "TObject.h"

// Local include(s):
#include "SLogger.h"

/**
 * Absolute base object in the SCycleBase hierarchy. All "parts" of SCycleBase
 * have to inherit from this with "public virtual", so that it only gets created
 * once in memory for each cycle...
 */
class SCycleBaseBase : public virtual TObject {

public:
   SCycleBaseBase();
   virtual ~SCycleBaseBase();

protected:
   mutable SLogger m_logger;

   ClassDef( SCycleBaseBase, 0 );

}; // class SCycleBaseBase

#endif // SFRAME_CORE_SCycleBaseBase_H
