// Dear emacs, this is -*- c++ -*-

#ifndef SFRAME_CORE_SErrorHandler_H
#define SFRAME_CORE_SErrorHandler_H

// ROOT include(s):
#include <TError.h>

/// Function printing log messages from ROOT
extern void SErrorHandler( int level, Bool_t abort, const char* location,
                           const char* message );

#endif // SFRAME_CORE_SErrorHandler_H
