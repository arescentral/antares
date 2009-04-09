#ifndef ANTARES_STUB_PROCESS_H_
#define ANTARES_STUB_PROCESS_H_

#include <Files.h>

typedef uint64_t ProcessSerialNumber;
typedef struct {
    unsigned char* processName;
    FSSpec* processAppSpec;
    size_t processInfoLength;
} ProcessInfoRec;
OSErr GetCurrentProcess(ProcessSerialNumber* serial);
OSErr GetProcessInformation(ProcessSerialNumber* serial, ProcessInfoRec* info);

#endif // ANTARES_STUB_PROCESS_H_
