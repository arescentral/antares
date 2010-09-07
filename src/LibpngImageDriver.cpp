// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "LibpngImageDriver.hpp"

#include <png.h>
#include <sfz/sfz.hpp>
#include "Error.hpp"
#include "FakeDrawing.hpp"

using sfz::ReadSource;
using sfz::WriteTarget;
using sfz::read;
using sfz::write;

namespace antares {

namespace {

void png_read_data(png_struct* png, png_byte* data, png_size_t length) {
    ReadSource* in = reinterpret_cast<ReadSource*>(png_get_io_ptr(png));
    read(*in, data, length);
}

void png_write_data(png_struct* png, png_byte* data, png_size_t length) {
    WriteTarget* out = reinterpret_cast<WriteTarget*>(png_get_io_ptr(png));
    write(*out, data, length);
}

void png_flush_data(png_struct*) { }

}  // namespace

LibpngImageDriver::LibpngImageDriver() { }

void LibpngImageDriver::read(ReadSource in, ArrayPixMap* pix) {
    png_byte sig[8];
    sfz::read(in, sig, 8);
    if (png_sig_cmp(sig, 0, 8) != 0) {
        fail("invalid png signature");
    }

    png_struct* png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fail("couldn't create png_struct");
    }

    png_info* info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        fail("couldn't create png_info");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fail("reading png failed");
    }

    png_set_sig_bytes(png, 8);
    png_set_read_fn(png, &in, png_read_data);
    png_read_info(png, info);

    png_uint_32 width;
    png_uint_32 height;
    int bit_depth;
    int color_type;
    png_get_IHDR(png, info, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);
    pix->resize(Rect(0, 0, width, height));

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
    if (color_type & PNG_COLOR_MASK_ALPHA) {
        png_set_swap_alpha(png);
    } else {
        png_set_filler(png, 0xFF, PNG_FILLER_BEFORE);
    }

    // Read in the data, one row at a time.
    for (size_t i = 0; i < height; ++i) {
        png_read_row(png, reinterpret_cast<uint8_t*>(pix->mutable_row(i)), NULL);
    }
}

void LibpngImageDriver::write(WriteTarget out, const PixMap& pix) {
    png_struct* png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fail("couldn't create png_struct");
    }

    png_info* info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, NULL);
        fail("couldn't create png_info");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fail("reading png failed");
    }

    png_set_write_fn(png, &out, png_write_data, png_flush_data);
    png_set_IHDR(
            png, info, pix.bounds().width(), pix.bounds().height(), 8, PNG_COLOR_TYPE_RGBA, NULL,
            NULL, NULL);
    png_set_swap_alpha(png);

    png_write_info(png, info);
    for (int i = 0; i < pix.bounds().height(); ++i) {
        png_write_row(png, const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(pix.row(i))));
    }
}

}  // namespace antares
