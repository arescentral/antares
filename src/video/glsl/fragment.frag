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
out vec4 frag_color;
# define texture2D texture
# define texture2DRect texture
#elif __VERSION__ == 110
# define in varying
# define frag_color gl_FragColor
#else
# error
#endif

in vec2 uv;
in vec4 color;
in vec2 screen_position;

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

vec3 pow3(vec3 v, float exp) {
    return vec3(pow(v.x, exp), pow(v.y, exp), pow(v.z, exp));
}

vec3 apple_rgb_to_srgb(vec3 apple_rgb_color) {
    vec3 linear_apple_rgb_color = pow3(max(apple_rgb_color, vec3(0)), 1.8);

    // Convert from Apple RGB linear to sRGB linear
    vec3 srgb_linear_color;
    srgb_linear_color = linear_apple_rgb_color.r * vec3(1.06870538834699, 0.024110476735, 0.00173499822713);
    srgb_linear_color += linear_apple_rgb_color.g * vec3(-0.07859532843279, 0.96007030899244, 0.02974755969275);
    srgb_linear_color += linear_apple_rgb_color.b * vec3(0.00988984558395, 0.01581936633364, 0.96851741859153);
    srgb_linear_color = max(srgb_linear_color, vec3(0));

    // Convert to sRGB gamma
    vec3 linear_section = min(12.92 * srgb_linear_color, vec3(0.040449936));
    vec3 exp_section = 1.055 * pow3(srgb_linear_color, 1.0 / 2.4) - 0.055;

    return min(vec3(1), max(linear_section, exp_section));
}

void main() {
    vec4 sprite_color = texture2DRect(sprite, uv);
    if (color_mode == FILL_MODE) {
        frag_color = color;
    } else if (color_mode == DITHER_MODE) {
        frag_color = color;
        frag_color.a /= 2.0;
    } else if (color_mode == DRAW_SPRITE_MODE) {
        frag_color = sprite_color;
    } else if (color_mode == TINT_SPRITE_MODE) {
        frag_color = color * sprite_color;
    } else if (color_mode == STATIC_SPRITE_MODE) {
        float f            = float(scale) / 256.0;
        vec2  uv2          = (screen_position + vec2(float(seed) * f, float(seed))) * vec2(f, f);
        vec4  static_color = texture2D(static_image, uv2).rrrg;
        if (static_color.w <= static_fraction) {
            vec4 sprite_alpha = vec4(1, 1, 1, sprite_color.w);
            frag_color        = color * sprite_alpha;
        } else {
            frag_color = sprite_color;
        }
    } else if (color_mode == OUTLINE_SPRITE_MODE) {
        float neighborhood = texture2DRect(sprite, uv + vec2(-unit.s, -unit.t)).w +
                             texture2DRect(sprite, uv + vec2(-unit.s, 0)).w +
                             texture2DRect(sprite, uv + vec2(-unit.s, unit.t)).w +
                             texture2DRect(sprite, uv + vec2(0, -unit.t)).w +
                             texture2DRect(sprite, uv + vec2(0, unit.t)).w +
                             texture2DRect(sprite, uv + vec2(unit.s, -unit.t)).w +
                             texture2DRect(sprite, uv + vec2(unit.s, 0)).w +
                             texture2DRect(sprite, uv + vec2(unit.s, unit.t)).w;
        if (sprite_color.w > (neighborhood / 8.0)) {
            frag_color = outline_color;
        } else if (sprite_color.w > 0.0) {
            frag_color = color;
        } else {
            frag_color = vec4(0, 0, 0, 0);
        }
    }
    frag_color.rgb = apple_rgb_to_srgb(frag_color.rgb);
}
