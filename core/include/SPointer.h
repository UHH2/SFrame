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

#ifndef SFRAME_CORE_SPOINTER_H
#define SFRAME_CORE_SPOINTER_H

// ROOT include(s):
#include <TObject.h>

/**
 *   @short Class used internally by SCycleBaseNTuple
 *
 *          SCycleBaseNTuple creates some objects when the user calls
 *          SCycleBaseNTuple::ConnectVariable(...). Since the user is not
 *          told to delete these objects, SCycleBaseNTuple has to do this
 *          itself. Such objects are used to do this.
 *
 *          Such an SPointer object owns an object given to it in its
 *          constructor. Very much like std::auto_ptr does. But unlike
 *          std::auto_ptr, it inherits from TObject. So it is possible to
 *          store multiple SPointer objects together without knowing their
 *          exact types.
 *
 * @version $Revision$
 */
template< class T >
class SPointer : public TObject {

public:
   /// Constructor taking ownership of the object
   SPointer( T* object );
   /// Destructor deleting the owned object
   ~SPointer();

private:
   /// Object owned by this pointer object
   T* m_object;

}; // class SPointer

#ifndef __CINT__
#include "SPointer.icc"
#endif

#endif // SFRAME_CORE_SPOINTER_H
