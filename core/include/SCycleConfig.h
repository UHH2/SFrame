// Dear emacs, this is -*- c++ -*-
// $Id: SCycleConfig.h,v 1.1.2.1 2008-12-01 14:52:56 krasznaa Exp $

#ifndef SFRAME_CORE_SCycleConfig_H
#define SFRAME_CORE_SCycleConfig_H

// STL include(s):
#include <vector>
#include <bits/stl_pair.h>

// ROOT include(s):
#include <TNamed.h>
#include <TString.h>

// Local include(s):
#include "SInputData.h"
#include "SError.h"

class SCycleConfig : public TNamed {

public:
   SCycleConfig( const char* name = "SCycleConfig" );
   ~SCycleConfig();

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
   typedef std::vector< std::pair< std::string, std::string > > property_type;
   typedef std::vector< SInputData > id_type;

   RunMode GetRunMode() const;
   void SetRunMode( RunMode mode );

   const TString& GetProofServer() const;
   void SetProofServer( const TString& server );

   const property_type& GetProperties() const;
   void SetProperty( const std::string& name, const std::string& value );

   const id_type& GetInputData() const;
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

   void PrintConfig() const;
   void ArrangeInputData() throw ( SError );

   void ClearConfig();

private:
   RunMode       m_mode;
   TString       m_server;
   property_type m_properties;
   id_type       m_inputData;
   Double_t      m_targetLumi;

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleConfig, 1 );
#endif // DOXYGEN_IGNORE

}; // class SCycleConfig

#endif // SFRAME_CORE_SCycleConfig_H
