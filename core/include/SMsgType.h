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

#ifndef SFRAME_CORE_SMsgType_H
#define SFRAME_CORE_SMsgType_H

/// Message type enumeration
/**
 * Enumeration for classifying messages. Based on this classification,
 * SLogWriter can decide if/how the message should be shown.
 * (Naming taken from ATLAS offline.)
 */
enum SMsgType {
   VERBOSE = 1, /**< Type for the most detailed messages. Only for serious debugging. */
   DEBUG = 2,   /**< Type for debugging messages. A few messages per event allowed. */
   INFO = 3,    /**< Type for normal information messages. No messages in event processing! */
   WARNING = 4, /**< Type for smaller problems. (Analysis is not affected in general.) */
   ERROR = 5,   /**< Type for "real" problems. (Analysis results probably affected.) */
   FATAL = 6,   /**< Type for problems that should halt the execution. */
   ALWAYS = 7   /**< Type that should always be shown. (Not really used.) */
};

#endif // SFRAME_CORE_SMsgType_H
