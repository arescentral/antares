#ifndef ANTARES_STUB_NET_SPROCKET_H_
#define ANTARES_STUB_NET_SPROCKET_H_

#include <Base.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum {
  kNSpPlayerJoined = 1,
  kNSpPlayerLeft = 2,
};

typedef struct { } NSpProtocolListReference;

typedef struct { } NSpEventProc;
typedef NSpEventProc* NSpEventProcPtr;

typedef struct { } NSpAddressReference;
typedef struct { } NSpMessageHeader;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_NET_SPROCKET_H_
