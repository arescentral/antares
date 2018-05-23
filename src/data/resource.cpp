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

#include "data/resource.hpp"

#include <stdio.h>
#include <array>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "data/audio.hpp"
#include "data/field.hpp"
#include "data/font-data.hpp"
#include "data/interface.hpp"
#include "data/replay.hpp"
#include "data/sprite-data.hpp"
#include "drawing/text.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

namespace path = sfz::path;

namespace antares {

static std::unique_ptr<sfz::mapped_file> load(pn::string_view resource_path) {
    pn::string      scenario = scenario_path();
    pn::string_view factory  = factory_scenario_path();
    pn::string_view app      = application_path();
    for (const auto& dir : std::array<pn::string_view, 3>{{scenario, factory, app}}) {
        pn::string path = pn::format("{0}/{1}", dir, resource_path);
        if (path::isfile(path)) {
            return std::unique_ptr<sfz::mapped_file>(new sfz::mapped_file(path));
        }
    }
    throw std::runtime_error(
            pn::format("couldn't find resource {0}", pn::dump(resource_path, pn::dump_short))
                    .c_str());
}

bool Resource::exists(pn::string_view resource_path) {
    pn::string      scenario = scenario_path();
    pn::string_view factory  = factory_scenario_path();
    pn::string_view app      = application_path();
    for (const auto& dir : std::array<pn::string_view, 3>{{scenario, factory, app}}) {
        pn::string path = pn::format("{0}/{1}", dir, resource_path);
        if (path::isfile(path)) {
            return true;
        }
    }
    return false;
}

static Texture load_png(pn::string_view path, int scale) {
    ArrayPixMap pix = read_png(load(pn::format("{0}.png", path))->data().open());
    return sys.video->texture(pn::format("/{0}.png", path), pix, scale);
}

static Texture load_hidpi_texture(pn::string_view name) {
    int scale = sys.video->scale();
    while (true) {
        try {
            pn::string path = name.copy();
            if (scale > 1) {
                return load_png(pn::format("{0}@{1}x", name, scale), scale);
            } else {
                return load_png(name, scale);
            }
        } catch (...) {
            if (scale > 1) {
                scale >>= 1;
            } else {
                throw;
            }
        }
    }
}

static SoundData load_audio(pn::string_view name) {
    static const struct {
        const char ext[6];
        SoundData (*fn)(pn::data_view);
    } fmts[] = {
            {".aiff", sndfile::convert}, {".s3m", modplug::convert}, {".xm", modplug::convert},
    };

    for (const auto& fmt : fmts) {
        pn::string path_ext = pn::format("{0}{1}", name, fmt.ext);
        if (!Resource::exists(path_ext)) {
            continue;
        }
        return fmt.fn(load(path_ext)->data());
    }
    throw std::runtime_error(
            pn::format("couldn't find sound {0}", pn::dump(name, pn::dump_short)).c_str());
}

FontData Resource::font(pn::string_view name) {
    return font_data(procyon(pn::format("fonts/{0}.pn", name)));
}

Texture Resource::font_image(pn::string_view name) {
    return load_hidpi_texture(pn::format("fonts/{0}", name));
}

std::vector<std::unique_ptr<InterfaceItem>> Resource::interface(pn::string_view name) {
    return interface_items(0, path_value{procyon(pn::format("interfaces/{0}.pn", name))});
}

SoundData Resource::music(pn::string_view name) {
    return load_audio(pn::format("music/{0}", name));
}

ReplayData Resource::replay(int id) {
    return ReplayData(load(pn::format("replays/{0}.NLRP", id))->data());
}

std::vector<int32_t> Resource::rotation_table() {
    auto                 mapped_file = load("rotation-table");
    pn::file             in          = mapped_file->data().open();
    std::vector<int32_t> v;
    v.resize(SystemGlobals::ROT_TABLE_SIZE);
    for (int32_t& i : v) {
        in.read(&i).check();
    }
    if (!in.read(pn::pad(1)).eof()) {
        throw std::runtime_error("didn't consume all of rotation data");
    }
    return v;
}

std::vector<pn::string> Resource::strings(int id) {
    pn::value strings;
    if (!pn::parse(load(pn::format("strings/{0}.pn", id))->data().open(), strings, nullptr)) {
        throw std::runtime_error(pn::format("Couldn't parse strings/{0}.pn", id).c_str());
    }
    pn::array_cref          l = strings.as_array();
    std::vector<pn::string> result;
    for (pn::value_cref x : l) {
        pn::string_view s = x.as_string();
        result.push_back(s.copy());
    }
    return result;
}

SoundData Resource::sound(pn::string_view name) {
    return load_audio(pn::format("sounds/{0}", name));
}

SpriteData Resource::sprite_data(pn::string_view name) {
    return ::antares::sprite_data(procyon(pn::format("sprites/{0}.pn", name)));
}

ArrayPixMap Resource::sprite_image(pn::string_view name) {
    return read_png(load(pn::format("sprites/{0}/image.png", name))->data().open());
}

ArrayPixMap Resource::sprite_overlay(pn::string_view name) {
    return read_png(load(pn::format("sprites/{0}/overlay.png", name))->data().open());
}

pn::string Resource::text(int id) { return load(pn::format("text/{0}.txt", id))->string().copy(); }

Texture Resource::texture(pn::string_view name) {
    return load_hidpi_texture(pn::format("pictures/{0}", name));
}

pn::value Resource::procyon(pn::string_view path) {
    pn::value  x;
    pn_error_t e;
    if (!pn::parse(load(path)->data().open(), x, &e)) {
        throw std::runtime_error(
                pn::format("{0}: {1}:{2}: {3}", path, e.lineno, e.column, pn_strerror(e.code))
                        .c_str());
    }
    return x;
}

}  // namespace antares
