// $Id: SCycleConfig.cxx,v 1.1.2.3 2009-01-21 14:34:54 krasznaa Exp $

// STL include(s):
#include <map>

// Local include(s):
#include "../include/SCycleConfig.h"
#include "../include/SLogger.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleConfig );
#endif // DOXYGEN_IGNORE

SCycleConfig::SCycleConfig( const char* name )
   : TNamed( name, "SFrame cycle configuration" ), m_mode( LOCAL ),
     m_server( "" ), m_properties(), m_inputData(),
     m_targetLumi( 1. ) {

}

SCycleConfig::RunMode SCycleConfig::GetRunMode() const {

   return m_mode;

}

void SCycleConfig::SetRunMode( SCycleConfig::RunMode mode ) {

   m_mode = mode;
   return;

}

const TString& SCycleConfig::GetProofServer() const {

   return m_server;

}

void SCycleConfig::SetProofServer( const TString& server ) {

   m_server = server;
   return;

}

const SCycleConfig::property_type& SCycleConfig::GetProperties() const {

   return m_properties;

}

void SCycleConfig::SetProperty( const std::string& name, const std::string& value ) {

   m_properties.push_back( std::make_pair( name, value ) );
   return;

}

const SCycleConfig::id_type& SCycleConfig::GetInputData() const {

   return m_inputData;

}

void SCycleConfig::AddInputData( const SInputData& id ) {

   m_inputData.push_back( id );
   return;

}

void SCycleConfig::SetOutputDirectory( const TString& outDir ) {

   m_outputDirectory = outDir;
   return;

}

const TString& SCycleConfig::GetOutputDirectory() const {

   return m_outputDirectory;

}

void SCycleConfig::SetPostFix( const TString& postFix ) {

   m_postFix = postFix;
   return;

}

const TString& SCycleConfig::GetPostFix() const {

   return m_postFix;

}

void SCycleConfig::SetMsgLevel( SMsgType level ) {

   m_msgLevel = level;
   return;

}

SMsgType SCycleConfig::GetMsgLevel() const {

   return m_msgLevel;

}

void SCycleConfig::PrintConfig() const {

   SLogger logger( "SCycleConfig" );
   logger << INFO << "==========================================================="
          << SLogger::endmsg;
   logger << INFO << "                    Cycle configuration" << SLogger::endmsg;
   logger << INFO << "  - Running mode: "
          << ( m_mode == LOCAL ? "LOCAL" : "PROOF" ) << SLogger::endmsg;
   if( m_mode == PROOF ) {
      logger << INFO << "  - PROOF server: " << m_server << SLogger::endmsg;
   }
   logger << INFO << "  - Target luminosity: " << m_targetLumi << SLogger::endmsg;
   logger << INFO << "  - Output directory: " << m_outputDirectory << SLogger::endmsg;
   logger << INFO << "  - Post-fix: " << m_postFix << SLogger::endmsg;

   for( id_type::const_iterator id = m_inputData.begin(); id != m_inputData.end();
        ++id ) {
      id->print();
   }

   logger << INFO << "==========================================================="
          << SLogger::endmsg;

}

void SCycleConfig::ArrangeInputData() throw ( SError ) {

   // multimap to hold all type strings of InputData objects; will be
   // used to search InputData objects with the same name, to make
   // sure these will be processed directly consecutively so that they
   // end up in the same output file
   std::multimap< std::string, int > inputDataHelper;
   std::vector< SInputData >::iterator be = m_inputData.begin();
   std::vector< SInputData >::iterator en = m_inputData.end();
   int index = 0;

   // loop over InputData vector and copy the type names into the
   // multimap, remembering the position in the vector
   for( ; be != en; ++be,++index ) {
      inputDataHelper.insert( std::make_pair( be->GetType(), index ) );
   }

   std::multimap< std::string, int >::iterator help1 = inputDataHelper.begin();
   std::multimap< std::string, int >::iterator help2 = inputDataHelper.end();
   index = 0;
   std::vector< SInputData > tmpInput;

   // Now copy the InputData objects to a temporary vector in the
   // order we want them to be processed
   for( ; help1 != help2; ++help1,++index ) {
      if( help1->second != index ) {
         SLogger logger( "SCycleConfig" );
         logger << WARNING << "InputData of type \"" << help1->first
                << "\" was given as input number " << ( help1->second + 1 )
                << " but will be repositioned and instead processed as number "
                << ( index + 1 ) << SLogger::endmsg;         
      }
      tmpInput.push_back( m_inputData[ help1->second ] );
   }

   // Sanity check
   if( m_inputData.size() != tmpInput.size() ) {
      SError error( SError::StopExecution );
      error << "Inconsistent InputData vectors: size " << m_inputData.size()
            << " and " << tmpInput.size();
      throw error;
   }

   // Now copy the objects back into the vector we use for processing
   for( size_t i = 0; i < tmpInput.size(); ++i ) {
      m_inputData[ i ] = tmpInput[ i ];
   }

   return;

}

void SCycleConfig::ValidateInput() {

   for( id_type::iterator id = m_inputData.begin(); id != m_inputData.end(); ++id ) {
      id->ValidateInput();
   }

   return;

}

void SCycleConfig::ClearConfig() {

   m_mode = LOCAL;
   m_server = "";
   m_properties.clear();
   m_inputData.clear();
   m_targetLumi = 1.0;
   m_outputDirectory = "./";
   m_postFix = "";
   return;

}
