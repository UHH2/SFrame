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

#ifndef SFRAME_PLUGINS_SSummedVar_H
#define SFRAME_PLUGINS_SSummedVar_H

// STL include(s):
#include <map>

// ROOT include(s):
#include <TNamed.h>
#include <TString.h>

// SFrame include(s):
#include "core/include/SError.h"

// Forward declaration(s):
class TCollection;
class ISCycleBaseHist;

/**
 *   @short Class used in the SSummedVar implementation
 *
 *          This class should not be too useful for the users, it's meant to
 *          be used internally by SSummedVar. Unfortunately ROOT goes crazy
 *          if I want to declare a templated class inside another templated
 *          class... Remember that we need to compile working dictionaries
 *          for at least this class, which rootcint can't currently do if it's
 *          a private member of SSummedVar.
 *
 * @version $Revision$
 */
template< class Type >
class ProofSummedVar : public TNamed {

public:
   /// Default constructor
   ProofSummedVar( const char* name = 0, const char* title = 0 );
   /// Function merging the results from the worker nodes
   virtual Int_t Merge( TCollection* coll );
   /// The wrapped variable
   Type m_member;

#ifndef DOXYGEN_IGNORE
   ClassDef( ProofSummedVar, 1 )
#endif // DOXYGEN_IGNORE

}; // class ProofSummedVar

/**
 *   @short Class to be used when counting something on PROOF
 *
 *          This class should make it quite simple to count various things
 *          when executing an analysis on a PROOF cluster. After instantiating
 *          it, one just has to use it like the variable that it wrapps.
 *          The inner workings of the class make sure that the values assigned
 *          to it on the worker nodes are added up and are visible on the
 *          master node.
 *
 *          Note that when you add such a variable as a member to your cycle,
 *          you should hide the variable from the dictionary generation in
 *          the newer ROOT releases. (5.28 and the development releases leading
 *          up to it.) This is done the easiest by putting "//!" after the
 *          variable declaration. See the FirstCycle example.
 *
 * @version $Revision$
 */
template< class Type >
class SSummedVar {

public:
   /// Declaration of the used type
   typedef Type ValueType;

   /// Constructor
   SSummedVar( const char* name, ISCycleBaseHist* parent );

   /// Automatic conversion operator
   operator Type&();
   /// Constant automatic conversion operator
   operator const Type&() const;

   /// Operator for accessing the wrapped object as a reference
   Type& operator*();
   /// Operator for accessing the wrapped object as a pointer
   Type* operator->();

   /// Function for accessing the wrapped object as a reference
   Type& GetReference();
   /// Function for accessing the wrapped object as a pointer
   Type* GetPointer();

   /// Constant operator for accessing the wrapped object as a reference
   const Type& operator*() const;
   /// Constant operator for accessing the wrapped object as a pointer
   const Type* operator->() const;

   /// Constant function for accessing the wrapped object as a reference
   const Type& GetReference() const;
   /// Constant function for accessing the wrapped object as a pointer
   const Type* GetPointer() const;

private:
   /// Function for accessing the internal object
   ProofSummedVar< Type >* GetObject() const throw( SError );

   TString                         m_objName; ///< Name of the object
   ISCycleBaseHist*                m_parent; ///< Pointer to the parent cycle
   mutable ProofSummedVar< Type >* m_object; ///< Cached pointer

}; // class SSummedVar

//
// Include template implementation:
//
#ifndef __CINT__
#include "SSummedVar.icc"
#endif // __CINT__

#endif // SFRAME_PLUGINS_SSummedVar_H
