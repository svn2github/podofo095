#ifndef PODOFO_API_H_20061017
#define PODOFO_API_H_20061017

/*
 * This header provides a macro to handle correct symbol imports/exports
 * on platforms that require explicit instructions to make symbols public,
 * or differentiate between exported and imported symbols.
 * 
 * Win32 compilers use this information, and gcc4 can use it on *nix
 * to reduce the size of the export symbol table and get faster runtime
 * linking.
 *
 * All declarations of public API should be marked with the PODOFO_API macro.
 * Separate definitions need not be annotated, even in headers.
 *
 * Usage examples:
 *
 * class PODOFO_API PdfArray : public PdfDataType {
 *     ...
 * };
 *
 * bool PODOFO_API doThatThing(void);
 *
 * For an exception type that may be thrown across a DSO boundary, you must
 * use:
 *
 * class PODOFO_EXCEPTION_API(PODOFO_API) MyException
 * {
 *     ...
 * };
 *
 * PODOFO_LOCAL can be used on members of a class exported with PODOFO_API
 * to omit some members from the symbol table on supporting platforms. This
 * helps keep the exported API cleaner and the symbol table smaller.
 *
 * To hide a given method in an otherwise exported class:
 *
 * class PODOFO_API Myclass
 * {
 *     // blah blah
 * private:
 *     void privateHelper() PODOFO_LOCAL;
 * };
 *
 * For information on the gcc visibility support see:
 *     http://gcc.gnu.org/wiki/Visibility
 *     http://people.redhat.com/drepper/dsohowto.pdf
 *
 *
 *
 *
 * Note that gcc has some other useful attributes:
 *     http://gcc.gnu.org/onlinedocs/gcc-4.0.0/gcc/Function-Attributes.html
 *     http://gcc.gnu.org/onlinedocs/gcc-4.0.0/gcc/Variable-Attributes.html
 *     http://gcc.gnu.org/onlinedocs/gcc-4.0.0/gcc/Type-Attributes.html
 *
 * including __attribute__((deprecated)) for deprecating old interfaces.
 *           (available as PODOFO_DEPRECATED)
 *
 *           __attribute__((pure)) for functions w/o side effects
 *           (available as PODOFO_PURE_FUNCTION)
 *
 */

// Peter Petrov 26 April 2008
/* Automatically defined by CMake when building a shared library */
#if defined (podofo_EXPORTS)
    #define COMPILING_SHARED_PODOFO
    #undef USING_SHARED_PODOFO
#endif

/* Automatically defined by CMake when building a shared library */
#if defined(podofo_shared_EXPORTS)
    #define COMPILING_SHARED_PODOFO
#endif

/* Sanity check - can't be both compiling and using shared podofo */
#if defined(COMPILING_SHARED_PODOFO) && defined(USING_SHARED_PODOFO)
    #error "Both COMPILING_SHARED_PODOFO and USING_SHARED_PODOFO defined!"
#endif

/*
 * Define COMPILING_SHARED_PODOFO when building the PoDoFo library as a
 * DLL. When building code that uses that DLL, define USING_SHARED_PODOFO.
 *
 * Building or linking to a static library does not require either
 * preprocessor symbol.
 */
#if defined(_WIN32)
    #if defined(COMPILING_SHARED_PODOFO)
        #define PODOFO_API __declspec(dllexport)
    #elif defined(USING_SHARED_PODOFO)
        #define PODOFO_API __declspec(dllimport)
    #else
        #define PODOFO_API
    #endif
    /* PODOFO_LOCAL doesn't mean anything on win32, it's to exclude
     * symbols from the export table with gcc4. */
    #define PODOFO_LOCAL
#else
    #if defined(PODOFO_HAVE_GCC_SYMBOL_VISIBILITY)
        /* Forces inclusion of a symbol in the symbol table, so
           software outside the current library can use it. */
        #define PODOFO_API __attribute__ ((visibility("default")))
        /* Within a section exported with PODOFO_API, forces a symbol to be
           private to the library / app. Good for private members. */
        #define PODOFO_LOCAL __attribute__ ((visibility("hidden")))
        /* Forces even stricter hiding of methods/functions. The function must
         * absolutely never be called from outside the module even via a function
         * pointer.*/
        #define PODOFO_INTERNAL __attribute__ ((visibility("internal")))
    #else
        #define PODOFO_API
        #define PODOFO_LOCAL
        #define PODOFO_INTERNAL
    #endif
#endif

/* Throwable classes must always be exported by all binaries when
 * using gcc. Marking exception classes with PODOFO_EXCEPTION_API
 * ensures this. */
#ifdef _WIN32
  #define PODOFO_EXCEPTION_API(api) api
#elif defined(PODOFO_HAVE_GCC_SYMBOL_VISIBILITY)
  #define PODOFO_EXCEPTION_API(api) PODOFO_API
#else
  #define PODOFO_EXCEPTION_API(api)
#endif

/* Set up some other compiler-specific but not platform-specific macros */

#if defined(__GNUC__)
    /* gcc will issue a warning if a function or variable so annotated is used */
    #define PODOFO_DEPRECATED       __attribute__((deprecated))
    /* gcc can do some additional optimisations on functions annotated as pure.
     * See the documentation on __attribute__((pure)) in the gcc docs. */
    #define PODOFO_PURE_FUNCTION    __attribute__((pure))
    /* PODOFO_NOTHROW can be used to tell the compiler the annotated function is
     * guaranteed not to throw. If it does throw, undefined behaviour will result,
     * so be VERY careful with this. This is NOT the same as the PODOFO_NOTHROW qualifier
     * (see CODINGSTYLE.txt) .*/
    #define PODOFO_NOTHROW          __attribute__((nothrow))
#else
    #define PODOFO_DEPRECATED
    #define PODOFO_PURE_FUNCTION
    #define PODOFO_NOTHROW          __declspec(nothrow)
#endif

// Peter Petrov 27 April 2008
// Disable warnings
#ifdef _WIN32
#pragma warning(disable: 4251)
#pragma warning(disable: 4309)
#endif // _WIN32

#endif // PODOFO_API_H
