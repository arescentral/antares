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

#include "drawing/pix-map.hpp"

#include <png.h>

namespace antares {

ArrayPixMap read_png(pn::file_view in) {
    ArrayPixMap pix{0, 0};
    png_byte    sig[8];
    fread(sig, 1, 8, in.c_obj());
    if (png_sig_cmp(sig, 0, 8) != 0) {
        throw std::runtime_error("invalid png signature");
    }

    png_struct* png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        throw std::runtime_error("couldn't create png_struct");
    }

    png_info* info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        throw std::runtime_error("couldn't create png_info");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        throw std::runtime_error("reading png failed");
    }

    png_set_sig_bytes(png, 8);
    png_init_io(png, in.c_obj());
    png_read_info(png, info);

    png_uint_32 width;
    png_uint_32 height;
    int         bit_depth;
    int         color_type;
    png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    pix.resize(Size(width, height));

    // We only want to deal with images in 8-bit format.
    if (bit_depth == 16) {
        png_set_strip_16(png);
    } else if (bit_depth < 8) {
        png_set_packing(png);
    }

    // Expand all palette-based and grayscale images to use RGB triples.
    if (color_type & PNG_COLOR_MASK_PALETTE) {
        png_set_palette_to_rgb(png);
    }
    if (!(color_type & PNG_COLOR_MASK_COLOR)) {
        png_set_gray_to_rgb(png);
    }

    // If we are reading an image with an alpha channel, we want the alpha channel to be the
    // first component, rather than the last.  If we are reading an image without an alpha
    // channel, add a 0xFF byte before each RGB triplet.
    png_set_swap_alpha(png);
    if (!(color_type & PNG_COLOR_MASK_ALPHA)) {
        png_set_filler(png, 0xFF, PNG_FILLER_BEFORE);
    }

    // Read in the data, one row at a time.
    for (size_t i = 0; i < height; ++i) {
        png_read_row(png, reinterpret_cast<uint8_t*>(pix.mutable_row(i)), NULL);
    }

    png_destroy_read_struct(&png, &info, NULL);
    return pix;
}

void PixMap::encode(pn::file_view out) {
    png_struct* png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        throw std::runtime_error("couldn't create png_struct");
    }

    png_info* info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        throw std::runtime_error("couldn't create png_info");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        throw std::runtime_error("reading png failed");
    }

    png_init_io(png, out.c_obj());
    png_set_IHDR(png, info, size().width, size().height, 8, PNG_COLOR_TYPE_RGBA, 0, 0, 0);
    png_set_swap_alpha(png);

    png_write_info(png, info);
    for (int i = 0; i < size().height; ++i) {
        png_write_row(png, const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(row(i))));
    }

    png_write_end(png, NULL);

    png_destroy_write_struct(&png, &info);
}

}  // namespace antares
