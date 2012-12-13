// Dear emacs, this is -*- c++ -*-
// $Id$
/***************************************************************************
 * @Project: SFrame - ROOT-based analysis framework for ATLAS
 * @Package: Plug-ins
 *
 * @author Stefan Ask       <Stefan.Ask@cern.ch>           - Manchester
 * @author David Berge      <David.Berge@cern.ch>          - CERN
 * @author Johannes Haller  <Johannes.Haller@cern.ch>      - Hamburg
 * @author A. Krasznahorkay <Attila.Krasznahorkay@cern.ch> - NYU/Debrecen
 *
 ***************************************************************************/

#ifndef SFRAME_PLUGINS_SToolBase_H
#define SFRAME_PLUGINS_SToolBase_H

// SFrame include(s):
#include "core/include/SLogger.h"
#include "core/include/SCycleBase.h"

// Forward declaration(s):
class TH1;
class TObject;
class TBranch;

/**
 *   @short Base class for tools that can be used during the analysis
 *
 *          The idea is that people will probably want to make their analysis
 *          code modular by breaking it into many classes. To make it easy
 *          to do common "SFrame tasks" in these classes (which are not cycles
 *          themselves), one can use this base class. It provides much of the
 *          same convenience functionality that SCycleBase does.
 *
 * @version $Revision$
 */
template< class Type >
class SToolBaseT {

   /// To enable the instantiation of SToolBaseT< SToolBase > tools
   friend class SToolBaseT< SToolBaseT< SCycleBase > >;

public:
   /// Declaration of the used type
   typedef Type ParentType;

   /// Defaul Constructor 
   SToolBaseT();
   /// Constructor specifying the parent of the tool
   SToolBaseT( ParentType* parent );
   /// Destructor (needed to make vtable happy)
   virtual ~SToolBaseT() {}

   /// Get a pointer to the parent cycle of this tool
   ParentType* GetParent() const;
   /// Get a pointer to the parent cycle of this tool
   void SetParent( ParentType* parent );

protected:
   /// @name Function inherited from SCycleBaseHist
   //@{
   /// Function placing a ROOT object in the output file
   template< class T > T* Book( const T& histo,
                                const char* directory = 0,
                                Bool_t inFile = kFALSE ) throw( SError );
   /// Function searching for a ROOT object in the output file
   template< class T > T*
   Retrieve( const char* name, const char* directory = 0,
             Bool_t outputOnly = kFALSE ) throw( SError );
   /// Function retrieving all ROOT objects of this name from the input file
   template< class T >
   std::vector< T* > RetrieveAll( const char* name,
                                  const char* directory = 0 ) throw( SError );
   /// Function for persistifying a ROOT object to the output
   void WriteObj( const TObject& obj,
                  const char* directory = 0 ) throw( SError );
   /// Function searching for 1-dimensional histograms in the output file
   TH1* Hist( const char* name, const char* dir = 0 );
   //@}

public:
   /// @name Functions inherited from SCycleBaseNTuple
   //@{
   /// Connect an input variable
   template< typename T >
   bool ConnectVariable( const char* treeName, const char* branchName,
                         T& variable ) throw ( SError );
   /// Declare an output variable
   template< typename T >
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
   //@}

protected:
   /// @name Functions inherited from SCycleBaseConfig
   //@{
   /// Declare a property
   template< typename T >
   void DeclareProperty( const std::string& name, T& value );
   /// Add a configuration object that should be available on the PROOF nodes
   void AddConfigObject( TObject* object );
   /// Get a configuration object on the PROOF nodes
   TObject* GetConfigObject( const char* name ) const;
   //@}

   /// Set the name under which the tool's log messages should appear
   void SetLogName( const char* name );

   mutable SLogger m_logger; ///< Logger object for the tool

private:
   ParentType* m_parent; ///< Pointer to the parent cycle of this tool

}; // class SToolBaseT

// Include the template implementation:
#ifndef __CINT__
#include "SToolBase.icc"
#endif // __CINT__

/// This typedef is for backward compatibility with the old implementation
/**
 * Since people probably have some code already lying around which uses tools
 * inheriting from SToolBase, this typedef takes care of all that code still
 * compiling.
 *
 * However I don't intend to add more typedefs like this, for new types of
 * tools (for instance tools that have another tool as parent) people should
 * create the template specialisations themselves.
 */
typedef SToolBaseT< SCycleBase > SToolBase;

#endif // SFRAME_PLUGINS_SToolBase_H
