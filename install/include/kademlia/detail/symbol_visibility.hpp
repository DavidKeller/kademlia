
#ifndef KADEMLIA_EXPORT_H
#define KADEMLIA_EXPORT_H

#ifdef KADEMLIA_STATIC_DEFINE
#  define KADEMLIA_EXPORT
#  define KADEMLIA_NO_EXPORT
#else
#  ifndef KADEMLIA_EXPORT
#    ifdef kademlia_EXPORTS
        /* We are building this library */
#      define KADEMLIA_EXPORT 
#    else
        /* We are using this library */
#      define KADEMLIA_EXPORT 
#    endif
#  endif

#  ifndef KADEMLIA_NO_EXPORT
#    define KADEMLIA_NO_EXPORT 
#  endif
#endif

#ifndef KADEMLIA_DEPRECATED
#  define KADEMLIA_DEPRECATED __declspec(deprecated)
#endif

#ifndef KADEMLIA_DEPRECATED_EXPORT
#  define KADEMLIA_DEPRECATED_EXPORT KADEMLIA_EXPORT KADEMLIA_DEPRECATED
#endif

#ifndef KADEMLIA_DEPRECATED_NO_EXPORT
#  define KADEMLIA_DEPRECATED_NO_EXPORT KADEMLIA_NO_EXPORT KADEMLIA_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef KADEMLIA_NO_DEPRECATED
#    define KADEMLIA_NO_DEPRECATED
#  endif
#endif

#endif /* KADEMLIA_EXPORT_H */
