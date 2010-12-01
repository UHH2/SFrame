// Dear emacs, this is -*- c++ -*-
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

#ifndef SFRAME_CORE_SCycleBaseConfig_H
#define SFRAME_CORE_SCycleBaseConfig_H

// STL include(s):
#include <vector>
#include <map>
#include <string>

// ROOT include(s):
#include "TList.h"

// Local include(s):
#include "ISCycleBaseConfig.h"
#include "SCycleBaseBase.h"
#include "SError.h"
#include "SCycleConfig.h"

// Forward declaration(s):
class TXMLNode;

/**
 *   @short Base class for reading the XML configuration
 *
 *          This part of the SCycleBase class is responsible
 *          for handling the configuration of the cycle. It can
 *          read the cycle's part of the configuration XML, it
 *          handles the properties of the cycle and it provides
 *          all these information for the "higher level" parts
 *          of the SCycleBase code.
 *
 * @version $Revision$
 */
class SCycleBaseConfig : public virtual ISCycleBaseConfig,
                         public virtual SCycleBaseBase {

public:
   /// Default constructor
   SCycleBaseConfig();

   /// Function initialising the cycle
   void Initialize( TXMLNode* ) throw( SError );

   /// Get the overall cycle configuration object
   const SCycleConfig& GetConfig() const;
   /// Get the overall cycle configuration object
   SCycleConfig& GetConfig();
   /// Set the overall cycle configuration
   void SetConfig( const SCycleConfig& config );

   /// Get the list of all declared configuration objects
   virtual const TList& GetConfigurationObjects() const;

   /// Set which list should be used for the configuration input
   virtual void SetConfInput( TList* input );
   /// Check which list should be used for the configuration input
   virtual TList* GetConfInput() const;

   /// Declare a <strong>std::string</strong> property
   void DeclareProperty( const std::string& name, std::string& value );
   /// Declare an <strong>int</strong> property
   void DeclareProperty( const std::string& name, int& value );
   /// Declare a <strong>double</strong> property
   void DeclareProperty( const std::string& name, double& value );
   /// Declare a <strong>bool</strong> property
   void DeclareProperty( const std::string& name, bool& value );
   /// Declare a <strong>std::vector<std::string></strong> property
   void DeclareProperty( const std::string& name, std::vector< std::string >& value );
   /// Declare a <strong>std::vector<int></strong> property
   void DeclareProperty( const std::string& name, std::vector< int >& value );
   /// Declare a <strong>std::vector<double></strong> property
   void DeclareProperty( const std::string& name, std::vector< double >& value );
   /// Declare a <strong>std::vector<bool></strong> property
   void DeclareProperty( const std::string& name, std::vector< bool >& value );

   /// Add a configuration object that should be available on the PROOF nodes
   void AddConfigObject( TObject* object );
   /// Get a configuration object on the PROOF nodes
   TObject* GetConfigObject( const char* name ) const;

protected:
   /// Function that reads an InputData definition
   virtual SInputData InitializeInputData( TXMLNode* ) throw( SError );
   /// Function that reads the user properties from the XML
   virtual void InitializeUserConfig( TXMLNode* ) throw( SError );

private:
   /// Internal function for setting a property value
   void SetProperty( const std::string& name,
                     const std::string& value ) throw( SError );

   /// Function for decoding a string to bool:
   bool ToBool( const std::string& value ) throw( SError );

   //
   // These are the object used to handle the preferences of the
   // derived classes. Note that because of some "improvements" in
   // the ROOT code, now these have to be very strictly hidden from
   // the dictionary generation. Unfortunately that doesn't work right
   // now, so all SFrame jobs in ROOT 5.28 will always print some
   // "harmless" ERROR messages.
   //
   //#ifndef __MAKECINT__
   //#ifndef G__DICTIONARY
   std::map< const std::string, std::string* >                m_stringPrefs;
   std::map< const std::string, int* >                        m_intPrefs;
   std::map< const std::string, double* >                     m_doublePrefs;
   std::map< const std::string, bool* >                       m_boolPrefs;
   std::map< const std::string, std::vector< std::string >* > m_stringListPrefs;
   std::map< const std::string, std::vector< int >* >         m_intListPrefs;
   std::map< const std::string, std::vector< double >* >      m_doubleListPrefs;
   std::map< const std::string, std::vector< bool >* >        m_boolListPrefs;
   //#endif // G_DICTIONARY
   //#endif // __MAKECINT__

   /// The cycle configuration:
   SCycleConfig m_config;

   /// A list of all the configuration objects
   TList m_configList;
   /// List with the PROOF input objects
   TList* m_input;

}; // class SCycleBaseConfig

#endif // SFRAME_CORE_SCycleBaseConfig_H
