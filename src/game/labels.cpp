// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include "game/labels.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "video/driver.hpp"

using sfz::Rune;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::quote;
using std::max;
using std::min;
using std::unique_ptr;

namespace antares {

namespace {

const int32_t kLabelBuffer = 4;
const int32_t kLabelInnerSpace = 3;
const int32_t kLabelTotalInnerSpace = kLabelInnerSpace << 1;

}  // namespace

// local function prototypes
static int32_t String_Count_Lines(const StringSlice& s);
static StringSlice String_Get_Nth_Line(const StringSlice& source, int32_t nth);
static void Auto_Animate_Line( Point *source, Point *dest);

static unique_ptr<Label[]> data;

Label* Label::get(int number) {
    if ((0 <= number) && (number < kMaxLabelNum)) {
        return &data[number];
    }
    return nullptr;
}

void Label::init() {
    data.reset(new Label[kMaxLabelNum]);
}

void Label::reset() {
    for (auto label: all()) {
        *label = Label();
    }
}

Handle<Label> Label::next_free_label() {
    for (auto label: all()) {
        if (!label->active) {
            return label;
        }
    }
    return Label::none();
}

Handle<Label> Label::add(
        int16_t h, int16_t v, int16_t hoff, int16_t voff, Handle<SpaceObject> object,
        bool objectLink, uint8_t color) {
    auto label = next_free_label();
    if (!label.get()) {
        return Label::none();  // no free label
    }

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
        label->visible = bool(label->object.get());
    } else {
        label->visible = true;
    }
    label->text.clear();
    label->lineNum = label->lineHeight = label->width = label->height = 0;

    return label;
}

void Label::remove(Handle<Label> label) {
    label->thisRect = Rect(0, 0, -1, -1);
    label->text.clear();
    label->active = false;
    label->killMe = false;
    label->object = SpaceObject::none();
    label->width = label->height = label->lineNum = label->lineHeight = 0;
}

void Label::draw() {
    for (auto label: all()) {
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
        VideoDriver::driver()->dither_rect(label->thisRect, dark);
        at.offset(kLabelInnerSpace, kLabelInnerSpace + tactical_font->ascent);

        if (label->lineNum > 1) {
            for (int j = 1; j <= label->lineNum; j++) {
                StringSlice line = String_Get_Nth_Line(text, j);

                tactical_font->draw_sprite(Point(at.h + 1, at.v + 1), line, RgbColor::kBlack);
                tactical_font->draw_sprite(Point(at.h - 1, at.v - 1), line, RgbColor::kBlack);
                tactical_font->draw_sprite(at, line, light);

                at.offset(0, label->lineHeight);
            }
        } else {
            tactical_font->draw_sprite(Point(at.h + 1, at.v + 1), text, RgbColor::kBlack);
            tactical_font->draw_sprite(at, text, light);
        }
    }
}

void Label::update_contents(int32_t units_done) {
    Rect clip = viewport;
    for (auto label: all()) {
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

void Label::show_all() {
    for (auto label: all()) {
        if (label->active && label->visible) {
            if (label->killMe) {
                label->active = false;
            }
        }
    }
}

void Label::set_position(Handle<Label> label, int16_t h, int16_t v) {
    label->where = label->offset;
    label->where.offset(h, v);
}

void Label::update_positions(int32_t units_done) {
    const Rect label_limits(
            viewport.left + kLabelBuffer, viewport.top + kLabelBuffer,
            viewport.right - kLabelBuffer, viewport.bottom - kLabelBuffer);

    for (auto label: all()) {
        bool isOffScreen = false;
        if ((label->active) && (!label->killMe)) {
            if (label->object.get() && (label->object->sprite != NULL)) {
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
                                (1 << globals()->gPlayerAdmiral.number()))) {
                        isOffScreen = true;
                    }

                    if (!label->keepOnScreenAnyway) {
                        if (isOffScreen) {
                            if (label->age == 0) {
                                label->age = -kVisibleTime;
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
                        HintLine::show(source, dest, label->color, DARK);
                    }
                } else {
                    Label::set_string(label, "");
                    if (label->attachedHintLine) {
                        HintLine::hide();
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
                    HintLine::show(source, dest, label->color, VERY_LIGHT);
                }
            }
            if (label->age > 0) {
                label->age -= units_done;
                if (label->age <= 0) {
                    label->visible = false;
                    label->age = 0;
                    label->object = SpaceObject::none();
                    label->text.clear();
                    if (label->attachedHintLine) {
                        HintLine::hide();
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

void Label::set_object(Handle<Label> label, Handle<SpaceObject> object) {
    label->object = object;
    label->visible = bool(object.get());
    label->age = 0;
}

void Label::set_age(Handle<Label> label, int32_t age) {
    label->age = age;
    label->visible = true;
}

void Label::set_string(Handle<Label> label, const StringSlice& string) {
    label->text.assign(string);
    Label::recalc_size(label);
}

void Label::clear_string(Handle<Label> label) {
    label->text.clear();
    label->width = label->height = 0;
}

void Label::set_color(Handle<Label> label, uint8_t color) {
    label->color = color;
}

void Label::set_keep_on_screen_anyway(Handle<Label> label, bool keepOnScreenAnyway) {
    label->keepOnScreenAnyway = keepOnScreenAnyway;
    label->retroCount = 0;
}

void Label::set_attached_hint_line(Handle<Label> label, bool attachedHintLine, Point toWhere) {
    if (label->attachedHintLine) {
        HintLine::hide();
    }
    label->attachedHintLine = attachedHintLine;
    label->attachedToWhere = toWhere;
    label->retroCount = 0;
}

void Label::set_offset(Handle<Label> label, int32_t hoff, int32_t voff) {
    label->offset.h = hoff;
    label->offset.v = voff;
}

int32_t Label::get_width(Handle<Label> label) {
    return label->width;
}

String* Label::get_string(Handle<Label> label) {
    return &label->text;
}

// do this if you mess with its string
void Label::recalc_size(Handle<Label> label) {
    int lineNum = String_Count_Lines(label->text);

    if (lineNum > 1) {
        label->lineNum = lineNum;
        int maxWidth = 0;
        for (int i = 1; i <= lineNum; i++) {
            StringSlice text = String_Get_Nth_Line(label->text, i);
            int32_t width = tactical_font->string_width(text);
            if (width > maxWidth) {
                maxWidth = width;
            }
        }
        label->width = maxWidth + kLabelTotalInnerSpace;
        label->height = (tactical_font->height * lineNum) + kLabelTotalInnerSpace;
        label->lineHeight = tactical_font->height;
    } else {
        label->lineNum = 1;
        label->width = tactical_font->string_width(label->text) + kLabelTotalInnerSpace;
        label->height = tactical_font->height + kLabelTotalInnerSpace;
        label->lineHeight = tactical_font->height;
    }
}

static int32_t String_Count_Lines(const StringSlice& s) {
    static const Rune kCarriageReturn = '\n';
    return 1 + std::count(s.begin(), s.end(), kCarriageReturn);
}

static StringSlice String_Get_Nth_Line(const StringSlice& source, int32_t nth) {
    if (nth < 1) {
        return StringSlice();
    }
    const StringSlice::size_type carriage_return = source.find('\n');
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
