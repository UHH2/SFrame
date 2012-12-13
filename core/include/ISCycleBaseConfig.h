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

#ifndef SFRAME_CORE_ISCycleBaseConfig_H
#define SFRAME_CORE_ISCycleBaseConfig_H

// Local include(s):
#include "SError.h"

// Forward declaration(s):
class TXMLNode;
class SCycleConfig;
class TList;

/**
 *   @short Interface providing configuration capabilities
 *
 *          This interface is used by the higher-level classes when configuring
 *          SCycleBase objects. This way the high-level classes don't directly
 *          depend on SCycleBaseConfig.
 *
 *          This interface provides all the configuration functions that the
 *          framework needs from an analysis cycle.
 *
 * @version $Revision$
 */
class ISCycleBaseConfig {

public:
   virtual ~ISCycleBaseConfig() {}

   /// Function initialising the cycle
   virtual void Initialize( TXMLNode* ) throw( SError ) = 0;

   /// Get the full configuration of the cycle
   virtual const SCycleConfig& GetConfig() const = 0;
   /// Get the full configuration of the cycle
   virtual SCycleConfig& GetConfig() = 0;
   /// Set the full configuration of the cycle
   virtual void SetConfig( const SCycleConfig& config ) = 0;

   /// Get the list of all declared configuration objects
   virtual const TList& GetConfigurationObjects() const = 0;

   /// Set which list should be used for the configuration input
   virtual void SetConfInput( TList* input ) = 0;
   /// Check which list should be used for the configuration input
   virtual TList* GetConfInput() const = 0;

}; // class ISCycleBaseConfig

#endif // SFRAME_CORE_ISCycleBaseConfig_H
