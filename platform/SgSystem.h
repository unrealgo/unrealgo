
#ifndef SG_SYSTEM_H
#define SG_SYSTEM_H

// Used by GNU Autotools
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


template<class T>
inline void SuppressUnused(const T &) {}

// warn for debug build
#ifndef NDEBUG
#define SG_DEBUG_ONLY(x)
#else
#define SG_DEBUG_ONLY(x) SuppressUnused(x)
#endif

//----------------------------------------------------------------------------

// Explicit inlining attributes. The macros are defined as non-empty only
// if supported by the compiler (note that Intel ICC and CLANG also define
// __GNUC__, but would ignore the attributes with a warning)

#if defined(__GNUC__) && !defined(__ICC)
#define SG_ATTR_ALWAYS_INLINE __attribute__((always_inline))
#define SG_ATTR_NOINLINE __attribute__((noinline))
#else
#define SG_ATTR_NOINLINE
#define SG_ATTR_ALWAYS_INLINE
#endif

#if defined(__GNUC__) && !defined(__ICC) && \
    !defined(__clang__) && \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#define SG_ATTR_FLATTEN __attribute__((flatten))
#else
#define SG_ATTR_FLATTEN
#endif

namespace SgDeterministic {
  void SetDeterministicMode(bool flag);
  bool IsDeterministicMode();
}

class AssertionHandlerInterface {
 public:
  AssertionHandlerInterface();
  virtual ~AssertionHandlerInterface();
  virtual void Run() = 0;
};


// only assert for debug build
#ifndef NDEBUG
/** Help the Clang static analyzer use our assertions.
    See http://clang-analyzer.llvm.org/annotations.html#attr_analyzer_noreturn
*/
#ifndef CLANG_ANALYZER_NORETURN
#if defined(__has_feature)
#if __has_feature(attribute_analyzer_noreturn)
#define CLANG_ANALYZER_NORETURN __attribute__((analyzer_noreturn))
#endif
#endif
#endif
#ifndef CLANG_ANALYZER_NORETURN
#define CLANG_ANALYZER_NORETURN
#endif

/** System-specific action when an DBG_ASSERT fails */
void SgHandleAssertion(const char *expr, const char *file, int line)
CLANG_ANALYZER_NORETURN;
#define DBG_ASSERT(x) \
    do \
    { \
        if (! (x)) \
            ::SgHandleAssertion(#x, __FILE__, __LINE__); \
    } while (false)
#else // Non Debug Build
#define DBG_ASSERT(x) (static_cast<void>(0))
#endif


#define DBG_ASSERT_EQUAL(x, y) DBG_ASSERT((x) == (y))
#define DBG_ASSERTRANGE(i, from, to) DBG_ASSERT((i) >= (from) && (i) <= (to))


#ifndef NDEBUG
const bool SG_CHECK = true;
const bool SG_HEAVYCHECK = SG_CHECK && true;
#else
const bool SG_CHECK = false;
const bool SG_HEAVYCHECK = false;
#endif


#ifdef _MSC_VER
#pragma warning(4:4355)

// Disable Visual C++ warnings about unsafe functions from the standard
// C++ library
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#endif // _MSC_VER


#ifdef __MINGW32__
#define WIN32 1
// Enable Windows2000 (0x0500) compatibility in MinGW header files
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#endif // __MINGW32__




void SetForceAbort(bool force_abort);
bool ForceAbort();




inline void SgSynchronizeThreadMemory() {
#ifdef ENABLE_CACHE_SYNC
#ifdef HAVE_SYNC_SYNCHRONIZE
  __sync_synchronize();
#else
#error "Explicit cache synchronization requires __sync_synchronize() builtin"
#endif
#endif
}

#endif
