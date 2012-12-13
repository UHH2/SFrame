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

#ifndef SFRAME_CORE_STreeTypes_H
#define SFRAME_CORE_STreeTypes_H

// ROOT include(s):
#include <Rtypes.h>

/**
 *   @short Namespace holding the constants describing SFrame TTree types
 *
 *          SInputData can now describe an arbitrary number of types of TTree-s.
 *          This is needed to make it possible for the base SFrame package to be
 *          unaware of what extensions other packages (e.g. SFrameARA) add to
 *          the base functionality.
 *
 *          While this is not the most elegant solution, I can't just use a C++
 *          enumeration to describe the TTree types, as that wouldn't be
 *          extendible either. On the other hand adding a new Int_t constant in
 *          one of the extension packages is pretty easy.
 *
 * @version $Revision$
 */
namespace STreeType {

   /// Event-wise TTree that needs no "decoding"
   /**
    * This constant describes the most common input TTree type. TTree-s that
    * contain one entry per HEP event, are put into this category. SFrame takes
    * care of handling these TTree-s fully.
    */
   static const Int_t InputSimpleTree = 0;
   /// Metadata TTree that the user has to handle by hand
   /**
    * This type describes a TTree that does not describe event-level
    * information. For instance ATLAS saves some configuration information into
    * metadata TTree-s in some file formats. The handling of these trees is much
    * more up to the user than for "simple" TTree-s.
    */
   static const Int_t InputMetaTree = 1;

   /// Event-wise output TTree
   /**
    * This type describes the most common output TTree type. SFrame takes care
    * of writing one entry to these TTree-s for each accepted event, and
    * provides the user with easy-to-use functions for filling these TTree-s.
    */
   static const Int_t OutputSimpleTree = 100;
   /// Output TTree that doesn't describe event-level information
   /**
    * In some cases it's beneficial to write out TTree-s that don't describe one
    * event per entry, but for instance one particle per entry. Since many
    * applications can be imagines, it's up to the user to take care of filling
    * such TTree-s. SFrame only takes care of creating them, and handling them
    * over a PROOF farm.
    */
   static const Int_t OutputMetaTree = 101;

} // namespace STreeType

#endif // SFRAME_CORE_STreeTypes_H
