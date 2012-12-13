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

// Local include(s):
#include "../include/STreeTypeDecoder.h"
#include "../include/STreeType.h"

// Initialize the static variable(s):
STreeTypeDecoder* STreeTypeDecoder::m_instance = 0;
const TString STreeTypeDecoder::m_unknownName = "Unknown";
const Int_t STreeTypeDecoder::m_unknownCode = -1;

/**
 * This function implements the singleton pattern.
 *
 * @returns The only instance of the STreeTypeDecoder object
 */
STreeTypeDecoder* STreeTypeDecoder::Instance() {

   if( ! m_instance ) {
      m_instance = new STreeTypeDecoder();
   }
   return m_instance;
}

/**
 * @param xmlName XML node name describing this tree type
 * @param name Human readable description for the tree type
 * @param code Code to identify this tree type in SInputData objects
 */
void STreeTypeDecoder::AddType( const TString& xmlName, const TString& name,
                                Int_t code ) {

   m_forwardMap[ name ] = code;
   m_reverseMap[ code ] = name;

   m_xmlForwardMap[ xmlName ] = code;
   m_xmlReverseMap[ code ] = xmlName;

   return;
}

/**
 * @param code The code to look up the description for
 * @returns The human readable description for the specified tree type
 */
const TString& STreeTypeDecoder::GetName( Int_t code ) const {

   std::map< Int_t, TString >::const_iterator itr;
   if( ( itr = m_reverseMap.find( code ) ) == m_reverseMap.end() ) {
      return m_unknownName;
   } else {
      return itr->second;
   }
}

/**
 * @param name The human readable description to look up the code for
 * @returns The code identifying this tree type in SInputData objects
 */
Int_t STreeTypeDecoder::GetCode( const TString& name ) const {

   std::map< TString, Int_t >::const_iterator itr;
   if( ( itr = m_forwardMap.find( name ) ) == m_forwardMap.end() ) {
      return m_unknownCode;
   } else {
      return itr->second;
   }
}

/**
 * @param code The code to look up the XML node name for
 * @returns The XML node name for the specified tree type
 */
const TString& STreeTypeDecoder::GetXMLName( Int_t code ) const {

   std::map< Int_t, TString >::const_iterator itr;
   if( ( itr = m_xmlReverseMap.find( code ) ) == m_xmlReverseMap.end() ) {
      return m_unknownName;
   } else {
      return itr->second;
   }
}

/**
 * @param name The XML node name to look up the code for
 * @returns The code identifying this tree type in SInputData objects
 */
Int_t STreeTypeDecoder::GetXMLCode( const TString& name ) const {

   std::map< TString, Int_t >::const_iterator itr;
   if( ( itr = m_xmlForwardMap.find( name ) ) == m_xmlForwardMap.end() ) {
      return m_unknownCode;
   } else {
      return itr->second;
   }
}

/**
 * The constructor takes care of teaching all the tree types to the singleton
 * object that the SFrameCore library can handle by itself.
 */
STreeTypeDecoder::STreeTypeDecoder()
   : m_forwardMap(), m_reverseMap() {

   AddType( "InputTree",         "Flat input tree",
            STreeType::InputSimpleTree );
   AddType( "MetadataInputTree", "Metadata input tree",
            STreeType::InputMetaTree );
   AddType( "OutputTree",         "Flat output tree",
            STreeType::OutputSimpleTree );
   AddType( "MetadataOutputTree", "Metadata output tree",
            STreeType::OutputMetaTree );
}
