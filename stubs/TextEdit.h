#ifndef ANTARES_STUB_AIFF_H_
#define ANTARES_STUB_AIFF_H_

#include <Base.h>

namespace antares {

struct TextEdit {
    int teLength;
    Handle hText;
};
typedef TextEdit* TEPtr;
typedef TextEdit** TEHandle;

STUB0(TEInit, void());
STUB2(TENew, TEHandle(const Rect&, const Rect&), NULL);
STUB1(TEDispose, void(TEHandle data));
STUB1(TEIdle, void(TEHandle data));
STUB3(TEClick, void(Point where, bool shift, TEHandle data));
STUB2(TEKey, void(char whichChar, TEHandle data));
STUB3(TESetText, void(Ptr, long, TEHandle data));
STUB3(TESetSelect, void(int start, int end, TEHandle data));
STUB1(TEActivate, void(TEHandle data));
STUB1(TEDeactivate, void(TEHandle data));
STUB2(TEUpdate, void(const Rect&, TEHandle data));

}  // namespace antares

#endif // ANTARES_STUB_AIFF_H_
