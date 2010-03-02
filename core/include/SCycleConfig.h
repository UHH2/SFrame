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

#ifndef SFRAME_CORE_SCycleConfig_H
#define SFRAME_CORE_SCycleConfig_H

// STL include(s):
#include <vector>
//#include <bits/stl_pair.h> // I have to include <map> here, because rootcint can't
#include <map>               // understand <bits/stl_pair.h> in Athena for some reason...

// ROOT include(s):
#include <TNamed.h>
#include <TString.h>

// Local include(s):
#include "SInputData.h"
#include "SError.h"
#include "SMsgType.h"

/**
 *   @short Class describing the entire configuration of a cycle
 *
 *          In a PROOF analysis the analysis cycles exist in multiple
 *          instances. To make their (coherent) configuration easier, they
 *          all get their setup from a single object of this type.
 *          SCycleController makes sure that the configuration is passed
 *          correctly to all cycle instances.
 *
 * @version $Revision$
 */
class SCycleConfig : public TNamed {

public:
   /// Simple constructor with a name
   SCycleConfig( const char* name = "SCycleConfig" );

   /// Run mode enumeration
   /**
    * This enumeration defines how the analysis cycle can be run. At the
    * moment local running and running the cycle on a PROOF cluster are
    * possible.
    */
   enum RunMode {
      LOCAL, ///< Run the analysis cycle locally
      PROOF  ///< Run the analysis cycle on a PROOF cluster
   };
   /// Definition of the type of the properties
   typedef std::vector< std::pair< std::string, std::string > > property_type;
   /// Definition of the type of the input data
   typedef std::vector< SInputData > id_type;

   /// Get the configured running mode
   RunMode GetRunMode() const;
   /// Set the configured running mode
   void SetRunMode( RunMode mode );

   /// Get the name of the PROOF server
   const TString& GetProofServer() const;
   /// Set the name of the PROOF server
   void SetProofServer( const TString& server );

   /// Get the number of parallel nodes
   const Int_t& GetProofNodes() const;
   /// Set the number of parallel nodes
   void SetProofNodes( const Int_t nodes );

   /// Get the path to the PROOF working directory
   const TString& GetProofWorkDir() const;
   /// Set the path to the PROOF working directory
   void SetProofWorkDir( const TString& workdir );

   /// Get the user defined properties
   const property_type& GetProperties() const;
   /// Set one user defined property
   void SetProperty( const std::string& name, const std::string& value );

   /// Get all input data objects
   const id_type& GetInputData() const;
   /// Add one input data object
   void AddInputData( const SInputData& id );

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
   void SetTargetLumi( Double_t targetlumi ) { m_targetLumi = targetlumi; }
   /// Get the target normalisation luminosity
   /**
    * @see SCycleBaseConfig::SetTargetLumi
    */
   Double_t GetTargetLumi() const { return m_targetLumi; }

   /// Set the directory where the output file will be stored
   void SetOutputDirectory( const TString& outDir );
   /// Get the directory where the output file will be stored
   const TString& GetOutputDirectory() const;

   /// Set the post-fix that should be added to the output file name
   void SetPostFix( const TString& postFix );
   /// Get the post-fix that should be added to the output file name
   const TString& GetPostFix() const;

   /// Set the minimum printed message level
   void SetMsgLevel( SMsgType level );
   /// Get the minimum printed message level
   SMsgType GetMsgLevel() const;

   /// Print the configuration to the screen
   void PrintConfig() const;
   /// Re-arrange the input data objects
   /**
    * After the re-arranging the objects with the same type will end up
    * beside each other.
    */
   void ArrangeInputData() throw ( SError );
   /// Fill the input data objects with information from the files
   /**
    * Some information about the input is gathered automatically from the
    * input files, and not from the XML configuration. This information is
    * needed for the correct event weight calculation. This function should
    * be called by SCycleController...
    */
   void ValidateInput();

   /// Clear the configuration
   void ClearConfig();

private:
   RunMode       m_mode;
   TString       m_server;
   TString       m_workdir;
   Int_t         m_nodes;
   property_type m_properties;
   id_type       m_inputData;
   Double_t      m_targetLumi;
   TString       m_outputDirectory;
   TString       m_postFix;
   SMsgType      m_msgLevel;

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleConfig, 1 );
#endif // DOXYGEN_IGNORE

}; // class SCycleConfig

#endif // SFRAME_CORE_SCycleConfig_H
