#ifndef ANTARES_STUB_AIFF_H_
#define ANTARES_STUB_AIFF_H_

#include <Base.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct TextEdit {
    int teLength;
    Handle hText;
};
typedef TextEdit* TEPtr;
typedef TextEdit** TEHandle;

void TEInit();
TEHandle TENew(Rect*, Rect*);
void TEDispose(TEHandle data);
void TEIdle(TEHandle data);
void TEClick(Point where, bool shift, TEHandle data);
void TEKey(char whichChar, TEHandle data);
void TESetText(Ptr, long, TEHandle data);
void TESetSelect(int start, int end, TEHandle data);
void TEActivate(TEHandle data);
void TEDeactivate(TEHandle data);
void TEUpdate(Rect*, TEHandle data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_AIFF_H_
