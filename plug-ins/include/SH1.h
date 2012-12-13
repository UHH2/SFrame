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

#ifndef SFRAME_PLUGINS_SH1_H
#define SFRAME_PLUGINS_SH1_H

// ROOT include(s):
#include <TNamed.h>

// SFrame include(s):
#include "core/include/SError.h"

// Forward declaration(s):
class TCollection;
class TH1;

/**
 *  @short Ligh-weight 1-dimensional histogram class
 *
 *         In many cases the TH1* histograms of ROOT can be unnecessarily
 *         "heavy". Those objects take quite a lot of memory, and are not the
 *         quickest. However they also provide a lot of functionality in return.
 *
 *         This class can be used when you just want to see some simple
 *         distribution. It only provides the very basic features. The created
 *         histograms can only have evenly sized bins for instance.
 *
 *         But for convenience when such an object is written out to a file, it
 *         is written out as an appropriate TH1 histogram, with the same
 *         contents as the object has in memory. So in principle changing one's
 *         code to use SH1D histograms instead of TH1D histograms in the cycle,
 *         should not be visible in the output file.
 *
 *         When creating the objects in the default mode (with statistical
 *         uncertainty computation enabled), they give about a 50% speed
 *         increase over using TH1. But when the statistical uncertainty
 *         computation is turned off, the speed increase is >200%.
 *
 * @version $Revision$
 */
template< typename Type >
class SH1 : public TNamed {

public:
   /// Default constructor
   SH1();
   /// Fancy copy constructor
   template< typename T > SH1( const SH1< T >& parent );
   /// Regular constructor with all parameters
   SH1( const char* name, const char* title, Int_t bins,
        Double_t low, Double_t high, Bool_t computeErrors = kTRUE );
   /// Destructor
   virtual ~SH1();

   /// Increase the contents of the bin at a specific position
   void Fill( Double_t pos, Type weight = 1 ) throw( SError );

   /// Get the number of bins
   Int_t GetNBins() const;
   /// Find the bin belonging to a specific position on the axis
   Int_t FindBin( Double_t pos ) const;

   /// Get the content of a specific bin
   Type GetBinContent( Int_t bin ) const;
   /// Set the content of a specific bin
   void SetBinContent( Int_t bin, Type content );

   /// Get the error of a specific bin
   Type GetBinError( Int_t bin ) const;
   /// Set the error of a specific bin
   void SetBinError( Int_t bin, Type error );

   /// Get the total number of entries in the histogram
   Int_t GetEntries() const;
   /// Set the total number of entries in the histogram
   void SetEntries( Int_t entries );

   /// Function creating a TH1 histogram with the contents of the object
   TH1* ToHist() const;

   /// Merge a collection of SH1 objects
   virtual Int_t Merge( TCollection* coll );
   /// Write the SH1 object as a TH1 object (const version)
   virtual Int_t Write( const char* name = 0, Int_t option = 0,
                        Int_t bufsize = 0 ) const;
   /// Write the SH1 object as a TH1 object (non-const version)
   virtual Int_t Write( const char* name = 0, Int_t option = 0,
                        Int_t bufsize = 0 );

private:
   /// Size of the internal arrays (needed for dictionary generation)
   const Int_t m_arraySize;
   /// Array holding the bin contents
   Type* m_content; //[m_arraySize]
   /// Array holding the square of the bin errors
   Type* m_errors; //[m_arraySize]
   /// Number of entries in the histogram
   Int_t m_entries;
   /// Number of bins of the histogram
   const Int_t    m_bins;
   /// The low end of the histogram axis
   const Double_t m_low;
   /// The high end of the histogram axis
   const Double_t m_high;
   /// Whether statistical errors should be calculated
   const Bool_t m_computeErrors;

#ifndef DOXYGEN_IGNORE
   ClassDef( SH1, 1 )
#endif // DOXYGEN_IGNORE

}; // class SH1

//
// Include the template implementation:
//
#ifndef __CINT__
#include "SH1.icc"
#endif // __CINT__

//
// Define the supported template specialisations:
//
typedef SH1< Float_t >  SH1F;
typedef SH1< Double_t > SH1D;
typedef SH1< Int_t >    SH1I;

#ifndef DOXYGEN_IGNORE
ClassImp( SH1F )
ClassImp( SH1D )
ClassImp( SH1I )
#endif // DOXYGEN_IGNORE

#endif // SFRAME_PLUGINS_SH1_H
