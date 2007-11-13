// Dear emacs, this is -*- c++ -*-
// $Id: SMsgType.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SMsgType_H
#define SFRAME_CORE_SMsgType_H

/**
 * Enumeration for classifying messages. Based on this classification,
 * SLogWriter can decide if/how the message should be shown.
 * (Naming taken from ATLAS offline.)
 */
enum SMsgType { VERBOSE = 1, DEBUG = 2, INFO = 3, WARNING = 4,
                ERROR = 5, FATAL = 6, ALWAYS = 7 };

#endif // SFRAME_CORE_SMsgType_H
