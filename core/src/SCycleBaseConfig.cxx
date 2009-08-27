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

// System include(s):
#include <cstdlib>

// ROOT include(s):
#include <TString.h>
#include <TXMLNode.h>
#include <TList.h>
#include <TXMLAttr.h>

// Local include(s):
#include "../include/SCycleBaseConfig.h"
#include "../include/SGeneratorCut.h"

using namespace std;

/**
 * The constructor only initialises the base class.
 */
SCycleBaseConfig::SCycleBaseConfig() {

   m_logger << VERBOSE << "SCycleBaseConfig constructed" << SLogger::endmsg;

}

/**
 * Another one of the "I don't do anything" destructors.
 */
SCycleBaseConfig::~SCycleBaseConfig() {

   m_logger << VERBOSE << "SCycleBaseConfig destructed" << SLogger::endmsg;

}

/**
 * Basically everything about an analysis in SFrame is set up through
 * a main XML configuration file that describes the given analysis.
 * This function is called by the framework to initialise the cycle
 * based on the part of the XML that describes it.
 *
 * <strong>It should generally not be called by the user.</strong>
 *
 * @param node The top XML node describing the cycle
 */
void SCycleBaseConfig::Initialize( TXMLNode* node ) throw( SError ) {

   m_logger << INFO << "Initializing..." << SLogger::endmsg;

   // Clear the current cycle configuration:
   m_config.ClearConfig();

   //
   // Get the properties of the Cycle node:
   //
   TListIter attribIt = node->GetAttributes();
   TXMLAttr* curAttr = 0;
   while( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {
      if( curAttr->GetName() == TString( "TargetLumi" ) ) {
         m_config.SetTargetLumi( atof( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "RunMode" ) ) {
         SCycleConfig::RunMode mode = SCycleConfig::LOCAL;
         if( curAttr->GetValue() == TString( "LOCAL" ) )
            mode = SCycleConfig::LOCAL;
         else if( curAttr->GetValue() == TString( "PROOF" ) )
            mode = SCycleConfig::PROOF;
         else {
            m_logger << WARNING << "Running mode (\"" << curAttr->GetValue()
                     << "\") not recognised. Running locally!" << SLogger::endmsg;
         }
         m_config.SetRunMode( mode );
      } else if( curAttr->GetName() == TString( "ProofServer" ) ) {
         m_config.SetProofServer( curAttr->GetValue() );
      } else if( curAttr->GetName() == TString( "ProofWorkDir" ) ) {
         m_config.SetProofWorkDir( curAttr->GetValue() );
      } else if( curAttr->GetName() == TString( "ProofNodes" ) ) {
         m_config.SetProofNodes( atoi(curAttr->GetValue()) );
      } else if( curAttr->GetName() == TString( "OutputDirectory" ) ) {
         m_config.SetOutputDirectory( curAttr->GetValue() );
      } else if( curAttr->GetName() == TString( "PostFix" ) ) {
         m_config.SetPostFix( curAttr->GetValue() );
      }
   }

   // ------------- parse xml element ------------------------------------

   TXMLNode* nodes = node->GetChildren();
   while( nodes != 0 ) {
      if( ! nodes->HasChildren() ) {
         nodes = nodes->GetNextNode();
         continue;
      }

      if( nodes->GetNodeName() == TString( "InputData" ) ) {

         this->InitializeInputData( nodes );
   
      } else if( nodes->GetNodeName() == TString( "UserConfig" ) ) {

         this->InitializeUserConfig( nodes );

      }
      nodes = nodes->GetNextNode();
   }

   // now check if input data type appears multiple times
   m_config.ArrangeInputData();

   // print configuration
   m_config.PrintConfig();

   // ------------- xml parsing terminated -------------------------------

   return;

}

const SCycleConfig& SCycleBaseConfig::GetConfig() const {

   return m_config;

}

void SCycleBaseConfig::SetConfig( const SCycleConfig& config ) {

   //
   // Save the new configuration:
   //
   m_config = config;

   //
   // Set the user properties according to the new configuration:
   //
   for( SCycleConfig::property_type::const_iterator it =
           m_config.GetProperties().begin();
        it != m_config.GetProperties().end(); ++it ) {
      SetProperty( it->first, it->second );
   }

   return;

}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Implementation of the functions declaring the properties of the       //
// derived class                                                         //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

/**
 * Use this function in the constructor of your analysis cycle to declare
 * an std::string object as a property to be set up by the framework
 * according to the configuration set in the XML file.
 *
 * @param name  The name under which the property appears in XML
 * @param value The std::string object that you use as the property
 */
void SCycleBaseConfig::DeclareProperty( const std::string& name, std::string& value ) {

   m_stringPrefs[ name ] = &value;
   return;

}

/**
 * Use this function in the constructor of your analysis cycle to declare
 * an int variable as a property to be set up by the framework
 * according to the configuration set in the XML file.
 *
 * @param name  The name under which the property appears in XML
 * @param value The int variable that you use as the property
 */
void SCycleBaseConfig::DeclareProperty( const std::string& name, int& value ) {

   m_intPrefs[ name ] = &value;
   return;

}

/**
 * Use this function in the constructor of your analysis cycle to declare
 * a double variable as a property to be set up by the framework
 * according to the configuration set in the XML file.
 *
 * @param name  The name under which the property appears in XML
 * @param value The double variable that you use as the property
 */
void SCycleBaseConfig::DeclareProperty( const std::string& name, double& value ) {

   m_doublePrefs[ name ] = &value;
   return;

}

/**
 * Use this function in the constructor of your analysis cycle to declare
 * a bool variable as a property to be set up by the framework
 * according to the configuration set in the XML file. The boolean
 * property can be specified in a number of ways. Either with the
 * (case insensitive) "true" and "false" words, or with the 0 (false)
 * and 1 (true) numbers. (Actually all non-zero numbers are translated
 * as "true".)
 *
 * @param name  The name under which the property appears in XML
 * @param value The bool variable that you use as the property
 */
void SCycleBaseConfig::DeclareProperty( const std::string& name, bool& value ) {

   m_boolPrefs[ name ] = &value;
   return;

}

/**
 * Use this function in the constructor of your analysis cycle to declare
 * a vector of std::string objects as a property to be set up by the framework
 * according to the configuration set in the XML file. The strings have to be
 * put as a space separated list in the XML.
 *
 * @param name  The name under which the property appears in XML
 * @param value The object that you use as the property
 */
void SCycleBaseConfig::DeclareProperty( const std::string& name,
                                        std::vector< std::string >& value ) {

   m_stringListPrefs[ name ] = &value;
   return;

}

/**
 * Use this function in the constructor of your analysis cycle to declare
 * a vector of int variables as a property to be set up by the framework
 * according to the configuration set in the XML file. Ints have to be
 * put as a space separated list in the XML.
 *
 * @param name  The name under which the property appears in XML
 * @param value The object that you use as the property
 */
void SCycleBaseConfig::DeclareProperty( const std::string& name,
                                        std::vector< int >& value ) {

   m_intListPrefs[ name ] = &value;
   return;

}

/**
 * Use this function in the constructor of your analysis cycle to declare
 * a vector of double variables as a property to be set up by the framework
 * according to the configuration set in the XML file. Doubles have to be
 * put as a space separated list in the XML.
 *
 * @param name  The name under which the property appears in XML
 * @param value The object that you use as the property
 */
void SCycleBaseConfig::DeclareProperty( const std::string& name,
                                        std::vector< double >& value ) {

   m_doubleListPrefs[ name ] = &value;
   return;

}

/**
 * Use this function in the constructor of your analysis cycle to declare
 * a vector of bool variables as a property to be set up by the framework
 * according to the configuration set in the XML file. Bools have to be
 * put as a space separated list in the XML. A boolean
 * property can be specified in a number of ways. Either with the
 * (case insensitive) "true" and "false" words, or with the 0 (false)
 * and 1 (true) numbers. (Actually all non-zero numbers are translated
 * as "true".) They can be freely mixed.
 *
 * @param name  The name under which the property appears in XML
 * @param value The object that you use as the property
 */
void SCycleBaseConfig::DeclareProperty( const std::string& name,
                                        std::vector< bool >& value ) {

   m_boolListPrefs[ name ] = &value;
   return;

}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Implementation of the functions called by the public Initialize(...)  //
// that handle the different aspects of the initialisation.              //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

void SCycleBaseConfig::InitializeInputData( TXMLNode* node ) throw( SError ) {

   // create SInputData object
   SInputData inputData;

   TListIter attribIt( node->GetAttributes() );
   TXMLAttr* curAttr( 0 );
   while( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {

      if( curAttr->GetName() == TString( "Type" ) ) {
         inputData.SetType( curAttr->GetValue() );
         m_logger << INFO << "Reading SInputData: " << inputData.GetType()
                  << SLogger::endmsg;
      }
      if( curAttr->GetName() == TString( "Version" ) )
         inputData.SetVersion( curAttr->GetValue() );
      if( curAttr->GetName() == TString( "Lumi" ) )
         inputData.SetTotalLumi( atof( curAttr->GetValue() ) );
      if( curAttr->GetName() == TString( "NEventsMax" ) ) 
         inputData.SetNEventsMax( atoi( curAttr->GetValue() ) );
      if( curAttr->GetName() == TString( "NEventsSkip" ) ) 
         inputData.SetNEventsSkip( atoi( curAttr->GetValue() ) );
   }

   TXMLNode* child = node->GetChildren();
   while( child != 0 ) {
      if( ! child->HasAttributes() ) {
         child = child->GetNextNode();
         continue;
      }

      TListIter attributes( child->GetAttributes() );
      TXMLAttr* attribute = 0;

      // get the generator cuts
      if( child->GetNodeName() == TString( "GeneratorCut" ) ) {
         TString treeName = "";
         TString formula = "";
         attribute = 0;

         while( ( attribute = dynamic_cast< TXMLAttr* >( attributes() ) ) != 0) {
            if( attribute->GetName() == TString( "Tree" ) ) treeName = attribute->GetValue();
            if( attribute->GetName() == TString( "Formula" ) ) formula = attribute->GetValue();
         }
         inputData.AddGenCut( SGeneratorCut( treeName, formula ) );

      }
      // get the input files
      else if( child->GetNodeName() == TString( "In" ) ) {
         std::string fileName = "";
         std::string treeName = "";
         Double_t lumi = 0.;

         attribute = 0;
         while( ( attribute = dynamic_cast< TXMLAttr* >( attributes() ) ) != 0) {
            if( attribute->GetName() == TString( "FileName" ) )
               fileName = attribute->GetValue();
            if( attribute->GetName() == TString( "Lumi" ) )
               lumi = atof( attribute->GetValue() );
         }
         inputData.AddSFileIn( SFile( fileName, lumi ) );

      }
      // get a "regular" input tree
      else if( child->GetNodeName() == TString( "InputTree" ) ) {
         std::string treeName = "";

         attribute = 0;
         while( ( attribute = dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Name" ) )
               treeName = attribute->GetValue();
         }
         inputData.AddInputSTree( STree( treeName ) );

      }
      // get a "persistent" input tree (aka. DPD)
      else if( child->GetNodeName() == TString( "PersTree" ) ) {
         std::string treeName = "";

         attribute = 0;
         while( ( attribute = dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Name" ) )
               treeName = attribute->GetValue();
         }
         inputData.AddPersSTree( STree( treeName ) );

      }
      // get an EventView input tree
      else if( child->GetNodeName() == TString( "EVInputTree" ) ) {
         std::string baseName = "";
         Int_t number = 0;
         std::string collTreeName = "";

         attribute = 0;
         while( ( attribute = dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "BaseName" ) )
               baseName = attribute->GetValue();
            if( attribute->GetName() == TString( "Number" ) )
               number = atoi( attribute->GetValue() );
            if( attribute->GetName() == TString( "CollTreeName" ) )
               collTreeName = attribute->GetValue();
         }
         for( int i = 0; i < number; ++i ) {
            std::stringstream trName;
            trName << baseName << i;
            inputData.AddEVInputSTree( SEVTree( trName.str(), baseName,
                                                i, collTreeName ) );
         }

      }
      // get an output tree
      else if( child->GetNodeName() == TString( "OutputTree" ) ) {
         std::string treeName = "";

         attribute = 0;
         while( ( attribute = dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Name" ) )
               treeName = attribute->GetValue();
         }
         inputData.AddOutputSTree( STree( treeName ) );

      } else {
         // unknown field error
         m_logger << ERROR << " *** ERROR - unknown field " << child->GetNodeName()
                  << " ***" << SLogger::endmsg;
      }
      child = child->GetNextNode();
   }

   // Add this input data to the list:
   m_config.AddInputData( inputData );

   return;
}

void SCycleBaseConfig::InitializeUserConfig( TXMLNode* node ) throw( SError ) {

   TXMLNode* userNode = node->GetChildren();
   while( userNode != 0 ) {

      if( ! userNode->HasAttributes() ||
          ( userNode->GetNodeName() != TString( "Item" ) ) ) {
         userNode = userNode->GetNextNode();
         continue;
      }

      std::string name = "", stringValue = "";
      TListIter userAttributes( userNode->GetAttributes() );
      TXMLAttr* attribute = 0;
      while( ( attribute = dynamic_cast< TXMLAttr* >( userAttributes() ) ) != 0 ) {
         if( attribute->GetName() == TString( "Name" ) )
            name = attribute->GetValue();
         if( attribute->GetName() == TString( "Value" ) )
            stringValue = attribute->GetValue();
      }
      m_logger << DEBUG << "Found user property with name \"" << name
               << "\" and value \"" << stringValue << "\"" << SLogger::endmsg;

      m_config.SetProperty( name, stringValue );
      this->SetProperty( name, stringValue );

      userNode = userNode->GetNextNode();
   }

   return;
}

void SCycleBaseConfig::SetProperty( const std::string& name,
                                    const std::string& stringValue ) throw( SError ) {

   // If it's a string property:
   if( m_stringPrefs.find( name ) != m_stringPrefs.end() ) {
      ( *m_stringPrefs[ name ] ) = stringValue;
   }
   // If it's an integer property:
   else if( m_intPrefs.find( name ) != m_intPrefs.end() ) {
      int value = atoi( stringValue.c_str() );
      ( *m_intPrefs[ name ] ) = value;
   }
   // If it's a double property:
   else if( m_doublePrefs.find( name ) != m_doublePrefs.end() ) {
      double value = atof( stringValue.c_str() );
      ( *m_doublePrefs[ name ] ) = value;
   }
   // If it's a boolean property:
   else if( m_boolPrefs.find( name ) != m_boolPrefs.end() ) {
      ( *m_boolPrefs[ name ] ) = ToBool( stringValue );
   }
   // If it's a string list property:
   else if( m_stringListPrefs.find( name ) != m_stringListPrefs.end() ) {
      m_stringListPrefs[ name ]->clear();
      istringstream stream( stringValue );
      while( ! stream.eof() ) {
         std::string value;
         stream >> value;
         m_stringListPrefs[ name ]->push_back( value );
      }
   }
   // If it's an integer list property:
   else if( m_intListPrefs.find( name ) != m_intListPrefs.end() ) {
      m_intListPrefs[ name ]->clear();
      istringstream stream( stringValue );
      while( ! stream.eof() ) {
         int value;
         stream >> value;
         m_intListPrefs[ name ]->push_back( value );
      }
   }
   // If it's a double list property:
   else if( m_doubleListPrefs.find( name ) != m_doubleListPrefs.end() ) {
      m_doubleListPrefs[ name ]->clear();
      istringstream stream( stringValue );
      while( ! stream.eof() ) {
         double value;
         stream >> value;
         m_doubleListPrefs[ name ]->push_back( value );
      }
   }
   // If it's a boolean list property:
   else if( m_boolListPrefs.find( name ) != m_boolListPrefs.end() ) {
      m_boolListPrefs[ name ]->clear();
      istringstream stream( stringValue );
      while( ! stream.eof() ) {
         std::string value;
         stream >> value;
         m_boolListPrefs[ name ]->push_back( ToBool( value ) );
      }
   }
   // If it hasn't been requested by the analysis cycle, issue a warning.
   // It might mean a typo somewhere...
   else {
      m_logger << WARNING << "User property not found: " << name << endl
               << "Value not set!" << SLogger::endmsg;
   }

   return;

}

/**
 * This function is used in InitializeUserConfig to translate the value(s)
 * given in the XML configuration to boolean values.
 */
bool SCycleBaseConfig::ToBool( const std::string& value ) throw( SError ) {

   TString tvalue( value );
   if( tvalue.IsAlpha() ) {
      if( tvalue.Contains( "true", TString::kIgnoreCase ) ) {
         return true;
      } else if( tvalue.Contains( "false", TString::kIgnoreCase ) ) {
         return false;
      }
   } else if( tvalue.IsDigit() ) {
      return tvalue.Atoi();
   }

   SError error( SError::SkipCycle );
   error << "Can't translate \"" << value << "\" to boolean";
   throw error;

   return false;

}
