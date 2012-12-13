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

/**
 * @file plug-ins/include/FPCompare.h
 * @author scott snyder
 * @date Sep 2008
 * @brief Workaround x86 precision issues for FP inequality comparisons.
 *
 * The functions contained here can be used to work around one of the
 * effects of the brain-damage of the x87 FPU.
 *
 * Brief summary: If you're writing a comparison function for sort,
 * where the comparison depends on computed floating-point values, eg:
 *
 *@code
 *    bool compare (IParticle* a, IParticle* b)
 *    { return a->pt() > b->pt(); }
 *@endcode
 *
 * then you should replace the comparison with a call to one of the
 * functions in this file:
 *
 *@code
 *    bool compare (IParticle* a, IParticle* b)
 *    { return CxxUtils::fpcompare::greater (a->pt(), b->pt()); }
 *@endcode
 *
 * Longer explanation:
 *
 * An expression like this (where pt() returns a double):
 *
 *@code
 *    a->pt() > b->pt()
 *@endcode
 *
 * is compiled (on x86) into a sequence like this:
 *
 *     call a->pt()
 *     save result from FPU to a double stack temporary
 *     call b->pt()
 *     load the temporary back into the FPU
 *     do the comparison
 *
 * If pt() returns a result with the extra precision bits used
 * (so that the value changes when rounded to a double), then
 * it is possible for this comparison to return true for the
 * case where a==b.  This violates the assumptions that std::sort
 * makes of the comparison function, and can cause a crash
 * (possibly even silently wrong results!).
 *
 * As a fix, we force both parameters into something that has been declared
 * @c volatile.  That forces them to be spilled to memory, ensuring
 * that they are both correctly rounded for the declared data type.
 * The comparison is then done on these rounded values.
 *
 * We condition this on the parameter @c __FLT_EVAL_METHOD__ being 2.
 * This is defined in the C standard; a value of 2 means that all
 * FP calculations are done as long double.  For other cases,
 * we leave out the @c volatile qualifiers; this should result
 * in the functions being inlined completely away.
 *
 * In addition to the free functions in the @c CxxUtils::fpcompare
 * namespace. we define corresponding functionals in the
 * @c CxxUtils::fpcompare_fn namespace.
 *
 * It's also worth pointing out that exactly the same issue arises
 * if one uses a floating-point value as the key for a STL associative
 * container.  In that case, this comparison instability may cause the
 * container to become corrupted.  While it's probably best to avoid
 * using floats for associative container keys in the first place,
 * if you do have to do that, you can work around this problem by
 * using one of the above functionals as the container's comparison type.
 */

#ifndef SFRAME_PLUGINS_FPCOMPARE_H
#define SFRAME_PLUGINS_FPCOMPARE_H

// STL include(s):
#include <functional>

// Decide whether we need to use volatile or not.
#if defined(__FLT_EVAL_METHOD__) && \
   (__FLT_EVAL_METHOD__ == 2 || __FLT_EVAL_METHOD__ < 0)
   // __FLT_EVAL_METHOD__ < 0 means unspecified.
   // Be pessimistic in that case.
#  define CXXUTILS_FPCOMPARE_VOLATILE volatile
#elif defined(__i386__) && !defined(__SSE2__)
   // On x86, gcc -msse -mfpmath=sse is observed to _not_ generate
   // sse fp instructions, but does set __FLT_EVAL_METHOD__ to 0.
   // -msse2 -mfpmath=sse does seem to work as expected.
   // Special-case this for now; should follow up with a gcc bug report
   // if this still happens in current releases.
#  define CXXUTILS_FPCOMPARE_VOLATILE volatile
#else
#  define CXXUTILS_FPCOMPARE_VOLATILE
#endif

/**
 * @short Namespace for the utility functions taken from the ATLAS offline code
 *
 *        I stole this code from the ATLAS offline code. This was written
 *        by Scott Snyder.
 *
 *   @see plug-ins/include/FPCompare.h
 *
 * $Revision$
 * $Date$
 */
namespace CxxUtils {

   /**
    * @short Namespace for the safe comparison functions
    *
    *        The functions in this namespace can be used to implement "safe"
    *        comparisons between floating point numbers in all circumstances.
    *
    *   @see plug-ins/include/FPCompare.h
    *
    * $Revision$
    * $Date$
    */
   namespace fpcompare {

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a == @a b
       */
      inline
      bool equal (double a, double b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE double va = a;
         CXXUTILS_FPCOMPARE_VOLATILE double vb = b;
         return va == vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a == @a b
       */
      inline
      bool equal (float a, float b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE float va = a;
         CXXUTILS_FPCOMPARE_VOLATILE float vb = b;
         return va == vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a > @a b
       */
      inline
      bool greater (double a, double b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE double va = a;
         CXXUTILS_FPCOMPARE_VOLATILE double vb = b;
         return va > vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a > @a b
       */
      inline
      bool greater (float a, float b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE float va = a;
         CXXUTILS_FPCOMPARE_VOLATILE float vb = b;
         return va > vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a < @a b
       */
      inline
      bool less (double a, double b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE double va = a;
         CXXUTILS_FPCOMPARE_VOLATILE double vb = b;
         return va < vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a < @a b
       */
      inline
      bool less (float a, float b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE float va = a;
         CXXUTILS_FPCOMPARE_VOLATILE float vb = b;
         return va < vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a >= @a b
       */
      inline
      bool greater_equal (double a, double b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE double va = a;
         CXXUTILS_FPCOMPARE_VOLATILE double vb = b;
         return va >= vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a >= @a b
       */
      inline
      bool greater_equal (float a, float b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE float va = a;
         CXXUTILS_FPCOMPARE_VOLATILE float vb = b;
         return va >= vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a <= @a b
       */
      inline
      bool less_equal (double a, double b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE double va = a;
         CXXUTILS_FPCOMPARE_VOLATILE double vb = b;
         return va <= vb;
      }

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       * @return @a a <= @a b
       */
      inline
      bool less_equal (float a, float b)
      {
         CXXUTILS_FPCOMPARE_VOLATILE float va = a;
         CXXUTILS_FPCOMPARE_VOLATILE float vb = b;
         return va <= vb;
      }

   } // namespace fpcompare

   /**
    * @short Namespace for the safe comparison functionals
    *
    *        The function objects in this namespace can be used to implement
    *        "safe" comparisons between floating point numbers in all
    *        circumstances.
    *
    *   @see plug-ins/include/FPCompare.h
    *
    * $Revision$
    * $Date$
    */
   namespace fpcompare_fn {

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct equal
      : public std::binary_function<double, double, bool>
      {
         bool
         operator()(double a, double b) const
         { return fpcompare::equal (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct equalf
      : public std::binary_function<float, float, bool>
      {
         bool
         operator()(float a, float b) const
         { return fpcompare::equal (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct greater
      : public std::binary_function<double, double, bool>
      {
         bool
         operator()(double a, double b) const
         { return fpcompare::greater (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct greaterf
      : public std::binary_function<float, float, bool>
      {
         bool
         operator()(float a, float b) const
         { return fpcompare::greater (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct less
      : public std::binary_function<double, double, bool>
      {
         bool
         operator()(double a, double b) const
         { return fpcompare::less (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct lessf
      : public std::binary_function<float, float, bool>
      {
         bool
         operator()(float a, float b) const
         { return fpcompare::less (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct greater_equal
      : public std::binary_function<double, double, bool>
      {
         bool
         operator()(double a, double b) const
         { return fpcompare::greater_equal (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct greater_equalf
      : public std::binary_function<float, float, bool>
      {
         bool
         operator()(float a, float b) const
         { return fpcompare::greater_equal (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct less_equal
      : public std::binary_function<double, double, bool>
      {
         bool
         operator()(double a, double b) const
         { return fpcompare::less_equal (a, b); }
      };

      /**
       * @brief Compare two FP numbers, working around x87 precision issues.
       */
      struct less_equalf
      : public std::binary_function<float, float, bool>
      {
         bool
         operator()(float a, float b) const
         { return fpcompare::less_equal (a, b); }
      };

   } // namespace fpcompare_fn

} // namespace CxxUtils

#endif // SFRAME_PLUGINS_FPCOMPARE_H
