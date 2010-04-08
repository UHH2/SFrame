// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Plug-ins
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - CERN/Debrecen
 *
 ***************************************************************************/

// ROOT include(s):
#include <TH1.h>
#include <TObject.h>

// SFrame include(s):
#include "core/include/SCycleBase.h"

// Local include(s):
#include "../include/SToolBase.h"

SToolBase::SToolBase( SCycleBase* parent )
   : m_logger( "SToolBase" ), m_parent( parent ) {

}

SCycleBase* SToolBase::GetParent() const {

   return m_parent;
}

void SToolBase::WriteObj( const TObject& obj, const char* directory ) throw( SError ) {

   GetParent()->WriteObj( obj, directory );
   return;
}

TH1* SToolBase::Hist( const char* name, const char* dir ) {

   return GetParent()->Hist( name, dir );
}

void SToolBase::AddConfigObject( TObject* object ) {

   GetParent()->AddConfigObject( object );
   return;
}

TObject* SToolBase::GetConfigObject( const char* name ) const {

   return GetParent()->GetConfigObject( name );
}

void SToolBase::SetLogName( const char* name ) {

   m_logger.SetSource( name );
   return;
}
