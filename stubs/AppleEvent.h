#ifndef ANTARES_STUB_APPLE_EVENT_H_
#define ANTARES_STUB_APPLE_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <Base.h>

typedef struct { } AppleEvent;
typedef struct { } AEDescList;
typedef struct { } AEKeyword;
typedef FourCharCode AEDescType;
typedef FourCharCode DescType;

typedef OSErr (*AEEventHandlerUPP)(AppleEvent* event, AppleEvent* reply,
                                   long refcon);
typedef AEEventHandlerUPP AEEventHandlerProcPtr;

enum {
    typeAEList = 3000,
    typeFSS = 3001,
    typeWildCard = 3002,

    errAEDescNotFound = 3100,
    errAEEventNotHandled = 3101,

    keyDirectObject = 3200,
    keyMissedKeywordAttr = 3201,

    kCoreEventClass = 3300,

    kAEQuitApplication = 3400,
    kAEPrint = 3401,
    kAEOpenDocuments = 3402,
    kAEOpenApplication = 3403,
};

OSErr AEGetParamDesc(AppleEvent* event, int key, int type, AEDescList* docList);
OSErr AECountItems(AEDescList* docList, long* itemsInList);
OSErr AEGetNthPtr(AEDescList* docList, int index, int type, AEKeyword* kwd,
                  DescType* returned_type, FSSpec* fsspec, size_t Size,
                  Size* actual_size);
OSErr AEDisposeDesc(AEDescList* docList);
OSErr AEGetAttributePtr(AppleEvent* event, int key, int type,
                        DescType* returned_type, void*, int, Size* actual_size);
OSErr AEInstallEventHandler(int class_, int action, AEEventHandlerUPP upp,
                            int, bool);

AEEventHandlerUPP NewAEEventHandlerProc(AEEventHandlerProcPtr handler);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_APPLE_EVENT_H_
