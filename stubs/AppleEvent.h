#ifndef ANTARES_STUB_APPLE_EVENT_H_
#define ANTARES_STUB_APPLE_EVENT_H_

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

STUB4(AEGetParamDesc, OSErr(AppleEvent* event, int key, int type, AEDescList* docList), noErr);
STUB2(AECountItems, OSErr(AEDescList* docList, long* itemsInList), noErr);
STUB8(AEGetNthPtr, OSErr(AEDescList* docList, int index, int type, AEKeyword* kwd,
                  DescType* returned_type, FSSpec* fsspec, size_t Size,
                  Size* actual_size), noErr);
STUB1(AEDisposeDesc, OSErr(AEDescList* docList), noErr);
STUB7(AEGetAttributePtr, OSErr(AppleEvent* event, int key, int type,
                        DescType* returned_type, void*, int, Size* actual_size), noErr);
STUB5(AEInstallEventHandler, OSErr(int class_, int action, AEEventHandlerUPP upp,
                            int, bool), noErr);
STUB1(AEProcessAppleEvent, void(EventRecord* event));

STUB1(NewAEEventHandlerProc, AEEventHandlerUPP(AEEventHandlerProcPtr handler), NULL);

#endif // ANTARES_STUB_APPLE_EVENT_H_
