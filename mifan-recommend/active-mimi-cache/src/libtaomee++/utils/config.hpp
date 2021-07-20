#ifndef LIBTAOMEEPP_CONFIG_HPP_
#define LIBTAOMEEPP_CONFIG_HPP_

// undefine INLINE if previously defined
#ifdef INLINE
#undef INLINE
#endif
// define INLINE to be `inline` if NONINLINE is not defined
#ifndef NONINLINE
#define INLINE inline
#else
#define INLINE
#endif

#endif /* LIBTAOMEEPP_CONFIG_HPP_ */
