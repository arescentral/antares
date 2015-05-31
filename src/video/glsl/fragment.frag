// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015 The Antares Authors
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

#version 120

varying vec2 uv;
varying vec4 color;
varying vec2 screen_position;

uniform int            color_mode;
uniform sampler2DRect  sprite;
uniform sampler2D      static_image;
uniform float          static_fraction;
uniform vec2           unit;
uniform vec4           outline_color;
uniform int            seed;

const int FILL_MODE            = 0;
const int DITHER_MODE          = 1;
const int DRAW_SPRITE_MODE     = 2;
const int TINT_SPRITE_MODE     = 3;
const int STATIC_SPRITE_MODE   = 4;
const int OUTLINE_SPRITE_MODE  = 5;

void main() {
    vec4 sprite_color = texture2DRect(sprite, uv);
    if (color_mode == FILL_MODE) {
        gl_FragColor = color;
    } else if (color_mode == DITHER_MODE) {
        if (mod(floor(screen_position.s) + floor(screen_position.t), 2) == 1) {
            gl_FragColor = color;
        } else {
            gl_FragColor = vec4(0, 0, 0, 0);
        }
    } else if (color_mode == DRAW_SPRITE_MODE) {
        gl_FragColor = sprite_color;
    } else if (color_mode == TINT_SPRITE_MODE) {
        gl_FragColor = color * sprite_color;
    } else if (color_mode == STATIC_SPRITE_MODE) {
        vec2 uv2 = (screen_position + vec2(seed / 256, seed)) * vec2(1.0/256, 1.0/256);
        vec4 static_color = texture2D(static_image, uv2).rrrg;
        if (static_color.w <= static_fraction) {
            vec4 sprite_alpha = vec4(1, 1, 1, sprite_color.w);
            gl_FragColor = color * sprite_alpha;
        } else {
            gl_FragColor = sprite_color;
        }
    } else if (color_mode == OUTLINE_SPRITE_MODE) {
        float neighborhood =
                texture2DRect(sprite, uv + vec2(-unit.s, -unit.t)).w +
                texture2DRect(sprite, uv + vec2(-unit.s,       0)).w +
                texture2DRect(sprite, uv + vec2(-unit.s,  unit.t)).w +
                texture2DRect(sprite, uv + vec2(      0, -unit.t)).w +
                texture2DRect(sprite, uv + vec2(      0,  unit.t)).w +
                texture2DRect(sprite, uv + vec2( unit.s, -unit.t)).w +
                texture2DRect(sprite, uv + vec2( unit.s,       0)).w +
                texture2DRect(sprite, uv + vec2( unit.s,  unit.t)).w;
        if (sprite_color.w > (neighborhood / 8)) {
            gl_FragColor = outline_color;
        } else if (sprite_color.w > 0) {
            gl_FragColor = color;
        } else {
            gl_FragColor = vec4(0, 0, 0, 0);
        }
    }
}
