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

#ifndef SFRAME_CORE_SCycleBaseNTuple_H
#define SFRAME_CORE_SCycleBaseNTuple_H

// STL include(s):
#include <vector>
#include <map>
#include <string>
#include <list>

// Local include(s):
#include "ISCycleBaseConfig.h"
#include "ISCycleBaseNTuple.h"
#include "SCycleBaseBase.h"
#include "SError.h"

// Forward declaration(s):
class TTree;
class TFile;
class TBranch;
class SInputData;
class SCycleBase;
template< class T > class SToolBaseT;
template< class T > class SInputVariables;
template< class T > class SOutputVariables;

/**
 *   @short NTuple handling part of SCycleBase
 *
 *          This is the most complex constituent of all the
 *          SCycleBase classes. It is responsible for handling
 *          input and output TTree-s. It has quite a number of
 *          protected functions which are used by SCycleBase, and
 *          are hidden from the user by that class. (A little
 *          C++ magic...)
 *
 * @version $Revision$
 */
class SCycleBaseNTuple : public virtual ISCycleBaseConfig,
                         public virtual ISCycleBaseNTuple,
                         public virtual SCycleBaseBase {

   /// To enable the usage of the protected functions for SToolBase
   friend class SToolBaseT< SCycleBase >;
   /// To enable the usage of the protected functions for SInputVariables
   friend class SInputVariables< SCycleBaseNTuple >;
   /// To enable the usage of the protected functions for SOutputVariables
   friend class SOutputVariables< SCycleBaseNTuple >;

public:
   /// Default constructor
   SCycleBaseNTuple();
   /// Default destructor
   virtual ~SCycleBaseNTuple();

   /// Set the object list used for NTuple output
   virtual void SetNTupleOutput( TList* output );
   /// Get the object list used for NTuple output
   virtual TList* GetNTupleOutput() const;

protected:
   /// Connect an input variable
   template< typename T >
   bool ConnectVariable( const char* treeName, const char* branchName,
                         T& variable ) throw ( SError );
   /// Specialisation for primitive arrays
   template< typename T, size_t size >
   bool ConnectVariable( const char* treeName, const char* branchName,
                         T ( &variable )[ size ] ) throw( SError );
   /// Specialisation for object pointers
   template< typename T >
   bool ConnectVariable( const char* treeName, const char* branchName,
                         T*& variable ) throw( SError );

   /// Declare an output variable
   template< class T >
   TBranch* DeclareVariable( T& obj, const char* name,
                             const char* treeName = 0 ) throw( SError );

   /// Access one of the metadata trees
   virtual TTree* GetMetadataTree( const char* name ) const throw( SError );
   /// Access one of the input trees
   virtual TTree* GetInputTree( const char* treeName ) const throw( SError );
   /// Access one of the output trees
   virtual TTree* GetOutputTree( const char* treeName ) const throw( SError );

   //////////////////////////////////////////////////////////
   //                                                      //
   //          Functions called by the framework:          //
   //                                                      //
   //////////////////////////////////////////////////////////

   /// Create the output trees
   void CreateOutputTrees( const SInputData& id,
                           std::vector< TTree* >& outTrees,
                           TFile* outputFile = 0 ) throw( SError );
   /// Save all the created output trees in the output
   void SaveOutputTrees( TDirectory* output ) throw( SError );
   /// Load the input trees
   void LoadInputTrees( const SInputData& id, TTree* main_tree ) throw( SError );
   /// Declare that the cache for the input trees is now configured
   void SetInputCacheConfigured() throw( SError );
   /// Read in the event from the "normal" trees
   void GetEvent( Long64_t entry ) throw( SError );
   /// Calculate the weight of the current event
   Double_t CalculateWeight( const SInputData& inputData, Long64_t entry );

private:
   static const char* RootType( const char* typeid_type ) throw( SError );
   static const char* TypeidType( const char* root_type ) throw( SError );
   void RegisterInputBranch( TBranch* br ) throw( SError );
   void DeleteInputVariables();

   //
   // These are the objects used to handle the input and output data:
   //
   std::vector< TTree* >   m_inputTrees; // List of input TTree pointers
   std::vector< TBranch* > m_inputBranches; // vector of input branch pointers
                                            // registered for the current cycle
   std::list< TObject* >   m_inputVarPointers; // Pointers storing the input objects
                                               // created by ConnectVariable(...)

   /// Vector to hold the output trees
   std::vector< TTree* > m_outputTrees;
   /// Vector to hold the metadata input trees
   std::vector< TTree* > m_metaInputTrees;
   /// Vector to hold the metadata output trees
   std::vector< TTree* > m_metaOutputTrees;

   // We have to keep the pointers to the output variables defined by the user.
   // ROOT keeps track of the objects by storing pointers to pointers to the
   // objects. Since the user probably wants to use the output objects directly
   // and not through pointers, the base class has to take care of this
   // pointer issue by itself...
   std::list< void* > m_outputVarPointers;

   TList* m_output;

}; // class SCycleBaseNTuple

// Don't include the templated function(s) when we're generating
// a dictionary:
#ifndef __CINT__
#include "SCycleBaseNTuple.icc"
#endif

#endif // SFRAME_CORE_SCycleBaseNTuple_H
