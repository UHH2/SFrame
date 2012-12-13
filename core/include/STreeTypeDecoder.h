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

#ifndef SFRAME_CORE_STreeTypeDecoder_H
#define SFRAME_CORE_STreeTypeDecoder_H

// STL include(s):
#include <map>

// ROOT include(s):
#include <TString.h>

/**
 *   @short Helper class for translating between tree type codes and names
 *
 *          In the new design of handling trees by SFrame, any kind of new type
 *          of input and output trees can be defined by the SFrame extension
 *          libraries. For instance now the base library doesn't need to know
 *          about persistent trees, SFrameARA is able to define those on its
 *          own.
 *
 *          The code now uses integer tree types to describe a tree to the
 *          framework. The constants declaring these tree types should all be in
 *          the STreeType namespace. To make the code able to display
 *          information about the known tree types in a nice way, it uses this
 *          "decoder" class to know what kind of tree it's handling at the
 *          moment.
 *
 *          The class is implemented as a singleton, as really only one such
 *          dictionary is needed for the process. The singleton instance knows
 *          on its own about the tree types that can be handled by plain SFrame.
 *          Extension libraries should use the AddType(...) function to teach
 *          new tree types to the dictionary. (For an example see the SFrameARA
 *          implementation.)
 *
 * @version $Revision$
 */
class STreeTypeDecoder {

public:
   /// Function accessing the singleton object instance
   static STreeTypeDecoder* Instance();

   /// Function adding a new type to the dictionary
   void AddType( const TString& xmlName, const TString& name, Int_t code );

   /// Get the name belonging to a given code
   const TString& GetName( Int_t code ) const;
   /// Get the code belonging to a given name
   Int_t GetCode( const TString& name ) const;

   /// Get the XML name belonging to a given code
   const TString& GetXMLName( Int_t code ) const;
   /// Get the code belonging to an XML name
   Int_t GetXMLCode( const TString& name ) const;

private:
   /// The constructor is private, to implement the singleton pattern
   STreeTypeDecoder();

   std::map< TString, Int_t > m_forwardMap; ///< Map assigning codes to names
   std::map< Int_t, TString > m_reverseMap; ///< Map assigning names to codes

   /// Map associating codes to XML names
   std::map< TString, Int_t > m_xmlForwardMap;
   /// Map associating XML names to codes
   std::map< Int_t, TString > m_xmlReverseMap;

   /// A variable needed for technical reasons
   static const TString m_unknownName;
   /// A variable used to be symmetric
   static const Int_t   m_unknownCode;

   static STreeTypeDecoder* m_instance; ///< Pointer to the singleton instance

}; // class STreeTypeDecoder

#endif // SFRAME_CORE_STreeTypeDecoder_H
