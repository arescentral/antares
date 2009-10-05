#ifndef ANTARES_STUB_PROCESS_H_
#define ANTARES_STUB_PROCESS_H_

#include <Files.h>

namespace antares {

typedef uint64_t ProcessSerialNumber;
typedef struct {
    unsigned char* processName;
    FSSpec* processAppSpec;
    size_t processInfoLength;
} ProcessInfoRec;
STUB1(GetCurrentProcess, OSErr(ProcessSerialNumber* serial), noErr);
STUB2(GetProcessInformation, OSErr(ProcessSerialNumber* serial, ProcessInfoRec* info), noErr);

}  // namespace antares

#endif // ANTARES_STUB_PROCESS_H_
