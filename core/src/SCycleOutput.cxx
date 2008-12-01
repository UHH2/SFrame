// $Id: SCycleOutput.cxx,v 1.1.2.1 2008-12-01 14:52:57 krasznaa Exp $

// System include(s):
#include <string.h>

// STL include(s):
#include <vector>

// ROOT include(s):
#include <TCollection.h>
#include <TList.h>
#include <TMethodCall.h>
#include <TDirectory.h>
#include <TTree.h>

// Local include(s):
#include "../include/SCycleOutput.h"
#include "../include/SLogger.h"

#ifndef DOXYGEN_IGNORE
ClassImp( SCycleOutput );
#endif // DOXYGEN_IGNORE

/**
 * The constructor just initialises the base class and the member(s)
 * to meaningul defauls.
 *
 * @param object Pointer to the object that should be wrapped
 * @param name Name of this object. The SCycleBase* classes should
 *             make sure that this is unique
 */
SCycleOutput::SCycleOutput( TObject* object, const char* name,
                            const char* path )
   : TNamed( name, "SFrame cycle output object" ), m_object( object ),
     m_path( path ), m_logger( "SCycleOutput" ) {

}

/**
 * Since this wrapper class owns the object it wraps, it deletes it
 * when it is deleted itself.
 */
SCycleOutput::~SCycleOutput() {

   if( m_object ) delete m_object;

}

/**
 * @returns A pointer to the wrapped object
 */
TObject* SCycleOutput::GetObject() const {

   return m_object;

}

/**
 * @param object Pointer to the wrapped object
 */
void SCycleOutput::SetObject( TObject* object ) {

   m_object = object;
   return;

}

const TString& SCycleOutput::GetPath() const {

   return m_path;

}

void SCycleOutput::SetPath( const TString& path ) {

   m_path = path;
   return;

}

/**
 * Now this is a tricky one. In order to be able to merge trees, histograms,
 * and any other kinds of ROOT objects, this function has to be very generic.
 * (Remember, TObject doesn't define a "Merge" function!) Much of the code
 * here is inspired by ROOT code. (I mean I never would've figured out the
 * usage of TMethodCall without seeing some examples...)
 *
 * @param coll Collection of objects to merge into this one. Usually PROOF
 *             takes care of creating it.
 */
Int_t SCycleOutput::Merge( TCollection* coll ) {

   //
   // Return right away if the input is flawed:
   //
   if( ! coll ) return 0;
   if( coll->IsEmpty() ) return 0;

   //
   // Select the elements from the collection that can actually be merged:
   //
   TList list;
   TIter next( coll );
   TObject* obj = 0;
   while( ( obj = next() ) ) {

      //
      // See if it is an SCycleOutput object itself:
      //
      SCycleOutput* sobj = dynamic_cast< SCycleOutput* >( obj );
      if( ! sobj ) {
         m_logger << ERROR << "Trying to merge \"" << obj->ClassName()
                  << "\" object into \"" << this->ClassName() << "\"" << SLogger::endmsg;
         continue;
      }

      //
      // See if it holds the same kind of object as this output object:
      //
      TObject* mobj = sobj->GetObject();
      if( ! mobj ) continue;
      if( strcmp( mobj->ClassName(), this->GetObject()->ClassName() ) ) {
         m_logger << ERROR << "Trying to merge \"" << mobj->ClassName()
                  << "\" object into \"" << this->GetObject()->ClassName()
                  << SLogger::endmsg;
         continue;
      }

      //
      // If everything is fine, add it to the list of objects to merge:
      //
      list.Add( mobj );

   }

   //
   // Stop if the list is empty:
   //
   if( list.IsEmpty() ) {
      m_logger << WARNING << "No suitable object found for merging" << SLogger::endmsg;
      return 0;
   }

   //
   // Make sure that my object supports merging:
   //
   TMethodCall mergeMethod;
   mergeMethod.InitWithPrototype( this->GetObject()->IsA(), "Merge", "TCollection*" );
   if( ! mergeMethod.IsValid() ) {
      m_logger << ERROR << "Object type \"" << this->GetObject()->ClassName()
               << "\" doesn't support merging" << SLogger::endmsg;
      return 0;
   }

   //
   // Execute the merging:
   //
   mergeMethod.SetParam( ( Long_t ) &list );
   mergeMethod.Execute( this->GetObject() );

   //
   // A little feedback of what we've done:
   //
   m_logger << DEBUG << "Merged objects of type \"" << this->GetObject()->ClassName()
            << "\"" << SLogger::endmsg;

   return 1;

}

Int_t SCycleOutput::Write( const char* name, Int_t option,
                           Int_t bufsize ) const {

   if( ! m_object ) return -1;

   TDirectory* origDir = gDirectory;
   TDirectory* outDir = MakeDirectory( m_path );

   TTree* tobject = dynamic_cast< TTree* >( m_object );
   if( tobject ) tobject->SetDirectory( outDir );

   outDir->cd();
   Int_t ret = m_object->Write( name, option, bufsize );
   if( tobject ) tobject->AutoSave();
   origDir->cd();

   if( tobject ) tobject->SetDirectory( 0 );

   m_logger << VERBOSE << "Written object \"" << m_object->GetName()
            <<"\" to: " << outDir->GetPath();

   return ret;

}

TDirectory* SCycleOutput::MakeDirectory( const TString& path ) throw( SError ) {

   if( ! path.Length() ) return gDirectory;

   TDirectory* dir = 0;
   if( ! ( dir = gDirectory->GetDirectory( path ) ) ) {

      SLogger logger( "SCycleOutput" );
      logger << VERBOSE << "Creating directory: "
             << gDirectory->GetPath() << "/" << path << SLogger::endmsg;

      //
      // Break up the path name at the slashes:
      //
      std::vector< TString > directories;
      Ssiz_t previous_pos = ( path[ 0 ] == '/' ? 1 : 0 );
      Ssiz_t current_pos = 0;
      for( ; ; ) {
         current_pos = path.Index( "/", 1, previous_pos, TString::kExact );
         directories.push_back( path( previous_pos,
                                      current_pos - previous_pos ).String() );
         if( current_pos == kNPOS ) break;
         previous_pos = current_pos + 1;
      }

      //
      // Create each necessary directory:
      //
      dir = gDirectory;
      TDirectory* tempDir = 0;
      for( std::vector< TString >::const_iterator it = directories.begin();
           it != directories.end(); ++it ) {

         logger << VERBOSE << "Accessing directory: " << *it << SLogger::endmsg;
         if( ! ( tempDir = dir->GetDirectory( *it ) ) ) {
            logger << VERBOSE << "Directory doesn't exist, creating it..."
                   << SLogger::endmsg;
            if( ! ( tempDir = dir->mkdir( *it, "dummy title" ) ) ) {
               SError error( SError::SkipInputData );
               error << "Couldn't create directory: " << path
                     << " in the output file!";
               throw error;
            }
         }
         dir = tempDir;

      }

   }

   return dir;

}
