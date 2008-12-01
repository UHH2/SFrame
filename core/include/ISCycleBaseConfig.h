// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBaseConfig.h,v 1.1.2.1 2008-12-01 14:52:56 krasznaa Exp $

#ifndef SFRAME_CORE_ISCycleBaseConfig_H
#define SFRAME_CORE_ISCycleBaseConfig_H

// Local include(s):
#include "SError.h"

// Forward declaration(s):
class TXMLNode;
class SCycleConfig;

class ISCycleBaseConfig {

public:
   virtual ~ISCycleBaseConfig() {}

   /// Function initialising the cycle
   virtual void Initialize( TXMLNode* ) throw( SError ) = 0;

   virtual const SCycleConfig& GetConfig() const = 0;
   virtual void SetConfig( const SCycleConfig& config ) = 0;

}; // class ISCycleBaseConfig

#endif // SFRAME_CORE_ISCycleBaseConfig_H
