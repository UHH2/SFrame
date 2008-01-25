// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBaseConfig.h,v 1.3 2008-01-25 14:33:53 krasznaa Exp $
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

// ROOT include(s):
#include <TString.h>

// Local include(s):
#include "ISCycleBaseConfig.h"
#include "SCycleBaseBase.h"
#include "SError.h"
#include "SInputData.h"

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
 * @version $Revision: 1.3 $
 */
class SCycleBaseConfig : public virtual ISCycleBaseConfig,
                         public virtual SCycleBaseBase {

public:
   /// Default constructor
   SCycleBaseConfig();
   /// Default destructor
   virtual ~SCycleBaseConfig();

   /// Function initialising the cycle
   void Initialize( TXMLNode* ) throw( SError );

   /// Set the output directory
   /**
    * It is specified in the XML file where the output ROOT
    * files should be placed. But since this configuration is
    * not accessible to the Initialize method directly, it
    * has to be set by SCycleController using this function.
    *
    * @see SCycleBaseConfig::GetOutputDirectory
    */
   void SetOutputDirectory( const TString& outputdirectory ) {
      m_outputDirectory = outputdirectory;
   }
   /// Set the output file post-fix
   /**
    * The post-fix that should be appended to the output ROOT
    * files is specified in the XML config file. But since this
    * configuration is not accessible to the Initialize method
    * directly, it has to be set by SCycleController using
    * this function.
    *
    * @see SCycleBaseConfig::GetPostFix
    */
   void SetPostFix( const TString& postfix ) { m_postfix = postfix; }
   /// Set the target normalisation luminosity
   /**
    * The total integrated luminosity to which all plots should
    * be normalised is specified in the XML config file. But
    * since this configuration is not accessible to the Initialize
    * method directly, it has to be set by SCycleController using
    * this function.
    *
    * @see SCycleBaseConfig::GetTargetLumi
    */
   void SetTargetLumi( Double_t targetlumi ) { m_targetlumi = targetlumi; }

   /// Get the name of the output directory
   /**
    * @see SCycleBaseConfig::SetOutputDirectory
    */
   const TString& GetOutputDirectory() const { return m_outputDirectory; }
   /// Get the output file post-fix
   /**
    * @see SCycleBaseConfig::SetPostFix
    */
   const TString& GetPostFix() const { return m_postfix; }
   /// Get the target normalisation luminosity
   /**
    * @see SCycleBaseConfig::SetTargetLumi
    */
   Double_t GetTargetLumi() const { return m_targetlumi; }

protected:
   /// Declare a <strong>std::string</strong> property
   void DeclareProperty( const std::string& name, std::string& value );
   /// Declare an <strong>int</strong> property
   void DeclareProperty( const std::string& name, int& value );
   /// Declare a <strong>double</strong> property
   void DeclareProperty( const std::string& name, double& value );
   /// Declare a <strong>std::vector<std::string></strong> property
   void DeclareProperty( const std::string& name, std::vector< std::string >& value );
   /// Declare a <strong>std::vector<int></strong> property
   void DeclareProperty( const std::string& name, std::vector< int >& value );
   /// Declare a <strong>std::vector<double></strong> property
   void DeclareProperty( const std::string& name, std::vector< double >& value );

private:
   //
   // Functions initialising different aspects of the cycle:
   //
   void InitializeInputData( TXMLNode* ) throw( SError );
   void InitializeUserConfig( TXMLNode* ) throw( SError );
   void CheckForMultipleInputData() throw ( SError );

   //
   // Properties set on the cycle level:
   //
   TString m_outputDirectory;
   TString m_postfix;
   Double_t m_targetlumi;

   //
   // These are the object used to handle the preferences of the
   // derived classes:
   //
   std::map< const std::string, std::string* >                m_stringPrefs;
   std::map< const std::string, int* >                        m_intPrefs;
   std::map< const std::string, double* >                     m_doublePrefs;
   std::map< const std::string, std::vector< std::string >* > m_stringListPrefs;
   std::map< const std::string, std::vector< int >* >         m_intListPrefs;
   std::map< const std::string, std::vector< double >* >      m_doubleListPrefs;

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleBaseConfig, 0 );
#endif // DOXYGEN_IGNORE

}; // class SCycleBaseConfig

#endif // SFRAME_CORE_SCycleBaseConfig_H
