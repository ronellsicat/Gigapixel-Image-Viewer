/** 
 * @file StdInclude.h
 *
 * @see StdInclude
 *
 * @author Jens Krueger
 */

#ifndef STDINCLUDE_H
#define STDINCLUDE_H

#include <iostream>

#ifndef NDEBUG

#define T_ERROR(...)                \
  do {                              \
    fprintf (stderr, __VA_ARGS__);  \
    fprintf (stderr, "\n");         \
  } while(0)
#define WARNING(...)                \
  do {                              \
    fprintf (stdout, __VA_ARGS__);  \
    fprintf (stdout, "\n");         \
  } while(0)
#define MESSAGE(...)                \
  do {                              \
    fprintf (stdout, __VA_ARGS__);  \
    fprintf (stdout, "\n");         \
  } while(0)
#define OTHER(...)                  \
  do {                              \
    fprintf (stdout, __VA_ARGS__);  \
    fprintf (stdout, "\n");         \
  } while(0)

#else

#define T_ERROR(...)                                              
#define WARNING(...)                                              
#define MESSAGE(...)                                              
#define OTHER(...)                                                

#endif

#ifdef __linux__

  #include <memory>
  #include <cstdint>

  using std::shared_ptr;

#else
/*
  // Disable checked iterators on Windows, only necessary for 
  // pre VS2010 versions (VS2010 has _MSC_VER = 1600)
  // in VS 2010 MS finally realized that a 2x performance
  // penalty for STL operations sucks, more details can be found here
  // http://channel9.msdn.com/Shows/Going+Deep/C9-Lectures-Stephan-T-Lavavej-Advanced-STL-3-of-n
  #ifndef _DEBUG
  #if _MSC_VER < 1600
  # undef _SECURE_SCL
  # define _SECURE_SCL 0
  #endif
  #endif
*/

  // shared_ptr etc
  #ifdef _MSC_VER
    // Get rid of stupid warnings.
    #define _CRT_SECURE_NO_WARNINGS 1
    // Get rid of stupid warnings Part 2.
    #define _SCL_SECURE_NO_WARNINGS 1
    #pragma warning(disable:4996)
    # include <memory>
  #else
    # include <tr1/memory>
  #endif

  /// get access to the size-defined integer types
  /// on vs 2010 we have native support for these
  /// files on older versions (and on gcc) we use
  /// boost, technically we can use the native 
  /// types on newer gccs too, so feel free to 
  /// extend this for gcc
  #if _MSC_VER >= 1600 // VS 2010 or newer
  using std::shared_ptr;

  #include <cstdint>
  #else 
  using std::tr1::shared_ptr;

  #include <boost/cstdint.hpp>
  using boost::int64_t;
  using boost::int32_t;
  using boost::int16_t;
  using boost::int8_t;
  using boost::uint64_t;
  using boost::uint32_t;
  using boost::uint16_t;
  using boost::uint8_t;
  #endif

#endif

#include <cassert>


#ifdef __GNUC__
# define FUNC_PURE  __attribute__((pure))
# define FUNC_CONST __attribute__((const))
#else
# define FUNC_PURE  /* nothing */
# define FUNC_CONST /* nothing */
#endif

#define UINT32_INVALID (std::numeric_limits<uint32_t>::max())

#endif // STDINCLUDE_H
