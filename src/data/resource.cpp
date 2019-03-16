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
#include "config/preferences.hpp"
#include "data/audio.hpp"
#include "data/base-object.hpp"
#include "data/briefing.hpp"
#include "data/condition.hpp"
#include "data/field.hpp"
#include "data/font-data.hpp"
#include "data/info.hpp"
#include "data/initial.hpp"
#include "data/interface.hpp"
#include "data/level.hpp"
#include "data/races.hpp"
#include "data/replay.hpp"
#include "data/sprite-data.hpp"
#include "drawing/text.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

namespace path = sfz::path;

namespace antares {

namespace {

class ResourceLister : public sfz::TreeWalker {
  public:
    ResourceLister(pn::string_view root, pn::string_view extension, std::vector<pn::string>* names)
            : _root_size(root.size()), _extension(extension), _names(names) {}

    void file(pn::string_view name, const sfz::Stat& st) const override {
        name = name.substr(_root_size + 1);

        const int extension_start = name.size() - _extension.size();
        if ((extension_start <= 0) || (name.substr(extension_start) != _extension)) {
            return;
        }
        name = name.substr(0, extension_start);

        _names->push_back(name.copy());
    }

    // Ignore non-regular-files:
    void pre_directory(pn::string_view name, const sfz::Stat& st) const override {}
    void cycle_directory(pn::string_view name, const sfz::Stat& st) const override {}
    void post_directory(pn::string_view name, const sfz::Stat& st) const override {}
    void symlink(pn::string_view name, const sfz::Stat& st) const override {}
    void broken_symlink(pn::string_view name, const sfz::Stat& st) const override {}
    void other(pn::string_view name, const sfz::Stat& st) const override {}

  private:
    const int                      _root_size;
    const pn::string_view          _extension;
    std::vector<pn::string>* const _names;
};

}  // namespace

static std::vector<pn::string> list_resources(pn::string_view dir, pn::string_view extension) {
    std::vector<pn::string> resources;
    pn::string              path;
    if (sys.prefs->scenario_identifier() == kFactoryScenarioIdentifier) {
        path = pn::format("{0}/{1}", application_path(), dir);
    } else {
        path = pn::format("{0}/{1}", scenario_path(), dir);
    }
    if (sfz::path::isdir(path)) {
        sfz::walk(path, sfz::WALK_PHYSICAL, ResourceLister(path, extension, &resources));
    }
    return resources;
}

std::vector<pn::string> Resource::list_levels() { return list_resources("levels", ".pn"); }
std::vector<pn::string> Resource::list_replays() { return list_resources("replays", ".NLRP"); }

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

static pn::value procyon(pn::string_view path) {
    pn::value  x;
    pn_error_t e;
    if (!pn::parse(load(path)->data().open(), &x, &e)) {
        throw std::runtime_error(
                pn::format("{0}: {1}:{2}: {3}", path, e.lineno, e.column, pn_strerror(e.code))
                        .c_str());
    }
    return x;
}

static bool exists(pn::string_view resource_path) {
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

static Texture load_hidpi_texture(pn::string_view name) {
    int scale = sys.video->scale();
    while (scale) {
        pn::string path;
        if (scale > 1) {
            path = pn::format("{0}@{1}x.png", name, scale);
        } else {
            path = pn::format("{0}.png", name);
        }
        if (!exists(path)) {
            scale >>= 1;
            continue;
        }
        try {
            ArrayPixMap pix = read_png(load(path)->data().open());
            return sys.video->texture(pn::format("/{0}", path), pix, scale);
        } catch (...) {
            std::throw_with_nested(std::runtime_error(path.c_str()));
        }
    }
    throw std::runtime_error(
            pn::format("couldn't find picture {0}", pn::dump(name, pn::dump_short)).c_str());
}

static SoundData load_audio(pn::string_view name) {
    static const struct {
        const char ext[6];
        SoundData (*fn)(pn::data_view);
    } fmts[] = {
            {".aiff", sndfile::convert}, {".s3m", modplug::convert}, {".xm", modplug::convert},
    };

    for (const auto& fmt : fmts) {
        pn::string path = pn::format("{0}{1}", name, fmt.ext);
        if (!exists(path)) {
            continue;
        }
        try {
            return fmt.fn(load(path)->data());
        } catch (...) {
            std::throw_with_nested(std::runtime_error(path.c_str()));
        }
    }
    throw std::runtime_error(
            pn::format("couldn't find sound {0}", pn::dump(name, pn::dump_short)).c_str());
}

bool Resource::object_exists(pn::string_view name) {
    return exists(pn::format("objects/{0}.pn", name));
}

FontData Resource::font(pn::string_view name) {
    pn::string path = pn::format("fonts/{0}.pn", name);
    try {
        return font_data(procyon(path));
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

Texture Resource::font_image(pn::string_view name) {
    return load_hidpi_texture(pn::format("fonts/{0}", name));
}

Info Resource::info() {
    static const char path[] = "info.pn";
    try {
        return ::antares::info(path_value{procyon(path)});
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path));
    }
}

InterfaceData Resource::interface(pn::string_view name) {
    pn::string path = pn::format("interfaces/{0}.pn", name);
    try {
        return ::antares::interface(path_value{procyon(path)});
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

Level Resource::level(pn::string_view name) {
    pn::string path = pn::format("levels/{0}.pn", name);
    try {
        return ::antares::level(procyon(path));
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

SoundData Resource::music(pn::string_view name) {
    return load_audio(pn::format("music/{0}", name));
}

static void merge_value(pn::value_ref base, pn::value_cref patch) {
    switch (patch.type()) {
        case PN_NULL:
        case PN_BOOL:
        case PN_INT:
        case PN_FLOAT:
        case PN_DATA:
        case PN_STRING:
        case PN_ARRAY: base = patch.copy(); return;

        case PN_MAP: break;
    }
    pn::map_ref m = base.to_map();
    for (pn::key_value_cref kv : patch.as_map()) {
        pn::string_view k = kv.key();
        if (m.has(k)) {
            pn::value v = m.get(k).copy();
            merge_value(v, patch.as_map().get(k));
            m.set(k, std::move(v));
        } else {
            m.set(k, kv.value().copy());
        }
    }
}

static pn::value merged_object(pn::string_view name) {
    pn::string path = pn::format("objects/{0}.pn", name);
    try {
        pn::value x = procyon(path);
        pn::value tpl;
        if (!x.is_map() || !x.to_map().pop("template", &tpl) || tpl.is_null()) {
            return x;
        } else if (tpl.is_string()) {
            pn::value base = merged_object(tpl.as_string());
            merge_value(base, x);
            return base;
        } else {
            throw std::runtime_error("template: must be null or string");
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

BaseObject Resource::object(pn::string_view name) {
    pn::value x = merged_object(name);
    try {
        return base_object(x);
    } catch (...) {
        std::throw_with_nested(std::runtime_error(pn::format("objects/{0}.pn", name).c_str()));
    }
}

Race Resource::race(pn::string_view name) {
    pn::string path = pn::format("races/{0}.pn", name);
    try {
        return ::antares::race(path_value{procyon(path)});
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

ReplayData Resource::replay(pn::string_view name) {
    pn::string path = pn::format("replays/{0}.NLRP", name);
    try {
        return ReplayData(load(path)->data());
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

std::vector<int32_t> Resource::rotation_table() {
    const char path[] = "rotation-table";
    try {
        auto                 mapped_file = load(path);
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
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path));
    }
}

std::vector<pn::string> Resource::strings(int id) {
    pn::string path = pn::format("strings/{0}.pn", id);
    try {
        pn::value               x = procyon(path);
        pn::array_cref          l = x.as_array();
        std::vector<pn::string> result;
        for (pn::value_cref x : l) {
            pn::string_view s = x.as_string();
            result.push_back(s.copy());
        }
        return result;
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

SoundData Resource::sound(pn::string_view name) {
    return load_audio(pn::format("sounds/{0}", name));
}

SpriteData Resource::sprite_data(pn::string_view name) {
    pn::string path = pn::format("sprites/{0}.pn", name);
    try {
        return ::antares::sprite_data(procyon(path));
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

ArrayPixMap Resource::sprite_image(pn::string_view name) {
    pn::string path = pn::format("sprites/{0}/image.png", name);
    try {
        return read_png(load(path)->data().open());
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

ArrayPixMap Resource::sprite_overlay(pn::string_view name) {
    pn::string path = pn::format("sprites/{0}/overlay.png", name);
    try {
        return read_png(load(path)->data().open());
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

pn::string Resource::text(int id) {
    pn::string path = pn::format("text/{0}.txt", id);
    try {
        return load(path)->string().copy();
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

Texture Resource::texture(pn::string_view name) {
    return load_hidpi_texture(pn::format("pictures/{0}", name));
}

}  // namespace antares
