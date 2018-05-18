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

#ifndef ANTARES_UI_INTERFACE_HANDLING_HPP_
#define ANTARES_UI_INTERFACE_HANDLING_HPP_

#include <vector>

#include "data/handle.hpp"
#include "data/interface.hpp"
#include "drawing/interface.hpp"

namespace antares {

enum PlayAgainResult {
    PLAY_AGAIN_QUIT,
    PLAY_AGAIN_RESTART,
    PLAY_AGAIN_RESUME,
    PLAY_AGAIN_SKIP,
};

bool BothCommandAndQ(void);
void CreateObjectDataText(pn::string& text, const BaseObject& object);
void Replace_KeyCode_Strings_With_Actual_Key_Names(pn::string& text, int16_t resID, size_t padTo);

}  // namespace antares

#endif  // ANTARES_UI_INTERFACE_HANDLING_HPP_
