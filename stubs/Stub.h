#ifndef ANTARES_STUB_STUB_H_
#define ANTARES_STUB_STUB_H_

#include <stdio.h>

namespace antares {

template <typename T>
struct function_traits { };

template <typename R>
struct function_traits<R()> {
  typedef R return_type;
};

template <typename R, typename A0>
struct function_traits<R(A0)> {
  typedef R return_type;
  typedef A0 arg0_type;
};

template <typename R, typename A0, typename A1>
struct function_traits<R(A0, A1)> {
  typedef R return_type;
  typedef A0 arg0_type;
  typedef A1 arg1_type;
};

template <typename R, typename A0, typename A1, typename A2>
struct function_traits<R(A0, A1, A2)> {
  typedef R return_type;
  typedef A0 arg0_type;
  typedef A1 arg1_type;
  typedef A2 arg2_type;
};

template <typename R, typename A0, typename A1, typename A2, typename A3>
struct function_traits<R(A0, A1, A2, A3)> {
  typedef R return_type;
  typedef A0 arg0_type;
  typedef A1 arg1_type;
  typedef A2 arg2_type;
  typedef A3 arg3_type;
};

template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
struct function_traits<R(A0, A1, A2, A3, A4)> {
  typedef R return_type;
  typedef A0 arg0_type;
  typedef A1 arg1_type;
  typedef A2 arg2_type;
  typedef A3 arg3_type;
  typedef A4 arg4_type;
};

template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct function_traits<R(A0, A1, A2, A3, A4, A5)> {
  typedef R return_type;
  typedef A0 arg0_type;
  typedef A1 arg1_type;
  typedef A2 arg2_type;
  typedef A3 arg3_type;
  typedef A4 arg4_type;
  typedef A5 arg5_type;
};

template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5,
         typename A6>
struct function_traits<R(A0, A1, A2, A3, A4, A5, A6)> {
  typedef R return_type;
  typedef A0 arg0_type;
  typedef A1 arg1_type;
  typedef A2 arg2_type;
  typedef A3 arg3_type;
  typedef A4 arg4_type;
  typedef A5 arg5_type;
  typedef A6 arg6_type;
};

template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5,
         typename A6, typename A7>
struct function_traits<R(A0, A1, A2, A3, A4, A5, A6, A7)> {
  typedef R return_type;
  typedef A0 arg0_type;
  typedef A1 arg1_type;
  typedef A2 arg2_type;
  typedef A3 arg3_type;
  typedef A4 arg4_type;
  typedef A5 arg5_type;
  typedef A6 arg6_type;
  typedef A7 arg7_type;
};

#define gdb() __asm__("int3")

#define STUB0(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME() { \
    return __VA_ARGS__; \
  }

#define STUB1(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME( \
      function_traits<TYPE>::arg0_type a0) { \
    (void)a0; \
    return __VA_ARGS__; \
  }

#define STUB2(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME( \
      function_traits<TYPE>::arg0_type a0, \
      function_traits<TYPE>::arg1_type a1) { \
    (void)a0; \
    (void)a1; \
    return __VA_ARGS__; \
  }

#define STUB3(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME( \
      function_traits<TYPE>::arg0_type a0, \
      function_traits<TYPE>::arg1_type a1, \
      function_traits<TYPE>::arg2_type a2) { \
    (void)a0; \
    (void)a1; \
    (void)a2; \
    return __VA_ARGS__; \
  }

#define STUB4(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME( \
      function_traits<TYPE>::arg0_type a0, \
      function_traits<TYPE>::arg1_type a1, \
      function_traits<TYPE>::arg2_type a2, \
      function_traits<TYPE>::arg3_type a3) { \
    (void)a0; \
    (void)a1; \
    (void)a2; \
    (void)a3; \
    return __VA_ARGS__; \
  }

#define STUB5(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME( \
      function_traits<TYPE>::arg0_type a0, \
      function_traits<TYPE>::arg1_type a1, \
      function_traits<TYPE>::arg2_type a2, \
      function_traits<TYPE>::arg3_type a3, \
      function_traits<TYPE>::arg4_type a4) { \
    (void)a0; \
    (void)a1; \
    (void)a2; \
    (void)a3; \
    (void)a4; \
    return __VA_ARGS__; \
  }

#define STUB6(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME( \
      function_traits<TYPE>::arg0_type a0, \
      function_traits<TYPE>::arg1_type a1, \
      function_traits<TYPE>::arg2_type a2, \
      function_traits<TYPE>::arg3_type a3, \
      function_traits<TYPE>::arg4_type a4, \
      function_traits<TYPE>::arg5_type a5) { \
    (void)a0; \
    (void)a1; \
    (void)a2; \
    (void)a3; \
    (void)a4; \
    (void)a5; \
    return __VA_ARGS__; \
  }

#define STUB7(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME( \
      function_traits<TYPE>::arg0_type a0, \
      function_traits<TYPE>::arg1_type a1, \
      function_traits<TYPE>::arg2_type a2, \
      function_traits<TYPE>::arg3_type a3, \
      function_traits<TYPE>::arg4_type a4, \
      function_traits<TYPE>::arg5_type a5, \
      function_traits<TYPE>::arg6_type a6) { \
    (void)a0; \
    (void)a1; \
    (void)a2; \
    (void)a3; \
    (void)a4; \
    (void)a5; \
    (void)a6; \
    return __VA_ARGS__; \
  }

#define STUB8(NAME, TYPE, ...) \
  inline function_traits<TYPE>::return_type \
  NAME( \
      function_traits<TYPE>::arg0_type a0, \
      function_traits<TYPE>::arg1_type a1, \
      function_traits<TYPE>::arg2_type a2, \
      function_traits<TYPE>::arg3_type a3, \
      function_traits<TYPE>::arg4_type a4, \
      function_traits<TYPE>::arg5_type a5, \
      function_traits<TYPE>::arg6_type a6, \
      function_traits<TYPE>::arg7_type a7) { \
    (void)a0; \
    (void)a1; \
    (void)a2; \
    (void)a3; \
    (void)a4; \
    (void)a5; \
    (void)a6; \
    (void)a7; \
    return __VA_ARGS__; \
  }

}  // namespace antares

#endif // ANTARES_STUB_STUB_H_
