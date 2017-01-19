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

#version 330 core

in vec2 uv;
in vec4 color;
in vec2 screen_position;

out vec4 frag_color;

uniform int scale;
uniform int color_mode;
uniform sampler2DRect sprite;
uniform sampler2D static_image;
uniform float     static_fraction;
uniform vec2 unit;
uniform vec4 outline_color;
uniform int  seed;

const int FILL_MODE           = 0;
const int DITHER_MODE         = 1;
const int DRAW_SPRITE_MODE    = 2;
const int TINT_SPRITE_MODE    = 3;
const int STATIC_SPRITE_MODE  = 4;
const int OUTLINE_SPRITE_MODE = 5;

void main() {
    vec4 sprite_color = texture(sprite, uv);
    if (color_mode == FILL_MODE) {
        frag_color = color;
    } else if (color_mode == DITHER_MODE) {
        frag_color = color;
        frag_color.a /= 2;
    } else if (color_mode == DRAW_SPRITE_MODE) {
        frag_color = sprite_color;
    } else if (color_mode == TINT_SPRITE_MODE) {
        frag_color = color * sprite_color;
    } else if (color_mode == STATIC_SPRITE_MODE) {
        float f            = scale / 256.0;
        vec2  uv2          = (screen_position + vec2(seed * f, seed)) * vec2(f, f);
        vec4  static_color = texture(static_image, uv2).rrrg;
        if (static_color.w <= static_fraction) {
            vec4 sprite_alpha = vec4(1, 1, 1, sprite_color.w);
            frag_color        = color * sprite_alpha;
        } else {
            frag_color = sprite_color;
        }
    } else if (color_mode == OUTLINE_SPRITE_MODE) {
        float neighborhood = texture(sprite, uv + vec2(-unit.s, -unit.t)).w +
                             texture(sprite, uv + vec2(-unit.s, 0)).w +
                             texture(sprite, uv + vec2(-unit.s, unit.t)).w +
                             texture(sprite, uv + vec2(0, -unit.t)).w +
                             texture(sprite, uv + vec2(0, unit.t)).w +
                             texture(sprite, uv + vec2(unit.s, -unit.t)).w +
                             texture(sprite, uv + vec2(unit.s, 0)).w +
                             texture(sprite, uv + vec2(unit.s, unit.t)).w;
        if (sprite_color.w > (neighborhood / 8)) {
            frag_color = outline_color;
        } else if (sprite_color.w > 0) {
            frag_color = color;
        } else {
            frag_color = vec4(0, 0, 0, 0);
        }
    }
}
