// $Id$

// STL include(s):
#include <map>

// Local include(s):
#include "../include/SCycleConfig.h"
#include "../include/SLogger.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleConfig )
#endif // DOXYGEN_IGNORE

SCycleConfig::SCycleConfig( const char* name )
   : TNamed( name, "SFrame cycle configuration" ), m_mode( LOCAL ),
     m_server( "" ), m_workdir( "" ), m_nodes( -1 ), m_properties(),
     m_inputData(), m_targetLumi( 1. ), m_outputDirectory( "" ),
     m_postFix( "" ), m_msgLevel( INFO ), m_useTreeCache( kFALSE ),
     m_cacheSize( 30000000 ), m_cacheLearnEntries( 100 ),
     m_processOnlyLocal( kFALSE ) {

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

const Int_t& SCycleConfig::GetProofNodes() const {

   return m_nodes;
}

void SCycleConfig::SetProofNodes( const Int_t nodes ) {

   m_nodes = nodes;
   return;
}

const TString& SCycleConfig::GetProofWorkDir() const {

   return m_workdir;
}

void SCycleConfig::SetProofWorkDir( const TString& workdir ) {

   m_workdir = workdir;
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

void SCycleConfig::SetUseTreeCache( Bool_t status ) {

   m_useTreeCache = status;
   return;
}

Bool_t SCycleConfig::GetUseTreeCache() const {

   return m_useTreeCache;
}

void SCycleConfig::SetCacheSize( Long64_t size ) {

   m_cacheSize = size;
   return;
}

Long64_t SCycleConfig::GetCacheSize() const {

   return m_cacheSize;
}

void SCycleConfig::SetCacheLearnEntries( Int_t entries ) {

   m_cacheLearnEntries = entries;
   return;
}

Int_t SCycleConfig::GetCacheLearnEntries() const {

   return m_cacheLearnEntries;
}

void SCycleConfig::SetProcessOnlyLocal( Bool_t flag ) {

   m_processOnlyLocal = flag;
   return;
}

Bool_t SCycleConfig::GetProcessOnlyLocal() const {

   return m_processOnlyLocal;
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
      logger << INFO << "  - PROOF nodes: " << m_nodes << SLogger::endmsg;
   }
   logger << INFO << "  - Target luminosity: " << m_targetLumi << SLogger::endmsg;
   logger << INFO << "  - Output directory: " << m_outputDirectory << SLogger::endmsg;
   logger << INFO << "  - Post-fix: " << m_postFix << SLogger::endmsg;
   if( m_useTreeCache ) {
      logger << INFO << "  - Using TTreeCache with size: " << m_cacheSize << SLogger::endmsg;
      if( m_cacheLearnEntries > 0 ) {
         logger << INFO << "                 learn entries: " << m_cacheLearnEntries
                << SLogger::endmsg;
      } else if( m_cacheLearnEntries < 0 ) {
         logger << INFO << "    All branches added to the cache" << SLogger::endmsg;
      } else {
         logger << INFO << "    The user is expected to choose branches to cache"
                << SLogger::endmsg;
      }
   }
   if( m_processOnlyLocal ) {
      logger << INFO << "  - Workers will only process local files"
             << SLogger::endmsg;
   }

   for( id_type::const_iterator id = m_inputData.begin(); id != m_inputData.end();
        ++id ) {
      id->Print();
   }

   logger << INFO << "==========================================================="
          << SLogger::endmsg;

   return;
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
      id->ValidateInput( m_server );
   }

   return;
}

void SCycleConfig::ClearConfig() {

   m_mode = LOCAL;
   m_server = "";
   m_workdir = "";
   m_nodes = -1;
   m_properties.clear();
   m_inputData.clear();
   m_targetLumi = 1.0;
   m_outputDirectory = "./";
   m_postFix = "";
   m_msgLevel = INFO;
   m_useTreeCache = kFALSE;
   m_cacheSize = 30000000;
   m_cacheLearnEntries = 100;

   return;
}
