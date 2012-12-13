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
#include <ctime>
#include <locale>

// STL include(s):
#include <map>

// ROOT include(s):
#include <TSystem.h>

// Local include(s):
#include "../include/SCycleConfig.h"
#include "../include/SLogger.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleConfig )
#endif // DOXYGEN_IGNORE

/**
 * @param name The name of the object
 */
SCycleConfig::SCycleConfig( const char* name )
   : TNamed( name, "SFrame cycle configuration" ),
     m_cycleName( "Unknown" ), m_mode( LOCAL ),
     m_server( "" ), m_workdir( "" ), m_nodes( -1 ), m_properties(),
     m_inputData(), m_targetLumi( 1. ), m_outputDirectory( "" ),
     m_postFix( "" ), m_msgLevel( INFO ), m_useTreeCache( kFALSE ),
     m_cacheSize( 30000000 ), m_cacheLearnEntries( 100 ),
     m_processOnlyLocal( kFALSE ) {

}

/**
 * @returns The name of the cycle class
 */
const TString& SCycleConfig::GetCycleName() const {

   return m_cycleName;
}

/**
 * @param name The name of the cycle class
 */
void SCycleConfig::SetCycleName( const TString& name ) {

   m_cycleName = name;
   return;
}

/**
 * @returns The running mode of the cycle
 */
SCycleConfig::RunMode SCycleConfig::GetRunMode() const {

   return m_mode;
}

/**
 * @param mode The running mode of the cycle
 */
void SCycleConfig::SetRunMode( SCycleConfig::RunMode mode ) {

   m_mode = mode;
   return;
}

/**
 * @returns The name of the PROOF server to use in PROOF mode
 */
const TString& SCycleConfig::GetProofServer() const {

   return m_server;
}

/**
 * @param server The name of the PROOF server to use in PROOF mode
 */
void SCycleConfig::SetProofServer( const TString& server ) {

   m_server = server;
   return;
}

/**
 * @returns The number of PROOF nodes to use for the cycle
 */
Int_t SCycleConfig::GetProofNodes() const {

   return m_nodes;
}

/**
 * @param nodes The number of PROOF nodes to use for the cycle
 */
void SCycleConfig::SetProofNodes( Int_t nodes ) {

   m_nodes = nodes;
   return;
}

/**
 * @returns The directory to use for storing merged ntuples from PROOF
 */
const TString& SCycleConfig::GetProofWorkDir() const {

   return m_workdir;
}

/**
 * @param workdir The directory to use for storing merged ntuples from PROOF
 */
void SCycleConfig::SetProofWorkDir( const TString& workdir ) {

   m_workdir = workdir;
   return;
}

/**
 * @returns All the cycle properties
 */
const SCycleConfig::property_type& SCycleConfig::GetProperties() const {

   return m_properties;
}

/**
 * @param name  The name of the property to set
 * @param value The value of the property to set
 */
void SCycleConfig::SetProperty( const std::string& name,
                                const std::string& value ) {

   m_properties.push_back( std::make_pair( name, value ) );
   return;
}

/**
 * @returns All the input data blocks of the cycle
 */
const SCycleConfig::id_type& SCycleConfig::GetInputData() const {

   return m_inputData;
}

/**
 * @param id A new input data block for the cycle
 */
void SCycleConfig::AddInputData( const SInputData& id ) {

   m_inputData.push_back( id );
   return;
}

/**
 * @param outDir The directory name to put the output file(s) in
 */
void SCycleConfig::SetOutputDirectory( const TString& outDir ) {

   m_outputDirectory = outDir;
   return;
}

/**
 * @returns The directory name to put the output file(s) in
 */
const TString& SCycleConfig::GetOutputDirectory() const {

   return m_outputDirectory;
}

/**
 * @param postFix The post-fix to put at the end of the output file name(s)
 */
void SCycleConfig::SetPostFix( const TString& postFix ) {

   m_postFix = postFix;
   return;
}

/**
 * @returns The post-fix to put at the end of the output file name(s)
 */
const TString& SCycleConfig::GetPostFix() const {

   return m_postFix;
}

/**
 * @param level The minimum level of printed messages
 */
void SCycleConfig::SetMsgLevel( SMsgType level ) {

   m_msgLevel = level;
   return;
}

/**
 * @returns The minimum level of printed messages
 */
SMsgType SCycleConfig::GetMsgLevel() const {

   return m_msgLevel;
}

/**
 * @param status <code>kTRUE</code> if TTreeCache should be used,
 *               <code>kFALSE</code> if not
 */
void SCycleConfig::SetUseTreeCache( Bool_t status ) {

   m_useTreeCache = status;
   return;
}

/**
 * @returns <code>kTRUE</code> if TTreeCache should be used,
 *          <code>kFALSE</code> if not
 */
Bool_t SCycleConfig::GetUseTreeCache() const {

   return m_useTreeCache;
}

/**
 * @param size The size of the TTreeCache in bytes
 */
void SCycleConfig::SetCacheSize( Long64_t size ) {

   m_cacheSize = size;
   return;
}

/**
 * @returns The size of the TTreeCache in bytes
 */
Long64_t SCycleConfig::GetCacheSize() const {

   return m_cacheSize;
}

/**
 * @param entries The number of events used to learn input tree access pattern
 */
void SCycleConfig::SetCacheLearnEntries( Int_t entries ) {

   m_cacheLearnEntries = entries;
   return;
}

/**
 * @returns The number of events used to learn input tree access pattern
 */
Int_t SCycleConfig::GetCacheLearnEntries() const {

   return m_cacheLearnEntries;
}

/**
 * @param flag <code>kTRUE</code> if PROOF workers are only allowed to process
 *             files local to them, <code>kFALSE</code> if not
 */
void SCycleConfig::SetProcessOnlyLocal( Bool_t flag ) {

   m_processOnlyLocal = flag;
   return;
}

/**
 * @returns <code>kTRUE</code> if PROOF workers are only allowed to process
 *          files local to them, <code>kFALSE</code> if not
 */
Bool_t SCycleConfig::GetProcessOnlyLocal() const {

   return m_processOnlyLocal;
}

/**
 * This function is used at the initialization stage to print the configuration
 * of the cycle in a nice way.
 */
void SCycleConfig::PrintConfig() const {

   SLogger logger( "SCycleConfig" );
   logger << INFO
          << "==========================================================="
          << SLogger::endmsg;
   logger << INFO << "                    Cycle configuration"
          << SLogger::endmsg;
   logger << INFO << "  - Running mode: "
          << ( m_mode == LOCAL ? "LOCAL" : "PROOF" ) << SLogger::endmsg;
   if( m_mode == PROOF ) {
      logger << INFO << "  - PROOF server: " << m_server << SLogger::endmsg;
      logger << INFO << "  - PROOF nodes: " << m_nodes << SLogger::endmsg;
   }
   logger << INFO << "  - Target luminosity: " << m_targetLumi
          << SLogger::endmsg;
   logger << INFO << "  - Output directory: " << m_outputDirectory
          << SLogger::endmsg;
   logger << INFO << "  - Post-fix: " << m_postFix << SLogger::endmsg;
   if( m_useTreeCache ) {
      logger << INFO << "  - Using TTreeCache with size: " << m_cacheSize
             << SLogger::endmsg;
      if( m_cacheLearnEntries > 0 ) {
         logger << INFO << "                 learn entries: "
                << m_cacheLearnEntries << SLogger::endmsg;
      } else if( m_cacheLearnEntries < 0 ) {
         logger << INFO << "    All branches added to the cache"
                << SLogger::endmsg;
      } else {
         logger << INFO
                << "    The user is expected to choose branches to cache"
                << SLogger::endmsg;
      }
   }
   if( m_processOnlyLocal ) {
      logger << INFO << "  - Workers will only process local files"
             << SLogger::endmsg;
   }

   for( id_type::const_iterator id = m_inputData.begin();
        id != m_inputData.end(); ++id ) {
      id->Print();
   }

   logger << INFO
          << "==========================================================="
          << SLogger::endmsg;

   return;
}

/**
 * After the re-arranging the objects with the same type will end up
 * beside each other.
 */
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
      const size_t i = static_cast< size_t >( help1->second );
      tmpInput.push_back( m_inputData[ i ] );
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

/**
 * Some information about the input is gathered automatically from the
 * input files, and not from the XML configuration. This information is
 * needed for the correct event weight calculation. This function should
 * be called by SCycleController...
 */
void SCycleConfig::ValidateInput() {

   for( id_type::iterator id = m_inputData.begin(); id != m_inputData.end();
        ++id ) {
      id->ValidateInput( m_server );
   }

   return;
}

/**
 * This function makes a nice string representation of the cycle's
 * configuration. This makes it easy to archive the configuration into the
 * output file of the cycle.
 *
 * @param id If specified only the configuration of this one ID is added to the
 *           string. If it's left as a null pointer, all IDs are added.
 * @returns The string that should be put into the output file of the cycle
 *          for future reference
 */
TString SCycleConfig::GetStringConfig( const SInputData* id ) const {

   // The result string:
   TString result;

   // Get the system information:
   SysInfo_t sysInfo;
   gSystem->GetSysInfo( &sysInfo );

   // Get the user information:
   UserGroup_t* userInfo = gSystem->GetUserInfo();

   // Get the current time:
   time_t rawtime = time( NULL );
   const char* currentTime = ctime( &rawtime );
   TString printedTime = currentTime ? currentTime : "unknown";
   if( printedTime.EndsWith( "\n" ) ) {
      printedTime.Remove( printedTime.Length() - 1 );
   }

   // Interestingly enough, it's not needed to delete the return value of
   // ctime(...). It even causes problems if I try to do it. Even though
   // it actually returns "char*" and not "const char*". Very strange...

   // Some disclaimer:
   result += "<!-- Archived cycle configuration -->\n";
   result += TString::Format( "<!--  host: %s -->\n", gSystem->HostName() );
   result += TString::Format( "<!--  syst: %s / %s -->\n", sysInfo.fOS.Data(),
                              sysInfo.fModel.Data() );
   result += TString::Format( "<!--  user: %s (%s) -->\n",
                              ( userInfo ? userInfo->fUser.Data() :
                                "unknown" ),
                              ( userInfo ? userInfo->fRealName.Data() :
                                "N/A" ) );
   result += TString::Format( "<!--  time: %s -->\n\n", printedTime.Data() );

   // Delete the created object(s):
   if( userInfo ) delete userInfo;

   // Put together the <Cycle...> part of the configuration:
   result += TString::Format( "<Cycle Name=\"%s\"\n", m_cycleName.Data() );
   result += TString::Format( "       OutputDirectory=\"%s\"\n",
                              m_outputDirectory.Data() );
   result += TString::Format( "       PostFix=\"%s\"\n",
                              m_postFix.Data() );
   result += TString::Format( "       TargetLumi=\"%g\"\n", m_targetLumi );
   result += "       RunMode=\"";
   if( m_mode == LOCAL ) {
      result += "LOCAL";
   } else if( m_mode == PROOF ) {
      result += "PROOF";
   } else {
      result += "UNKNOWN";
   }
   result += "\"\n";
   result += TString::Format( "       ProofServer=\"%s\"\n",
                              m_server.Data() );
   result += TString::Format( "       ProofNodes=\"%i\"\n", m_nodes );
   result += TString::Format( "       ProofWorkDir=\"%s\"\n",
                              m_workdir.Data() );
   result += TString::Format( "       UseTreeCache=\"%s\"\n",
                              ( m_useTreeCache ? "True" : "False" ) );
   result += TString::Format( "       TreeCacheSize=\"%lld\"\n", m_cacheSize );
   result += TString::Format( "       TreeCacheLearnEntries=\"%i\"\n",
                              m_cacheLearnEntries );
   result += TString::Format( "       ProcessOnlyLocal=\"%s\">\n\n",
                              ( m_processOnlyLocal ? "True" : "False" ) );

   // Decide how to add the input data information:
   if( id ) {
      // Add just this one SInputData to the output:
      result += TString::Format( "%s\n\n", id->GetStringConfig().Data() );
   } else {
      // Put all the InputData options in there:
      id_type::const_iterator i_itr = m_inputData.begin();
      id_type::const_iterator i_end = m_inputData.end();
      for( ; i_itr != i_end; ++i_itr ) {
         result += TString::Format( "%s\n\n", i_itr->GetStringConfig().Data() );
      }
   }

   // Put all the user configuration options in there:
   result += "    <UserConfig>\n";
   property_type::const_iterator p_itr = m_properties.begin();
   property_type::const_iterator p_end = m_properties.end();
   for( ; p_itr != p_end; ++p_itr ) {
      result += TString::Format( "       <Item Name=\"%s\" Value=\"%s\"/>\n",
                                 p_itr->first.c_str(),
                                 p_itr->second.c_str() );
   }
   result += "    </UserConfig>\n";

   // Close the <Cycle> block:
   result += "</Cycle>";

   // Return the string that we just made:
   return result;
}

/**
 * The function just resets all configuration options in the object to
 * meaningful defaults.
 */
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
