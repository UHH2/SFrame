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

#ifndef SFRAME_CORE_SErrorHandler_H
#define SFRAME_CORE_SErrorHandler_H

// ROOT include(s):
#include <Rtypes.h>

/// Function printing log messages from ROOT
extern void SErrorHandler( int level, Bool_t abort, const char* location,
                           const char* message );

#endif // SFRAME_CORE_SErrorHandler_H
