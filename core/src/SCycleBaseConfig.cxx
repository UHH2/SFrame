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

// System include(s):
#include <cstdlib>
#include <sstream>

// ROOT include(s):
#include <TString.h>
#include <TXMLNode.h>
#include <TList.h>
#include <TXMLAttr.h>
#include <TSystem.h>

// Local include(s):
#include "../include/SCycleBaseConfig.h"
#include "../include/SGeneratorCut.h"
#include "../include/STreeTypeDecoder.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleBaseConfig )
#endif // DOXYGEN_IGNORE

/// Macro used in the DeclareProperty(...) functions
#define CHECK_FOR_DUPLICATES(NAME) {                                    \
      if( ( m_stringPrefs.find( NAME ) !=                               \
            m_stringPrefs.end() ) ||                                    \
          ( m_intPrefs.find( NAME ) !=                                  \
            m_intPrefs.end() ) ||                                       \
          ( m_doublePrefs.find( NAME ) !=                               \
            m_doublePrefs.end() ) ||                                    \
          ( m_boolPrefs.find( NAME ) !=                                 \
            m_boolPrefs.end() ) ||                                      \
          ( m_stringListPrefs.find( NAME ) !=                           \
            m_stringListPrefs.end() ) ||                                \
          ( m_intListPrefs.find( NAME ) !=                              \
            m_intListPrefs.end() ) ||                                   \
          ( m_doubleListPrefs.find( NAME ) !=                           \
            m_doubleListPrefs.end() ) ||                                \
          ( m_boolListPrefs.find( NAME ) !=                             \
            m_boolListPrefs.end() ) ) {                                 \
         REPORT_ERROR( "The property name \"" << NAME                   \
                       << "\" is used in multiple locations!" );        \
         REPORT_ERROR( "Some parts of the code will not be "            \
                       "configured correctly!" );                       \
      }                                                                 \
   } while( 0 )

namespace {

   /**
    * This little helper function is used to translate the vector type user
    * properties into simple strings. Right now all types stored in vectors
    * can be put into an output stream by default, we only need to care
    * about handling the "vector part" correctly.
    *
    * @param out The output stream to write the vector values to
    * @param vec The vector to write to the output stream
    * @returns The output stream
    */
   template< typename T >
   std::ostream& operator<<( std::ostream& out,
                             const std::vector< T >& vec ) {

      for( size_t i = 0; i < vec.size(); ++i ) {
         out << vec[ i ];
         if( i + 1 < vec.size() ) out << " ";
      }

      return out;
   }

} // private namespace

/**
 * The constructor only initialises the base class.
 */
SCycleBaseConfig::SCycleBaseConfig()
   : m_input( 0 ) {

   REPORT_VERBOSE( "SCycleBaseConfig constructed" );
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

   m_logger << INFO << "Initializing from configuration" << SLogger::endmsg;

   // Clear the current cycle configuration:
   m_config.ClearConfig();
   REPORT_VERBOSE( "Cleared the current configuration" );

   // Set the name of the cycle in the configuration:
   m_config.SetCycleName( m_logger.GetSource() );

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
                     << "\") not recognised. Running locally!"
                     << SLogger::endmsg;
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
      } else if( curAttr->GetName() == TString( "UseTreeCache" ) ) {
         m_config.SetUseTreeCache( ToBool( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "TreeCacheSize" ) ) {
         m_config.SetCacheSize( atoi( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "TreeCacheLearnEntries" ) ) {
         m_config.SetCacheLearnEntries( atoi( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "ProcessOnlyLocal" ) ) {
         m_config.SetProcessOnlyLocal( ToBool( curAttr->GetValue() ) );
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

         m_config.AddInputData( this->InitializeInputData( nodes ) );

      } else if( nodes->GetNodeName() == TString( "UserConfig" ) ) {

         this->InitializeUserConfig( nodes );

      }
      nodes = nodes->GetNextNode();
   }

   // now check if input data type appears multiple times
   m_config.ArrangeInputData();
   REPORT_VERBOSE( "Arranged the input data" );

   // print configuration
   m_config.PrintConfig();

   // ------------- xml parsing terminated -------------------------------

   return;
}

const SCycleConfig& SCycleBaseConfig::GetConfig() const {

   return m_config;
}

SCycleConfig& SCycleBaseConfig::GetConfig() {

   return m_config;
}

void SCycleBaseConfig::SetConfig( const SCycleConfig& config ) {

   //
   // Save the new configuration:
   //
   m_config = config;

   // Clear the configuration name cache:
   m_configuredPrefs.clear();

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

const TList& SCycleBaseConfig::GetConfigurationObjects() const {

   return m_configList;
}

void SCycleBaseConfig::SetConfInput( TList* input ) {

   m_input = input;
   return;
}

TList* SCycleBaseConfig::GetConfInput() const {

   return m_input;
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
void SCycleBaseConfig::DeclareProperty( const std::string& name,
                                        std::string& value ) {

   // Check if the property name is still available:
   CHECK_FOR_DUPLICATES( name );

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

   // Check if the property name is still available:
   CHECK_FOR_DUPLICATES( name );

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
void SCycleBaseConfig::DeclareProperty( const std::string& name,
                                        double& value ) {

   // Check if the property name is still available:
   CHECK_FOR_DUPLICATES( name );

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

   // Check if the property name is still available:
   CHECK_FOR_DUPLICATES( name );

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

   // Check if the property name is still available:
   CHECK_FOR_DUPLICATES( name );

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

   // Check if the property name is still available:
   CHECK_FOR_DUPLICATES( name );

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

   // Check if the property name is still available:
   CHECK_FOR_DUPLICATES( name );

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

   // Check if the property name is still available:
   CHECK_FOR_DUPLICATES( name );

   m_boolListPrefs[ name ] = &value;
   return;

}

void SCycleBaseConfig::AddConfigObject( TObject* object ) {

   m_configList.Add( object );
   return;
}

TObject* SCycleBaseConfig::GetConfigObject( const char* name ) const {

   if( ! m_input ) return 0;

   return m_input->FindObject( name );
}

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Implementation of the functions called by the public Initialize(...)  //
// that handle the different aspects of the initialisation.              //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

SInputData
SCycleBaseConfig::InitializeInputData( TXMLNode* node ) throw( SError ) {

   // Create the SInputData object
   SInputData inputData;

   REPORT_VERBOSE( "Reading an input data definition" );

   TListIter attribIt( node->GetAttributes() );
   TXMLAttr* curAttr( 0 );
   while( ( curAttr = dynamic_cast< TXMLAttr* >( attribIt() ) ) != 0 ) {

      if( curAttr->GetName() == TString( "Type" ) ) {
         inputData.SetType( curAttr->GetValue() );
      } else if( curAttr->GetName() == TString( "Version" ) ) {
         inputData.SetVersion( curAttr->GetValue() );
      } else if( curAttr->GetName() == TString( "Lumi" ) ) {
         inputData.SetTotalLumi( atof( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "NEventsMax" ) ) {
         inputData.SetNEventsMax( atoi( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "NEventsSkip" ) ) {
         inputData.SetNEventsSkip( atoi( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "Cacheable" ) ) {
         inputData.SetCacheable( ToBool( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "SkipValid" ) ) {
         inputData.SetSkipValid( ToBool( curAttr->GetValue() ) );
      } else if( curAttr->GetName() == TString( "SkipLookup" ) ) {
         inputData.SetSkipLookup( ToBool( curAttr->GetValue() ) );
      }
   }

   m_logger << INFO << "Reading SInputData: " << inputData.GetType()
            << " - " << inputData.GetVersion() << SLogger::endmsg;

   // Access the tree type decoder:
   const STreeTypeDecoder* decoder = STreeTypeDecoder::Instance();

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
         while( ( attribute =
                  dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Tree" ) ) {
               treeName = attribute->GetValue();
            }
            if( attribute->GetName() == TString( "Formula" ) ) {
               formula = attribute->GetValue();
            }
         }

         REPORT_VERBOSE( "Found a generator cut on tree \"" << treeName
                         << "\" with formula: " << formula );
         inputData.AddGenCut( SGeneratorCut( treeName, formula ) );

      }
      // get the input datasets
      else if( child->GetNodeName() == TString( "DataSet" ) ) {

         TString name = "";
         Double_t lumi = 0.;
         attribute = 0;
         while( ( attribute =
                  dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Name" ) )
               name = attribute->GetValue();
            if( attribute->GetName() == TString( "Lumi" ) )
               lumi = atof( attribute->GetValue() );
         }

         REPORT_VERBOSE( "Found a dataset with name \"" << name
                         << "\" and lumi: " << lumi );
         inputData.AddDataSet( SDataSet( name, lumi ) );

      }
      // get the input files
      else if( child->GetNodeName() == TString( "In" ) ) {

         TString fileName = "";
         Double_t lumi = 0.;
         attribute = 0;
         while( ( attribute =
                  dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "FileName" ) )
               fileName = attribute->GetValue();
            if( attribute->GetName() == TString( "Lumi" ) )
               lumi = atof( attribute->GetValue() );
         }

         REPORT_VERBOSE( "Found an input file with name \"" << fileName
                         << "\" and lumi: " << lumi );
         inputData.AddSFileIn( SFile( fileName, lumi ) );

      }
      // get a "regular" input tree
      else if( child->GetNodeName() == TString( "InputTree" ) ) {

         TString treeName = "";
         attribute = 0;
         while( ( attribute =
                  dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Name" ) )
               treeName = attribute->GetValue();
         }

         REPORT_VERBOSE( "Found regular input tree with name: " << treeName );
         inputData.AddTree( decoder->GetXMLCode( "InputTree" ),
                            STree( treeName, ( STree::INPUT_TREE |
                                               STree::EVENT_TREE ) ) );

      }
      // get an output tree
      else if( child->GetNodeName() == TString( "OutputTree" ) ) {

         TString treeName = "";
         attribute = 0;
         while( ( attribute =
                  dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Name" ) )
               treeName = attribute->GetValue();
         }

         REPORT_VERBOSE( "Found regular output tree with name: " << treeName );
         inputData.AddTree( decoder->GetXMLCode( "OutputTree" ),
                            STree( treeName, ( STree::OUTPUT_TREE |
                                               STree::EVENT_TREE ) ) );

      }
      // get an input metadata tree
      else if( child->GetNodeName() == TString( "MetadataInputTree" ) ) {

         TString treeName = "";
         attribute = 0;
         while( ( attribute =
                  dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Name" ) )
               treeName = attribute->GetValue();
         }

         REPORT_VERBOSE( "Found input metadata tree with name: " << treeName );
         inputData.AddTree( decoder->GetXMLCode( "MetadataInputTree" ),
                            STree( treeName, STree::INPUT_TREE ) );

      }
      // get an output metadata tree
      else if( child->GetNodeName() == TString( "MetadataOutputTree" ) ) {

         TString treeName = "";
         attribute = 0;
         while( ( attribute =
                  dynamic_cast< TXMLAttr* >( attributes() ) ) != 0 ) {
            if( attribute->GetName() == TString( "Name" ) )
               treeName = attribute->GetValue();
         }

         REPORT_VERBOSE( "Found output metadata tree with name: " << treeName );
         inputData.AddTree( decoder->GetXMLCode( "MetadataOutputTree" ),
                            STree( treeName, STree::OUTPUT_TREE ) );

      } else {
         // Unknown field notification. It's not an ERROR anymore, as this
         // function may actually find XML nodes that it doesn't recognise.
         m_logger << DEBUG << "Unknown field: " << child->GetNodeName()
                  << SLogger::endmsg;
      }
      child = child->GetNextNode();
   }

   return inputData;
}

void SCycleBaseConfig::InitializeUserConfig( TXMLNode* node ) throw( SError ) {

   REPORT_VERBOSE( "Initializing the user configuration" );

   // Clear the configuration name cache:
   m_configuredPrefs.clear();

   TXMLNode* userNode = node->GetChildren();
   while( userNode != 0 ) {

      REPORT_VERBOSE( "Node name " << userNode->GetNodeName() << " has attrib "
                      << userNode->HasAttributes() );

      if( ! userNode->HasAttributes() ||
          ( userNode->GetNodeName() != TString( "Item" ) ) ) {
         userNode = userNode->GetNextNode();
         continue;
      }

      std::string name = "", stringValue = "";
      TListIter userAttributes( userNode->GetAttributes() );
      TXMLAttr* attribute = 0;
      while( ( attribute = dynamic_cast< TXMLAttr* >( userAttributes() ) ) !=
             0 ) {
         if( attribute->GetName() == TString( "Name" ) )
            name = attribute->GetValue();
         if( attribute->GetName() == TString( "Value" ) )
            stringValue = DecodeEnvVar( attribute->GetValue() );
      }
      m_logger << DEBUG << "Found user property with name \"" << name
               << "\" and value \"" << stringValue << "\"" << SLogger::endmsg;

      this->SetProperty( name, stringValue );

      userNode = userNode->GetNextNode();
   }

   // Add all user properties to the SCycleConfig object, not just the ones
   // which have been set explicitly in the XML file:
   AddUserOptions( m_stringPrefs );
   AddUserOptions( m_intPrefs );
   AddUserOptions( m_doublePrefs );
   AddUserOptions( m_boolPrefs );
   AddUserOptions( m_stringListPrefs );
   AddUserOptions( m_intListPrefs );
   AddUserOptions( m_doubleListPrefs );
   AddUserOptions( m_boolListPrefs );

   return;
}

void SCycleBaseConfig::
SetProperty( const std::string& name,
             const std::string& stringValue ) throw( SError ) {

   // Check if the user is specifying the same property multiple times.
   // XML doesn't guarantee in which order the properties are getting
   // read, so print explicitly what's happening.
   if( ! m_configuredPrefs.insert( name ).second ) {
      m_logger << WARNING << "Property \"" << name
               << "\" is getting set multiple times" << SLogger::endmsg;
      m_logger << WARNING << "Now taking value: " << stringValue
               << SLogger::endmsg;
   }

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
      std::istringstream stream( stringValue );
      while( ! stream.eof() && ( stringValue != "" ) ) {
         std::string value;
         stream >> value;
         m_stringListPrefs[ name ]->push_back( value );
      }
   }
   // If it's an integer list property:
   else if( m_intListPrefs.find( name ) != m_intListPrefs.end() ) {
      m_intListPrefs[ name ]->clear();
      std::istringstream stream( stringValue );
      while( ! stream.eof() && ( stringValue != "" ) ) {
         int value;
         stream >> value;
         m_intListPrefs[ name ]->push_back( value );
      }
   }
   // If it's a double list property:
   else if( m_doubleListPrefs.find( name ) != m_doubleListPrefs.end() ) {
      m_doubleListPrefs[ name ]->clear();
      std::istringstream stream( stringValue );
      while( ! stream.eof() && ( stringValue != "" ) ) {
         double value;
         stream >> value;
         m_doubleListPrefs[ name ]->push_back( value );
      }
   }
   // If it's a boolean list property:
   else if( m_boolListPrefs.find( name ) != m_boolListPrefs.end() ) {
      m_boolListPrefs[ name ]->clear();
      std::istringstream stream( stringValue );
      while( ! stream.eof() && ( stringValue != "" ) ) {
         std::string value;
         stream >> value;
         m_boolListPrefs[ name ]->push_back( ToBool( value ) );
      }
   }
   // If it hasn't been requested by the analysis cycle, issue a warning.
   // It might mean a typo somewhere...
   else {
      m_logger << WARNING << "User property not found: " << name << std::endl
               << "  Value not set!" << SLogger::endmsg;
   }

   return;
}

/**
 * This function simply uses TSystem to do the environment variable expansion in
 * path names. One can use either "$SOMETHING" or "$(SOMETHING)" in the path
 * names. (The latter is also Win32 compatible, not that it matters for
 * SFrame...)
 *
 * Note that now the expansion is only done if the property begins with ":exp:".
 * If it does, then these 5 characters are removed from the beginning of the
 * string, and the rest of the string is given to TSystem for expansion.
 *
 * @param value The property that you want expanded based on the environment
 *              settings
 * @returns The path name that was expanded to be a real file name
 */
std::string SCycleBaseConfig::DecodeEnvVar( const std::string& value ) const { 

   // If the string doesn't begin with ":exp:", then do nothing:
   if( value.find( ":exp:" ) != 0 ) {
      return value;
   }

   // TSystem operates on TString objects. Note that we remove the ":exp:" from
   // the beginning of the string here:
   TString result( value.substr( 5, value.npos ) );

   // Let TSystem do the expansion:
   if( gSystem->ExpandPathName( result ) ) {
      REPORT_ERROR( "Failed 'expanding' property: " << value );
      return value;
   }

   return result.Data(); 
} 

/**
 * This function is used in InitializeUserConfig to translate the value(s)
 * given in the XML configuration to boolean values.
 *
 * Any capitalization of "true" and "false", and numerical values are all
 * acceptable.
 *
 * @param value The string to be interpreted as a boolean value
 * @returns A boolean value made from the string
 */
bool SCycleBaseConfig::ToBool( const std::string& value ) throw( SError ) {

   // The decoding is done using TString:
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

   // Report the decoding problem:
   REPORT_ERROR( "Can't translate \"" << value << "\" to boolean value" );
   SError error( SError::SkipCycle );
   error << "Can't translate \"" << value << "\" to boolean value";
   throw error;

   // This is just here for completeness...
   return false;
}

/**
 * This is just a convenience function for filling up the SCycleConfig object
 * with *every* configuration option. While for running purposes it would be
 * enough to just remember the properties which have been explicitly specified
 * in the configuration XML, in order to write out the full configuration of the
 * cycle into the output file, one has to take the state of the configuration
 * options after all of them have been set.
 *
 * @param prefs One of the preference maps
 */
template< typename T >
void SCycleBaseConfig::
AddUserOptions( const std::map< const std::string, T* >& prefs ) {

   // Iterate over all properties of this type:
   typename std::map< const std::string, T* >::const_iterator itr =
      prefs.begin();
   typename std::map< const std::string, T* >::const_iterator end =
      prefs.end();
   for( ; itr != end; ++itr ) {
      // Translate the property value to a string:
      std::ostringstream value;
      value << *( itr->second );
      // Set the property:
      m_config.SetProperty( itr->first, value.str() );
   }

   return;
}
