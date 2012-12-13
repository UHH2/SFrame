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

#ifndef SFRAME_CORE_SCycleOutput_H
#define SFRAME_CORE_SCycleOutput_H

// ROOT include(s):
#include <TNamed.h>
#include <TString.h>

// Local include(s):
#include "SError.h"
#include "SLogger.h"

// Forward declaration(s):
class TCollection;
class TDirectory;

/**
 *   @short Special object type for the outputs produced by a cycle
 *
 *          In order to specify an output directory for the objects
 *          produced in a cycle, the objects can't be put directly into
 *          the output list of TSelector. Instead I wrap them into such
 *          an object.
 *
 *          The trickiest part of this class is the Merge function that
 *          takes care of summing up the results obtained from the
 *          individual worder nodes.
 *
 * @version $Revision$
 */
class SCycleOutput : public TNamed {

public:
   /// Constructor with child object and name
   SCycleOutput( TObject* object = 0, const char* name = "",
                 const char* path = "" );
   /// Destructor
   virtual ~SCycleOutput();

   /// Get the wrapped object
   TObject* GetObject() const;
   /// Set the pointer to the wrapped object
   void SetObject( TObject* object );

   /// Get the output path of the wrapped object
   const TString& GetPath() const;
   /// Set the output path of the wrapped object
   void SetPath( const TString& path );

   /// Merge the contents of other objects into this one
   Int_t Merge( TCollection* list );
   /// Write the wrapped object in the correct output directory (const version)
   virtual Int_t Write( const char* name = 0, Int_t option = 0,
                        Int_t bufsize = 0 ) const;
   /// Write the wrapped object in the correct output directory (non-const version)
   virtual Int_t Write( const char* name = 0, Int_t option = 0,
                        Int_t bufsize = 0 );

private:
   /// Return the requested output directory
   TDirectory* MakeDirectory( const TString& path ) const throw( SError );

   /// The object that this class wraps
   TObject* m_object;
   /// Path of the object in the output file
   TString m_path;

   /// Transient logger object
   mutable SLogger m_logger; //!

#ifndef DOXYGEN_IGNORE
   ClassDef( SCycleOutput, 1 )
#endif // DOXYGEN_IGNORE

}; // class SCycleOutput

#endif // SFRAME_CORE_SCycleOutput_H
