// Dear emacs, this is -*- c++ -*-
// $Id: SLogger.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SLogger_H
#define SFRAME_CORE_SLogger_H

// STL include(s):
#include <string>
#include <sstream>

// Local include(s):
#include "SMsgType.h"
#include "SLogWriter.h"

// Forward declaration(s):
class TObject;
class SLogWriter;

/**
 * Class that should be used in the whole framework for printing messages
 * on the terminal. It produces nicely formatted log messages using the
 * SLogWriter class.
 */
class SLogger : public std::ostringstream {

public:
   SLogger( const TObject* source );
   SLogger( const std::string& source );
   SLogger( const SLogger& parent );
   ~SLogger();

   // Needed for copying:
   SLogger& operator= ( const SLogger& parent );

   // Stream modifier(s):
   static SLogger& endmsg( SLogger& logger );

   // Accept stream modifiers:
   SLogger& operator<< ( SLogger& ( *_f )( SLogger& ) );
   SLogger& operator<< ( std::ostream& ( *_f )( std::ostream& ) );
   SLogger& operator<< ( std::ios& ( *_f )( std::ios& ) );

   // Accept message type specification:
   SLogger& operator<< ( SMsgType type );

   // For all the "conventional" inputs:
   template < class T > SLogger& operator<< ( T arg ) {
      if( m_activeType >= m_logWriter->minType() ) {
         ( * ( std::ostringstream* ) this ) << arg;
      }
      return *this;
   }

   // The old style message sender function:
   void send( SMsgType type, const std::string& message ) const;

private:
   void send();

   const TObject* m_objSource;
   std::string    m_strSource;
   SLogWriter*    m_logWriter;
   SMsgType       m_activeType;

}; // class SLogger

//////////////////////////////////////////////////////////////////////
//                                                                  //
//   To speed up the code a bit, the following operators are        //
//   declared 'inline'.                                             //
//                                                                  //
//////////////////////////////////////////////////////////////////////

inline SLogger& SLogger::operator<< ( SLogger& ( *_f )( SLogger& ) ) {

   return ( _f )( *this );

}

inline SLogger& SLogger::operator<< ( std::ostream& ( *_f )( std::ostream& ) ) {

   if( m_activeType >= m_logWriter->minType() ) {
      ( _f )( *this );
   }
   return *this;

}

inline SLogger& SLogger::operator<< ( std::ios& ( *_f )( std::ios& ) ) {

   if( m_activeType >= m_logWriter->minType() ) {
      ( _f )( *this );
   }
   return *this;

}

inline SLogger& SLogger::operator<< ( SMsgType type ) {

   m_activeType = type;
   return *this;

}

#endif // SFRAME_CORE_SLogger_H
