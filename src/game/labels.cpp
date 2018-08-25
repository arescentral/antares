// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/space-object.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"
#include "video/driver.hpp"

using std::max;
using std::min;
using std::unique_ptr;

namespace antares {

namespace {

const int32_t kLabelBuffer          = 4;
const int32_t kLabelInnerSpace      = 3;
const int32_t kLabelTotalInnerSpace = kLabelInnerSpace << 1;

}  // namespace

const ticks Label::kVisibleTime = secs(1);

// local function prototypes
static int32_t         String_Count_Lines(pn::string_view s);
static pn::string_view String_Get_Nth_Line(pn::string_view source, int32_t nth);
static void            Auto_Animate_Line(Point* source, Point* dest);

Label* Label::get(int number) {
    if ((0 <= number) && (number < kMaxLabelNum)) {
        return &g.labels[number];
    }
    return nullptr;
}

void Label::init() { g.labels.reset(new Label[kMaxLabelNum]); }

void Label::reset() {
    for (auto label : all()) {
        *label = Label();
    }
}

Handle<Label> Label::next_free_label() {
    for (auto label : all()) {
        if (!label->active) {
            return label;
        }
    }
    return Label::none();
}

Handle<Label> Label::add(
        int16_t h, int16_t v, int16_t hoff, int16_t voff, Handle<SpaceObject> object,
        bool objectLink, Hue hue) {
    auto label = next_free_label();
    if (!label.get()) {
        return Label::none();  // no free label
    }

    label->active             = true;
    label->killMe             = false;
    label->where              = Point(h, v);
    label->offset             = Point(hoff, voff);
    label->hue                = hue;
    label->object             = object;
    label->objectLink         = objectLink;
    label->keepOnScreenAnyway = false;
    label->attachedHintLine   = false;
    if (objectLink) {
        label->visible = bool(label->object.get());
    } else {
        label->visible = true;
    }
    label->text.clear();
    label->lineNum = label->lineHeight = label->width = label->height = 0;

    return label;
}

void Label::remove() {
    thisRect = Rect(0, 0, -1, -1);
    text.clear();
    active = false;
    killMe = false;
    object = SpaceObject::none();
    width = height = lineNum = lineHeight = 0;
}

void Label::draw() {
    for (auto label : all()) {
        // We anchor the image at the corner of the rect instead of label->where.  In some cases,
        // label->where is changed between update_all_label_contents() and draw time, but the rect
        // remains unchanged.  Since that function used to do this drawing, the rect's corner is
        // the original location we drew at.
        Point at(label->thisRect.left, label->thisRect.top);

        if (!label->active || label->killMe || (label->text.empty()) || !label->visible ||
            (label->thisRect.width() <= 0) || (label->thisRect.height() <= 0)) {
            continue;
        }
        pn::string_view text = label->text;
        if ((0 <= label->retroCount) && (label->retroCount < text.size())) {
            text = text.substr(0, label->retroCount);
        }
        const RgbColor light = GetRGBTranslateColorShade(label->hue, LIGHTEST);
        const RgbColor dark  = GetRGBTranslateColorShade(label->hue, VERY_DARK);
        sys.video->dither_rect(label->thisRect, dark);
        at.offset(kLabelInnerSpace, kLabelInnerSpace + sys.fonts.tactical.ascent);

        if (label->lineNum > 1) {
            for (int j = 1; j <= label->lineNum; j++) {
                pn::string_view line = String_Get_Nth_Line(text, j);

                sys.fonts.tactical.draw(Point(at.h + 1, at.v + 1), line, RgbColor::black());
                sys.fonts.tactical.draw(Point(at.h - 1, at.v - 1), line, RgbColor::black());
                sys.fonts.tactical.draw(at, line, light);

                at.offset(0, label->lineHeight);
            }
        } else {
            sys.fonts.tactical.draw(Point(at.h + 1, at.v + 1), text, RgbColor::black());
            sys.fonts.tactical.draw(at, text, light);
        }
    }
}

void Label::update_contents(ticks units_done) {
    Rect clip = viewport();
    for (auto label : all()) {
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
            pn::string_view::iterator it{label->text.data(), label->text.size(),
                                         label->retroCount};
            for (size_t i = 0; i < units_done.count(); ++i) {
                ++it;  // Increment rune, not byte.
            }
            label->retroCount = it.offset();
            if (static_cast<size_t>(label->retroCount) >= label->text.size()) {
                label->retroCount = -1;
            } else {
                sys.sound.teletype();
            }
        }
    }
}

void Label::show_all() {
    for (auto label : all()) {
        if (label->active && label->visible) {
            if (label->killMe) {
                label->active = false;
            }
        }
    }
}

void Label::set_position(int16_t h, int16_t v) {
    where = offset;
    where.offset(h, v);
}

void Label::update_positions(ticks units_done) {
    const Rect label_limits(
            viewport().left + kLabelBuffer, viewport().top + kLabelBuffer,
            viewport().right - kLabelBuffer, viewport().bottom - kLabelBuffer);

    for (auto label : all()) {
        bool isOffScreen = false;
        if ((label->active) && (!label->killMe)) {
            if (label->object.get() && label->object->sprite.get()) {
                if (label->object->active) {
                    label->where.h = label->object->sprite->where.h + label->offset.h;

                    if (label->where.h < label_limits.left) {
                        isOffScreen    = true;
                        label->where.h = label_limits.left;
                    }

                    if (label->where.h > (label_limits.right - label->width)) {
                        isOffScreen    = true;
                        label->where.h = label_limits.right - label->width;
                    }

                    label->where.v = label->object->sprite->where.v + label->offset.v;

                    if (label->where.v < label_limits.top) {
                        isOffScreen    = true;
                        label->where.v = label_limits.top;
                    }

                    if (label->where.v > (label_limits.bottom - label->height)) {
                        isOffScreen    = true;
                        label->where.v = label_limits.bottom - label->height;
                    }

                    if (!(label->object->seenByPlayerFlags & (1 << g.admiral.number()))) {
                        isOffScreen = true;
                    }

                    if (!label->keepOnScreenAnyway) {
                        if (isOffScreen) {
                            if (label->age == ticks(0)) {
                                label->age = -kVisibleTime;
                            }
                        } else if (label->age < ticks(0)) {
                            label->age     = ticks(0);
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
                        HintLine::show(source, dest, label->hue, DARK);
                    }
                } else {
                    label->set_string("");
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
                    HintLine::show(source, dest, label->hue, LIGHTEST);
                }
            }
            if (label->age > ticks(0)) {
                label->age -= units_done;
                if (label->age <= ticks(0)) {
                    label->visible = false;
                    label->age     = ticks(0);
                    label->object  = SpaceObject::none();
                    label->text.clear();
                    if (label->attachedHintLine) {
                        HintLine::hide();
                    }
                }
            } else if (label->age < ticks(0)) {
                label->age += units_done;
                if (label->age >= ticks(0)) {
                    label->age     = ticks(0);
                    label->visible = false;
                }
            }
        }
    }
}

void Label::set_object(Handle<SpaceObject> object) {
    this->object = object;
    visible      = bool(object.get());
    age          = ticks(0);
}

void Label::set_age(ticks age) {
    this->age = age;
    visible   = true;
}

void Label::set_string(pn::string_view string) {
    text = string.copy();
    recalc_size();
}

void Label::clear_string() {
    text.clear();
    width = height = 0;
}

void Label::set_hue(Hue hue) { this->hue = hue; }

void Label::set_keep_on_screen_anyway(bool keepOnScreenAnyway) {
    this->keepOnScreenAnyway = keepOnScreenAnyway;
    retroCount               = 0;
}

void Label::set_attached_hint_line(bool attachedHintLine, Point toWhere) {
    if (attachedHintLine) {
        HintLine::hide();
    }
    this->attachedHintLine = attachedHintLine;
    attachedToWhere        = toWhere;
    retroCount             = 0;
}

void Label::set_offset(int32_t hoff, int32_t voff) {
    offset.h = hoff;
    offset.v = voff;
}

// do this if you mess with its string
void Label::recalc_size() {
    int lineNum = String_Count_Lines(text);

    if (lineNum > 1) {
        this->lineNum = lineNum;
        int maxWidth  = 0;
        for (int i = 1; i <= lineNum; i++) {
            pn::string_view text  = String_Get_Nth_Line(this->text, i);
            int32_t         width = sys.fonts.tactical.string_width(text);
            if (width > maxWidth) {
                maxWidth = width;
            }
        }
        width      = maxWidth + kLabelTotalInnerSpace;
        height     = (sys.fonts.tactical.height * lineNum) + kLabelTotalInnerSpace;
        lineHeight = sys.fonts.tactical.height;
    } else {
        width      = sys.fonts.tactical.string_width(text) + kLabelTotalInnerSpace;
        height     = sys.fonts.tactical.height + kLabelTotalInnerSpace;
        lineHeight = sys.fonts.tactical.height;
    }
}

static int32_t String_Count_Lines(pn::string_view s) {
    bool trailing_nl = false;
    int  count       = 0;
    for (pn::rune r : s) {
        if (r == pn::rune{'\n'}) {
            ++count;
            trailing_nl = true;
        } else {
            trailing_nl = false;
        }
    }
    if (trailing_nl) {
        return count;
    } else {
        return count + 1;
    }
}

static pn::string_view String_Get_Nth_Line(pn::string_view source, int32_t nth) {
    if (nth < 1) {
        return pn::string_view{};
    }
    pn::string_view::size_type carriage_return = source.find(pn::rune{'\n'});
    if (carriage_return == pn::string_view::npos) {
        if (nth == 1) {
            return source;
        } else {
            return pn::string_view{};
        }
    }
    if (nth == 1) {
        return source.substr(0, carriage_return);
    } else {
        return String_Get_Nth_Line(source.substr(carriage_return + 1), nth - 1);
    }
}

static void Auto_Animate_Line(Point* source, Point* dest) {
    switch ((std::chrono::time_point_cast<ticks>(g.time).time_since_epoch().count() >> 3) & 0x03) {
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

        case 3: break;
    }
}

}  // namespace antares
