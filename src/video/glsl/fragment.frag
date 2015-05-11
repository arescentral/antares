#version 120

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
    vec2 uv = gl_TexCoord[0].xy;
    vec4 sprite_color = texture2DRect(sprite, uv);
    if (color_mode == FILL_MODE) {
        gl_FragColor = gl_Color;
    } else if (color_mode == DITHER_MODE) {
        if (mod(floor(gl_TexCoord[1].s) + floor(gl_TexCoord[1].t), 2) == 1) {
            gl_FragColor = gl_Color;
        } else {
            gl_FragColor = vec4(0, 0, 0, 0);
        }
    } else if (color_mode == DRAW_SPRITE_MODE) {
        gl_FragColor = sprite_color;
    } else if (color_mode == TINT_SPRITE_MODE) {
        gl_FragColor = gl_Color * sprite_color;
    } else if (color_mode == STATIC_SPRITE_MODE) {
        vec2 uv2 = (gl_TexCoord[1].xy + vec2(seed / 256, seed)) * vec2(1.0/256, 1.0/256);
        vec4 static_color = texture2D(static_image, uv2);
        if (static_color.w <= static_fraction) {
            vec4 sprite_alpha = vec4(1, 1, 1, sprite_color.w);
            gl_FragColor = gl_Color * sprite_alpha;
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
            gl_FragColor = gl_Color;
        } else {
            gl_FragColor = vec4(0, 0, 0, 0);
        }
    }
}
