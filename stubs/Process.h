#ifndef ANTARES_STUB_PROCESS_H_
#define ANTARES_STUB_PROCESS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <Files.h>

typedef uint64_t ProcessSerialNumber;
typedef struct {
    unsigned char* processName;
    FSSpec* processAppSpec;
    size_t processInfoLength;
} ProcessInfoRec;
OSErr GetCurrentProcess(ProcessSerialNumber* serial);
OSErr GetProcessInformation(ProcessSerialNumber* serial, ProcessInfoRec* info);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_PROCESS_H_
