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

#include "data/string-list.hpp"

#include <sfz/sfz.hpp>

#include "data/resource.hpp"

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Exception;
using sfz::Json;
using sfz::JsonDefaultVisitor;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::quote;
using sfz::read;
using sfz::string_to_json;
using std::vector;

namespace macroman = sfz::macroman;
namespace utf8 = sfz::utf8;

namespace antares {

namespace {

struct StringListVisitor : public JsonDefaultVisitor {
    enum State {
        NEW,
        ARRAY,
        DONE,
    };

    StringListVisitor(State& state, vector<String*>& vec):
            _state(state),
            _vec(vec) {
        _state = NEW;
    }
    virtual void visit_array(const std::vector<Json>& value) const {
        switch (_state) {
          case NEW:
            _state = ARRAY;
            for (size_t i = 0; i < value.size(); ++i) {
                value[i].accept(*this);
            }
            _state = DONE;
            break;
          default:
            return JsonDefaultVisitor::visit_array(value);
        }
    }
    virtual void visit_string(const StringSlice& value) const {
        switch (_state) {
          case ARRAY:
            _vec.push_back(new String(value));
            break;
          default:
            return JsonDefaultVisitor::visit_string(value);
        }
    }
    virtual void visit_default(const char* type) const {
        throw Exception(format("got unexpected JSON {0}", type));
    }

    State& _state;
    vector<String*>& _vec;
};

}  // namespace

StringList::StringList(int id) {
    Resource rsrc("strings", "json", id);
    String in(utf8::decode(rsrc.data()));
    Json strings;
    if (!string_to_json(in, strings)) {
        throw Exception(sfz::format("Couldn't parse strings/{0}.json", id));
    }
    StringListVisitor::State state;
    strings.accept(StringListVisitor(state, _strings));
}

StringList::~StringList() {
    clear();
}

void StringList::clear() {
    // TODO(sfiera): make exception-safe.
    for (String* string: _strings) {
        delete string;
    }
    _strings.clear();
}

ssize_t StringList::index_of(const StringSlice& result) const {
    for (size_t i = 0; i < size(); ++i) {
        if (at(i) == result) {
            return i;
        }
    }
    return -1;
}

size_t StringList::size() const {
    return _strings.size();
}

const String& StringList::at(size_t index) const {
    return *_strings.at(index);
}

void string_to_pstring(const String& src, unsigned char* dst) {
    Bytes src_bytes(macroman::encode(src));
    if (src_bytes.size() > 254) {
        throw Exception(format("{0} is too long to convert to a pstring", quote(src)));
    }
    *dst = src_bytes.size();
    memcpy(dst + 1, src_bytes.data(), src_bytes.size());
}

}  // namespace antares
