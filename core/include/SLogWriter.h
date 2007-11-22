// Dear emacs, this is -*- c++ -*-
// $Id: SLogWriter.h,v 1.2 2007-11-22 18:19:25 krasznaa Exp $
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

#ifndef SFRAME_CORE_SLogWriter_H
#define SFRAME_CORE_SLogWriter_H

// STL include(s):
#include <string>
#include <map>

// Local include(s):
#include "SMsgType.h"

/**
 *   @short Message writing class
 *
 *          Singleton class for actually writing the formatted
 *          messages to the console.
 *
 *          Right now it only writes messages to the terminal, but
 *          one possibility would be to write messages to a file
 *          for batch running later on. (Just an idea...)
 *
 *     @see SLogger
 * @version $Revision: 1.2 $
 */
class SLogWriter {

public:
   /// Function for accessing the single object
   static SLogWriter* Instance();
   /// Default destructor
   ~SLogWriter();

   /// Function writing a message to the output
   void Write( SMsgType type, const std::string& line ) const;

   /// Set the message type above which messages are printed
   void SetMinType( SMsgType type );
   /// Get the message type above which messages are printed
   SMsgType GetMinType() const;

protected:
   /// Protected default constructor
   SLogWriter();

private:
   static SLogWriter* m_instance;

   std::map< SMsgType, std::string > m_typeMap;
   std::map< SMsgType, std::string > m_colorMap;
   SMsgType                          m_minType;

}; // class SLogWriter

#endif // SFRAME_CORE_SLogWriter_H
