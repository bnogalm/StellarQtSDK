#ifndef STELLAR_QT_SDK_DEPRECATED_H
#define STELLAR_QT_SDK_DEPRECATED_H

/**
 * STELLAR_DEPRECATED(msg)
 * ------------------------
 * Marker for SDK symbols that are kept for source-compatibility but should
 * not be used in new code. Wraps `[[deprecated(msg)]]` (C++14, supported by
 * MSVC ≥ 2015 even in C++11 mode).
 *
 * Downstream consumers can globally silence deprecation noise by defining
 *   STELLAR_QT_SDK_ENABLE_DEPRECATED 0
 * before including SDK headers (the symbols stay; only the attribute is
 * dropped, so warnings disappear).
 *
 * Default: deprecations are active.
 */

#ifndef STELLAR_QT_SDK_ENABLE_DEPRECATED
#  define STELLAR_QT_SDK_ENABLE_DEPRECATED 1
#endif

#if STELLAR_QT_SDK_ENABLE_DEPRECATED
#  if defined(__has_cpp_attribute)
#    if __has_cpp_attribute(deprecated)
#      define STELLAR_DEPRECATED(msg) [[deprecated(msg)]]
#    endif
#  endif
#  ifndef STELLAR_DEPRECATED
#    if defined(_MSC_VER)
#      define STELLAR_DEPRECATED(msg) __declspec(deprecated(msg))
#    elif defined(__GNUC__) || defined(__clang__)
#      define STELLAR_DEPRECATED(msg) __attribute__((deprecated(msg)))
#    else
#      define STELLAR_DEPRECATED(msg)
#    endif
#  endif
#else
#  define STELLAR_DEPRECATED(msg)
#endif

#endif // STELLAR_QT_SDK_DEPRECATED_H
