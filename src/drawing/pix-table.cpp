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

#include "drawing/pix-table.hpp"

#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::Json;
using sfz::JsonDefaultVisitor;
using sfz::ReadSource;
using sfz::String;
using sfz::StringMap;
using sfz::StringSlice;
using sfz::format;
using sfz::range;
using sfz::read;
using sfz::string_to_json;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

struct PixTableVisitor : public JsonDefaultVisitor {
    enum StateEnum {
        NEW,
        ROWS,
        COLS,
        CENTER,
        CENTER_X,
        CENTER_Y,
        FRAMES,
        FRAME,
        FRAME_LEFT,
        FRAME_TOP,
        FRAME_RIGHT,
        FRAME_BOTTOM,
        IMAGE,
        OVERLAY,
        DONE,
    };
    struct State {
        int         rows, cols;
        Point       center;
        Rect        frame;
        StateEnum   state;
        ArrayPixMap image, overlay;
        State() : state(NEW), image(0, 0), overlay(0, 0) {}
    };
    State&                       state;
    int16_t                      id;
    uint8_t                      color;
    vector<NatePixTable::Frame>& frames;

    PixTableVisitor(State& state, int16_t id, uint8_t color, vector<NatePixTable::Frame>& frames)
            : state(state), id(id), color(color), frames(frames) {}

    bool descend(StateEnum state, const StringMap<Json>& value, StringSlice key) const {
        auto it = value.find(key);
        if (it == value.end()) {
            return false;
        }
        StateEnum saved_state = this->state.state;
        this->state.state     = state;
        it->second.accept(*this);
        this->state.state = saved_state;
        return true;
    }

    virtual void visit_object(const StringMap<Json>& value) const {
        switch (state.state) {
            case NEW:
                state.center = Point(0, 0);
                descend(CENTER, value, "center");

                if (!descend(ROWS, value, "rows")) {
                    throw Exception("missing rows in sprite json");
                }
                if (!descend(COLS, value, "cols")) {
                    throw Exception("missing cols in sprite json");
                }
                if (!descend(IMAGE, value, "image")) {
                    throw Exception("missing image in sprite json");
                }

                if (color) {
                    if (!descend(OVERLAY, value, "overlay")) {
                        throw Exception("missing overlay in sprite json");
                    }
                    if (state.image.size() != state.overlay.size()) {
                        throw Exception("size mismatch between image and overlay");
                    }
                }

                if (state.image.size().width % state.cols) {
                    throw Exception("sprite column count does not evenly split image");
                }
                if (state.image.size().height % state.rows) {
                    throw Exception("sprite row count does not evenly split image");
                }

                if (!descend(FRAMES, value, "frames")) {
                    throw Exception("missing frames in sprite json");
                }
                break;

            case CENTER:
                descend(CENTER_X, value, "x");
                descend(CENTER_Y, value, "y");
                break;

            case FRAME:
                if (descend(FRAME_LEFT, value, "left") && descend(FRAME_TOP, value, "top") &&
                    descend(FRAME_RIGHT, value, "right") &&
                    descend(FRAME_BOTTOM, value, "bottom")) {
                    const int  frame       = frames.size();
                    const int  col         = frame % state.cols;
                    const int  row         = frame / state.cols;
                    const int  cell_width  = state.image.size().width / state.cols;
                    const int  cell_height = state.image.size().height / state.rows;
                    const Rect cell(
                            Point(cell_width * col, cell_height * row),
                            Size(cell_width, cell_height));
                    Rect sprite(state.frame);
                    sprite.offset(state.center.h, state.center.v);
                    auto image = state.image.view(cell).view(sprite);
                    Rect bounds(state.frame);
                    bounds.offset(2 * -bounds.left, 2 * -bounds.top);
                    if (color) {
                        auto overlay = state.overlay.view(cell).view(sprite);
                        frames.emplace_back(bounds, image, id, frame, overlay, color);
                    } else {
                        frames.emplace_back(bounds, image, id, frame);
                    }
                } else {
                    throw Exception("bad frame rect");
                }
                break;

            default: return visit_default("object");
        }
    }

    virtual void visit_array(const std::vector<Json>& value) const {
        switch (state.state) {
            case FRAMES:
                if (value.size() != (state.rows * state.cols)) {
                    throw Exception("frame count not equal to rows * cols");
                }
                state.state = FRAME;
                for (const auto& v : value) {
                    v.accept(*this);
                }
                state.state = DONE;
                break;

            default: return visit_default("array");
        }
    }

    void load_image(ArrayPixMap& image, StringSlice path) const {
        Resource   rsrc(path);
        BytesSlice data = rsrc.data();
        read(data, image);
    }

    virtual void visit_string(const StringSlice& value) const {
        switch (state.state) {
            case IMAGE: return load_image(state.image, value);
            case OVERLAY: return load_image(state.overlay, value);
            default: return visit_default("string");
        }
    }

    virtual void visit_number(double value) const {
        switch (state.state) {
            case ROWS: state.rows                 = value; break;
            case COLS: state.cols                 = value; break;
            case CENTER_X: state.center.h         = value; break;
            case CENTER_Y: state.center.v         = value; break;
            case FRAME_LEFT: state.frame.left     = value; break;
            case FRAME_TOP: state.frame.top       = value; break;
            case FRAME_RIGHT: state.frame.right   = value; break;
            case FRAME_BOTTOM: state.frame.bottom = value; break;
            default: return visit_default("number");
        }
    }

    virtual void visit_default(const char* type) const {
        throw Exception(format("unexpected {0} in sprite json", type));
    }
};

}  // namespace

NatePixTable::NatePixTable(int id, uint8_t color) {
    Resource rsrc("sprites", "json", id);
    String   data(utf8::decode(rsrc.data()));
    Json     json;
    if (!string_to_json(data, json)) {
        throw Exception("invalid sprite json");
    }
    PixTableVisitor::State state;
    json.accept(PixTableVisitor(state, id, color, _frames));
}

NatePixTable::~NatePixTable() {}

const NatePixTable::Frame& NatePixTable::at(size_t index) const {
    return _frames[index];
}

size_t NatePixTable::size() const {
    return _size;
}

NatePixTable::Frame::Frame(
        Rect bounds, const PixMap& image, int16_t id, int frame, const PixMap& overlay,
        uint8_t color)
        : _bounds(bounds), _pix_map(bounds.width(), bounds.height()) {
    load_image(image);
    load_overlay(overlay, color);
    build(id, frame);
}

NatePixTable::Frame::Frame(Rect bounds, const PixMap& image, int16_t id, int frame)
        : _bounds(bounds), _pix_map(bounds.width(), bounds.height()) {
    load_image(image);
    build(id, frame);
}

NatePixTable::Frame::~Frame() {}

void NatePixTable::Frame::load_image(const PixMap& pix) {
    _pix_map.copy(pix);
}

void NatePixTable::Frame::load_overlay(const PixMap& pix, uint8_t color) {
    for (auto x : range(width())) {
        for (auto y : range(height())) {
            RgbColor over  = pix.get(x, y);
            uint8_t  value = over.red;
            uint8_t  frac  = over.alpha;
            over           = RgbColor::tint(color, value);
            RgbColor under = _pix_map.get(x, y);
            RgbColor composite;
            composite.red   = ((over.red * frac) + (under.red * (255 - frac))) / 255;
            composite.green = ((over.green * frac) + (under.green * (255 - frac))) / 255;
            composite.blue  = ((over.blue * frac) + (under.blue * (255 - frac))) / 255;
            composite.alpha = under.alpha;
            _pix_map.set(x, y, composite);
        }
    }
}

uint16_t NatePixTable::Frame::width() const {
    return _bounds.width();
}
uint16_t NatePixTable::Frame::height() const {
    return _bounds.height();
}
Point NatePixTable::Frame::center() const {
    return _bounds.origin();
}
const PixMap& NatePixTable::Frame::pix_map() const {
    return _pix_map;
}
const Texture& NatePixTable::Frame::texture() const {
    return _texture;
}

void NatePixTable::Frame::build(int16_t id, int frame) {
    _texture = sys.video->texture(format("/sprites/{0}.SMIV/{1}", id, frame), _pix_map);
}

}  // namespace antares
