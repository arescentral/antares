// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#if __VERSION__ == 330
#elif __VERSION__ == 110
# define in attribute
# define out varying
#else
# error
#endif

in vec4 vertex;
in vec4 in_color;
in vec2 tex_coord;

out vec2 uv;
out vec4 color;
out vec2 screen_position;

uniform vec2 screen;

void main() {
    mat4 transform =
            mat4(2.0 / screen.x, 0, 0, 0, 0, -2.0 / screen.y, 0, 0, 0, 0, 0, 0, -1.0, 1.0, 0, 1);

    gl_Position     = transform * vertex;
    uv              = tex_coord;
    screen_position = vertex.xy;
    color           = in_color;
}
