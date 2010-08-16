// $Id$
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

// Local include(s):
#include "../include/STreeTypeDecoder.h"
#include "../include/STreeType.h"

// Initialize the static variable(s):
STreeTypeDecoder* STreeTypeDecoder::m_instance = 0;
const TString STreeTypeDecoder::m_unknownName = "Unknown";
const Int_t STreeTypeDecoder::m_unknownCode = -1;

STreeTypeDecoder* STreeTypeDecoder::Instance() {

   if( ! m_instance ) {
      m_instance = new STreeTypeDecoder();
   }
   return m_instance;
}

void STreeTypeDecoder::AddType( const TString& name, Int_t code ) {

   m_forwardMap[ name ] = code;
   m_reverseMap[ code ] = name;

   return;
}

const TString& STreeTypeDecoder::GetName( Int_t code ) const {

   std::map< Int_t, TString >::const_iterator itr;
   if( ( itr = m_reverseMap.find( code ) ) == m_reverseMap.end() ) {
      return m_unknownName;
   } else {
      return itr->second;
   }
}

Int_t STreeTypeDecoder::GetCode( const TString& name ) const {

   std::map< TString, Int_t >::const_iterator itr;
   if( ( itr = m_forwardMap.find( name ) ) == m_forwardMap.end() ) {
      return m_unknownCode;
   } else {
      return itr->second;
   }
}

STreeTypeDecoder::STreeTypeDecoder()
   : m_forwardMap(), m_reverseMap() {

   AddType( "Flat input tree",      STreeType::InputSimpleTree );
   AddType( "Metadata input tree",  STreeType::InputMetaTree );
   AddType( "Flat output tree",     STreeType::OutputSimpleTree );
   AddType( "Metadata output tree", STreeType::OutputMetaTree );
}