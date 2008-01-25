// Dear emacs, this is -*- c++ -*-
// $Id: ISCycleBaseConfig.h,v 1.1 2008-01-25 14:33:53 krasznaa Exp $
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

#ifndef SFRAME_CORE_ISCycleBaseConfig_H
#define SFRAME_CORE_ISCycleBaseConfig_H

// STL include(s):
#include <vector>

// Local include(s):
#include "SError.h"
#include "SInputData.h"

// Forward declaration(s):
class TXMLNode;
class TString;

/**
 *   @short Interface class providing the XML configurability of the cycle
 *
 *          The SCycleBase class is broken into multiple classes. Some of
 *          these constituents can work independently, not knowing what the
 *          other constituents can do. But some parts (like SCycleBaseExec)
 *          rely on other constituents as well. To make those parts as modular
 *          as possible, they don't rely directly on the concrete implementations
 *          of the other constituents, but on interfaces like this.
 *
 *          This interface provides all the configuration functions that the
 *          framework needs from an analysis cycle.
 *
 * @version $Revision: 1.1 $
 */
class ISCycleBaseConfig {

public:
   virtual ~ISCycleBaseConfig() {}

   /// Function initialising the cycle
   /**
    * This function is called by the framework to set up the user
    * cycle based on the contents of the XML file.
    */
   virtual void Initialize( TXMLNode* ) throw( SError ) = 0;

   /// Set the output directory
   /**
    * It is specified in the XML file where the output ROOT
    * files should be placed. But since this configuration is
    * not accessible to the Initialize method directly, it
    * has to be set by SCycleController using this function.
    *
    * @see SCycleBaseConfig::GetOutputDirectory
    */
   virtual void SetOutputDirectory( const TString& outputdirectory ) = 0;
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
   virtual void SetPostFix( const TString& postfix ) = 0;
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
   virtual void SetTargetLumi( Double_t targetlumi ) = 0;

   /// Get the name of the output directory
   /**
    * @see SCycleBaseConfig::SetOutputDirectory
    */
   virtual const TString& GetOutputDirectory() const  = 0;
   /// Get the output file post-fix
   /**
    * @see SCycleBaseConfig::SetPostFix
    */
   virtual const TString& GetPostFix() const = 0;
   /// Get the target normalisation luminosity
   /**
    * @see SCycleBaseConfig::SetTargetLumi
    */
   virtual Double_t GetTargetLumi() const = 0;

protected:
   /// List of input data to run over
   /**
    * Much of the configuration from the XML is put into this member
    * variable. It needs to be available to basically all parts of the
    * cycle, so it ended up in the interface class.
    */
   std::vector< SInputData > m_inputData;

}; // class ISCycleBaseConfig

#endif // SFRAME_CORE_ISCycleBaseConfig_H
