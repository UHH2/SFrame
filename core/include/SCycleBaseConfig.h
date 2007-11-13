// Dear emacs, this is -*- c++ -*-
// $Id: SCycleBaseConfig.h,v 1.1.1.1 2007-11-13 12:42:21 krasznaa Exp $
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

#ifndef SFRAME_CORE_SCycleBaseConfig_H
#define SFRAME_CORE_SCycleBaseConfig_H

// STL include(s):
#include <vector>
#include <map>

// ROOT include(s):
#include <TString.h>

// Local include(s):
#include "SCycleBaseBase.h"
#include "SError.h"
#include "SInputData.h"

// Forward declaration(s):
class TXMLNode;

/**
 * This part of the SCycleBase class is responsible for handling the
 * configuration of the cycle. It can read the cycle's part of the configuration
 * XML, it handles the properties of the cycle and it provides all these
 * information for the "higher level" parts of the SCycleBase code.
 */
class SCycleBaseConfig : public virtual SCycleBaseBase {

public:
   SCycleBaseConfig();
   virtual ~SCycleBaseConfig();

   /// Function initialising the cycle:
   void Initialize( TXMLNode* ) throw( SError );

   //
   // Properties set by the cycle controller:
   //
   void SetOutputDirectory( const TString& outputdirectory ) { m_outputDirectory = outputdirectory; }
   void SetPostFix( const TString& postfix ) { m_postfix = postfix; }
   void SetTargetLumi( Double_t targetlumi ) { m_targetlumi = targetlumi; }

   const TString& GetOutputDirectory() const { return m_outputDirectory; }
   const TString& GetPostFix() const         { return m_postfix; }
   Double_t       GetTargetLumi() const      { return m_targetlumi; }

protected:
   //
   // Functions declaring the properties of the derived class:
   //
   void DeclareProperty( const std::string& name, std::string& value );
   void DeclareProperty( const std::string& name, int& value );
   void DeclareProperty( const std::string& name, double& value );
   void DeclareProperty( const std::string& name, std::vector< std::string >& value );
   void DeclareProperty( const std::string& name, std::vector< int >& value );
   void DeclareProperty( const std::string& name, std::vector< double >& value );

   /// List of input data to run over
   std::vector< SInputData > m_inputData;

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

   ClassDef( SCycleBaseConfig, 0 );

}; // class SCycleBaseConfig

#endif // SFRAME_CORE_SCycleBaseConfig_H
