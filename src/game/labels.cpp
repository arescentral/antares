// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "game/labels.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/text.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "video/driver.hpp"

using sfz::Rune;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::quote;
using sfz::scoped_ptr;
using std::min;
using std::max;

namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int32_t kMaxLabelNum = 16;

const int32_t kLabelBuffer = 4;
const int32_t kLabelInnerSpace = 3;
const int32_t kLabelTotalInnerSpace = kLabelInnerSpace << 1;

}  // namespace

// local function prototypes
static long String_Count_Lines(const StringSlice& s);
static StringSlice String_Get_Nth_Line(const StringSlice& source, long nth);
static void Auto_Animate_Line( Point *source, Point *dest);

void ScreenLabelInit() {
    globals()->gScreenLabelData.reset(new screenLabelType[kMaxLabelNum]);
}

void ResetAllLabels() {
    for (int i = 0; i < kMaxLabelNum; ++i) {
        zero(globals()->gScreenLabelData[i]);
    }
}

screenLabelType::screenLabelType() {
    zero(*this);
}

void zero(screenLabelType& label) {
    label.thisRect = Rect(0, 0, -1, -1);
    label.text.clear();
    label.active = false;
    label.killMe = false;
    label.whichObject = kNoShip;
    label.object = NULL;
    label.visible = false;
    label.age = 0;
    label.objectLink = true;
    label.lineNum = 1;
    label.keepOnScreenAnyway = false;
    label.attachedHintLine = false;
    label.retroCount = -1;
}

short AddScreenLabel(
        short h, short v, short hoff, short voff, spaceObjectType* object, bool objectLink,
        unsigned char color) {
    screenLabelType* label = NULL;

    for (int i = 0; i < kMaxLabelNum; ++i) {
        if (!globals()->gScreenLabelData[i].active) {
            label = globals()->gScreenLabelData.get() + i;
            break;
        }
    }
    if (label == NULL) {
        return -1;  // no free label
    }
    int label_num = label - globals()->gScreenLabelData.get();

    label->active = true;
    label->killMe = false;
    label->where = Point(h, v);
    label->offset = Point(hoff, voff);
    label->color = color;
    label->object = object;
    label->objectLink = objectLink;
    label->keepOnScreenAnyway = false;
    label->attachedHintLine = false;
    if (objectLink) {
        if (label->object == NULL) {
            label->visible = false;
        } else {
            label->visible = true;
            label->whichObject = object->entryNumber;
        }
    } else {
        label->visible = true;
    }
    label->text.clear();
    label->lineNum = label->lineHeight = label->width = label->height = 0;

    return label_num;
}

void RemoveScreenLabel(long which) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->thisRect = Rect(0, 0, -1, -1);
    label->text.clear();
    label->active = false;
    label->killMe = false;
    label->object = NULL;
    label->width = label->height = label->lineNum = label->lineHeight = 0;
}

void draw_labels() {
    mSetDirectFont(kTacticalFontNum);

    for (int i = 0; i < kMaxLabelNum; ++i) {
        screenLabelType* const label = globals()->gScreenLabelData.get() + i;

        // We anchor the image at the corner of the rect instead of label->where.  In some cases,
        // label->where is changed between update_all_label_contents() and draw time, but the rect
        // remains unchanged.  Since that function used to do this drawing, the rect's corner is
        // the original location we drew at.
        Point at(label->thisRect.left, label->thisRect.top);

        if (!label->active
                || label->killMe
                || (label->text.empty())
                || !label->visible
                || (label->thisRect.width() <= 0)
                || (label->thisRect.height() <= 0)) {
            continue;
        }
        StringSlice text = label->text;
        if (label->retroCount >= 0) {
            text = text.slice(0, label->retroCount);
        }
        const RgbColor light = GetRGBTranslateColorShade(label->color, VERY_LIGHT);
        const RgbColor dark = GetRGBTranslateColorShade(label->color, VERY_DARK);
        ArrayPixMap pix(label->thisRect.width(), label->thisRect.height());
        pix.fill(RgbColor::kClear);
        DrawNateRectVScan(&pix, pix.size().as_rect(), dark, (at.h ^ at.v) & 0x1);

        scoped_ptr<Sprite> sprite(VideoDriver::driver()->new_sprite(
                    format("/x/screen_label: {0}", quote(text)), pix));
        sprite->draw(at.h, at.v);
        at.offset(kLabelInnerSpace, kLabelInnerSpace + gDirectText->ascent);

        if (label->lineNum > 1) {
            for (int j = 1; j <= label->lineNum; j++) {
                StringSlice line = String_Get_Nth_Line(text, j);

                gDirectText->draw_sprite(Point(at.h + 1, at.v + 1), line, RgbColor::kBlack);
                gDirectText->draw_sprite(Point(at.h - 1, at.v - 1), line, RgbColor::kBlack);
                gDirectText->draw_sprite(at, line, light);

                at.offset(0, label->lineHeight);
            }
        } else {
            gDirectText->draw_sprite(Point(at.h + 1, at.v + 1), text, RgbColor::kBlack);
            gDirectText->draw_sprite(at, text, light);
        }
    }
}

void update_all_label_contents(int32_t units_done) {
    Rect clip = viewport;
    mSetDirectFont(kTacticalFontNum);

    for (int i = 0; i < kMaxLabelNum; ++i) {
        screenLabelType* const label = globals()->gScreenLabelData.get() + i;
        if (!label->active || label->killMe || (label->text.empty()) || !label->visible) {
            label->thisRect.left = label->thisRect.right = 0;
            continue;
        }

        label->thisRect = Rect(0, 0, label->width, label->height);
        label->thisRect.offset(label->where.h, label->where.v);
        label->thisRect.clip_to(clip);
        if ((label->thisRect.width() <= 0) || (label->thisRect.height() <= 0)) {
            continue;
        }

        if (label->retroCount >= 0) {
            // TODO(sfiera): get data on the original rate here.  It looks like the rate of
            // printing was tied to the frame rate before: 3 per frame.  Here, we've switched to 1
            // per tick, so this would be equivalent to the old code at 20 FPS.  The question is,
            // does it feel equivalent?  It only comes up in the tutorial.
            label->retroCount += units_done;
            if (static_cast<size_t>(label->retroCount) > label->text.size()) {
                label->retroCount = -1;
            } else {
                PlayVolumeSound(kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
            }
        }
    }
}

void ShowAllLabels() {
    for (int i = 0; i < kMaxLabelNum; i++) {
        screenLabelType *label = globals()->gScreenLabelData.get() + i;
        if (label->active && label->visible) {
            if (label->killMe) {
                label->active = false;
            }
        }
    }
}

void SetScreenLabelPosition(long which, short h, short v) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->where = label->offset;
    label->where.offset(h, v);
}

void update_all_label_positions(int32_t units_done) {
    const Rect label_limits(
            viewport.left + kLabelBuffer, viewport.top + kLabelBuffer,
            viewport.right - kLabelBuffer, viewport.bottom - kLabelBuffer);

    for (int i = 0; i < kMaxLabelNum; i++) {
        screenLabelType *label = globals()->gScreenLabelData.get() + i;
        bool isOffScreen = false;
        if ((label->active) && (!label->killMe)) {
            if ((label->object != NULL) && (label->object->sprite != NULL)) {
                if (label->object->active) {
                    label->where.h = label->object->sprite->where.h + label->offset.h;

                    if (label->where.h < label_limits.left) {
                        isOffScreen = true;
                        label->where.h = label_limits.left;
                    }

                    if (label->where.h > (label_limits.right - label->width)) {
                        isOffScreen = true;
                        label->where.h = label_limits.right - label->width;
                    }

                    label->where.v = label->object->sprite->where.v + label->offset.v;

                    if (label->where.v < label_limits.top) {
                        isOffScreen = true;
                        label->where.v = label_limits.top;
                    }

                    if (label->where.v > (label_limits.bottom - label->height)) {
                        isOffScreen = true;
                        label->where.v = label_limits.bottom - label->height;
                    }

                    if (!(label->object->seenByPlayerFlags &
                                (1 << globals()->gPlayerAdmiralNumber))) {
                        isOffScreen = true;
                    }

                    if (!label->keepOnScreenAnyway) {
                        if (isOffScreen) {
                            if (label->age == 0) {
                                label->age = -kLabelOffVisibleTime;
                            }
                        } else if (label->age < 0) {
                            label->age = 0;
                            label->visible = true;
                        }
                    }
                    if (label->attachedHintLine && !(label->text.empty())) {
                        Point dest = label->attachedToWhere = label->object->sprite->where;
                        Point source;
                        source.h = label->where.h + (label->width / 4);

                        if (label->attachedToWhere.v < label->where.v) {
                            source.v = label->where.v - 2;
                        } else {
                            source.v = label->where.v + label->height + 2;
                        }
                        Auto_Animate_Line(&source, &dest);
                        ShowHintLine(source, dest, label->color, DARK);
                    }
                } else {
                    SetScreenLabelString(i, "");
                    if (label->attachedHintLine) {
                        HideHintLine();
                    }
                }
            } else if (label->keepOnScreenAnyway) {
                label->where.h = max(label->where.h, label_limits.left);
                label->where.h = min<int32_t>(label->where.h, label_limits.right - label->width);
                label->where.v = max(label->where.v, label_limits.top);
                label->where.v = min<int32_t>(label->where.v, label_limits.bottom - label->height);

                if ((label->attachedHintLine) && !(label->text.empty())) {
                    Point dest = label->attachedToWhere;
                    Point source;
                    source.v = label->where.v + (label->height / 2);
                    if (label->attachedToWhere.h < label->where.h) {
                        source.h = label->where.h - 2;
                    } else {
                        source.h = label->where.h + label->width + 2;
                    }
                    Auto_Animate_Line(&source, &dest);
                    ShowHintLine(source, dest, label->color, VERY_LIGHT);
                }
            }
            if (label->age > 0) {
                label->age -= units_done;
                if (label->age <= 0) {
                    label->visible = false;
                    label->age = 0;
                    label->object = NULL;
                    label->text.clear();
                    if (label->attachedHintLine) {
                        HideHintLine();
                    }
                }
            } else if (label->age < 0) {
                label->age += units_done;
                if (label->age >= 0) {
                    label->age = 0;
                    label->visible = false;
                }
            }
        }
    }
}

void SetScreenLabelObject(long which, spaceObjectType *object) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->object = object;

    if (label->object != NULL) {
        label->age = 0;
        label->visible = true;
        label->whichObject = object->entryNumber;
    } else {
        label->visible = false;
        label->age = 0;
        label->whichObject = kNoShip;
    }
}

void SetScreenLabelAge(long which, long age) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->age = age;
    label->visible = true;
}

void SetScreenLabelString(long which, const StringSlice& string) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->text.assign(string);
    RecalcScreenLabelSize( which);
}

void ClearScreenLabelString(long which) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->text.clear();
    label->width = label->height = 0;
}

void SetScreenLabelColor(long which, unsigned char color) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->color = color;
}

void SetScreenLabelKeepOnScreenAnyway(long which, bool keepOnScreenAnyway) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->keepOnScreenAnyway = keepOnScreenAnyway;
    label->retroCount = 0;
}

void SetScreenLabelAttachedHintLine(long which, bool attachedHintLine, Point toWhere) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    if (label->attachedHintLine) {
        HideHintLine();
    }
    label->attachedHintLine = attachedHintLine;
    label->attachedToWhere = toWhere;
    label->retroCount = 0;
}

void SetScreenLabelOffset(long which, long hoff, long voff) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    label->offset.h = hoff;
    label->offset.v = voff;
}

long GetScreenLabelWidth(long which) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    return label->width;
}

String* GetScreenLabelStringPtr( long which) {
    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    return &label->text;
}

// do this if you mess with its string
void RecalcScreenLabelSize(long which) {
    mSetDirectFont(kTacticalFontNum);

    screenLabelType *label = globals()->gScreenLabelData.get() + which;
    int lineNum = String_Count_Lines(label->text);

    if (lineNum > 1) {
        label->lineNum = lineNum;
        int maxWidth = 0;
        for (int i = 1; i <= lineNum; i++) {
            StringSlice text = String_Get_Nth_Line(label->text, i);
            mGetDirectStringDimensions(text, label->width, label->height);
            label->width += kLabelTotalInnerSpace;
            if (label->width > maxWidth) {
                maxWidth = label->width;
            }
        }
        label->width = maxWidth;
        label->lineHeight = label->height;
        label->height = label->height * lineNum;
        label->height += kLabelTotalInnerSpace;
    } else {
        label->lineNum = 1;
        mGetDirectStringDimensions(label->text, label->width, label->height);
        label->width += kLabelTotalInnerSpace;
        label->lineHeight = label->height;
        label->height += kLabelTotalInnerSpace;
    }
}

static long String_Count_Lines(const StringSlice& s) {
    static const Rune kCarriageReturn = '\r';
    return 1 + std::count(s.begin(), s.end(), kCarriageReturn);
}

static StringSlice String_Get_Nth_Line(const StringSlice& source, long nth) {
    if (nth < 1) {
        return StringSlice();
    }
    const StringSlice::size_type carriage_return = source.find('\r');
    if (carriage_return == StringSlice::npos) {
        if (nth == 1) {
            return source;
        } else {
            return StringSlice();
        }
    }
    if (nth == 1) {
        return source.slice(0, carriage_return);
    } else {
        return String_Get_Nth_Line(source.slice(carriage_return + 1), nth - 1);
    }
}

static void Auto_Animate_Line( Point *source, Point *dest) {
    switch ((usecs_to_ticks(globals()->gGameTime) >> 3) & 0x03) {
        case 0:
            dest->h = source->h + ((dest->h - source->h) >> 2);
            dest->v = source->v + ((dest->v - source->v) >> 2);
            break;

        case 1:
            dest->h = source->h + ((dest->h - source->h) >> 1);
            dest->v = source->v + ((dest->v - source->v) >> 1);
            break;

        case 2:
            dest->h = dest->h + ((source->h - dest->h) >> 2);
            dest->v = dest->v + ((source->v - dest->v) >> 2);
            break;

        case 3:
            break;
    }
}

}  // namespace antares
