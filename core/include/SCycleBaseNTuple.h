// Dear emacs, this is -*- c++ -*-
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

public:
   /// Default constructor
   SCycleBaseNTuple();
   /// Default destructor
   virtual ~SCycleBaseNTuple();

   /// Set the object list used for NTuple output
   virtual void SetNTupleOutput( TList* output );
   /// Get the object list used for NTuple output
   virtual TList* GetNTupleOutput() const;
   /// Set the object list used for NTuple input
   virtual void SetNTupleInput( TList* input );
   /// Get the object list used for NTuple input
   virtual TList* GetNTupleInput() const;

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
   /// Access one of the input metadata trees
   virtual TTree*
   GetInputMetadataTree( const char* name ) const throw( SError );
   /// Access one of the output metadata trees
   virtual TTree*
   GetOutputMetadataTree( const char* name ) const throw( SError );
   /// Access one of the input trees
   virtual TTree* GetInputTree( const char* treeName ) const throw( SError );
   /// Access one of the output trees
   virtual TTree* GetOutputTree( const char* treeName ) const throw( SError );

protected:
   //////////////////////////////////////////////////////////
   //                                                      //
   //          Functions called by the framework:          //
   //                                                      //
   //////////////////////////////////////////////////////////

   /// Function creating an output file on demand
   virtual TDirectory* GetOutputFile() throw( SError );
   /// Function closing a potentially open output file
   virtual void CloseOutputFile() throw( SError );
   /// Create the output trees
   void CreateOutputTrees( const SInputData& id,
                           std::vector< TTree* >& outTrees ) throw( SError );
   /// Save all the created output trees in the output
   void SaveOutputTrees() throw( SError );
   /// Load the input trees
   void LoadInputTrees( const SInputData& id, TTree* main_tree,
                        TDirectory*& inputFile ) throw( SError );
   /// Read in the event from the "normal" trees
   void GetEvent( Long64_t entry ) throw( SError );
   /// Calculate the weight of the current event
   Double_t CalculateWeight( const SInputData& inputData,
                             Long64_t entry ) const;
   /// Forget about the internally cached TTree pointers
   void ClearCachedTrees();

private:
   /// Function translating a "typeid type" into a ROOT type character
   static const char* RootType( const char* typeid_type ) throw( SError );
   /// Function translating a ROOT type character into a "typeid type"
   static const char* TypeidType( const char* root_type ) throw( SError );
   /// Function registering an input branch for use during the event loop
   void RegisterInputBranch( TBranch* br ) throw( SError );
   /// Function deleting the object created on the heap by ROOT
   void DeleteInputVariables();
   /// Function creating a sub-directory inside an existing directory
   TDirectory* MakeSubDirectory( const TString& path,
                                 TDirectory* dir ) const throw( SError );

   //
   // These are the objects used to handle the input and output data:
   //
   /// List of input TTree pointers
   std::vector< TTree* >   m_inputTrees;
   /// Vector of input branch pointers registered for the current cycle
   std::vector< TBranch* > m_inputBranches;
   /// Pointers storing the input objects created by ConnectVariable(...)
   std::list< TObject* >   m_inputVarPointers;

   TFile* m_outputFile; ///< Pointer to the active temporary output file

   /// Vector to hold the output trees
   std::vector< TTree* > m_outputTrees;
   /// Vector to hold the metadata input trees
   std::vector< TTree* > m_metaInputTrees;
   /// Vector to hold the metadata output trees
   std::vector< TTree* > m_metaOutputTrees;

   /// Output object pointers
   /**
    * We have to keep the pointers to the output variables defined by the user.
    * ROOT keeps track of the objects by storing pointers to pointers to the
    * objects. Since the user probably wants to use the output objects directly
    * and not through pointers, the base class has to take care of this
    * pointer issue by itself...
    */
   std::list< void* > m_outputVarPointers;

   TList* m_input; ///< Pointer to the input object list
   TList* m_output; ///< Pointer to the output object list

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleBaseNTuple, 0 )
#endif // DOXYGEN_IGNORE

}; // class SCycleBaseNTuple

// Don't include the templated function(s) when we're generating
// a dictionary:
#ifndef __CINT__
#include "SCycleBaseNTuple.icc"
#endif

#endif // SFRAME_CORE_SCycleBaseNTuple_H
