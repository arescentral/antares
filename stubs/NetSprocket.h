#ifndef ANTARES_STUB_NET_SPROCKET_H_
#define ANTARES_STUB_NET_SPROCKET_H_

#include <Base.h>

enum {
  kNSpPlayerJoined = 1,
  kNSpPlayerLeft = 2,
};

typedef struct { } NSpProtocolListReference;

typedef struct { } NSpEventProc;
typedef NSpEventProc* NSpEventProcPtr;

typedef struct { } NSpAddressReference;
typedef struct { } NSpMessageHeader;

#endif // ANTARES_STUB_NET_SPROCKET_H_
