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

#include "Picture.hpp"

#include <assert.h>
#include <glob.h>
#include <fcntl.h>
#include <unistd.h>
#include "BinaryStream.hpp"
#include "FakeDrawing.hpp"

namespace antares {

extern PixMap* gActiveWorld;

Picture::Picture(int32_t id) {
    char fileglob[64];
    glob_t g;
    g.gl_offs = 0;

    sprintf(fileglob, "pictures/%d.bin", id);
    glob(fileglob, 0, NULL, &g);
    sprintf(fileglob, "pictures/%d *.bin", id);
    glob(fileglob, GLOB_APPEND, NULL, &g);

    if (g.gl_pathc == 0) {
        throw PictureNotFoundException();
    } else if (g.gl_pathc == 1) {
        MappedFile file(g.gl_pathv[0]);
        BufferBinaryReader bin(file.data(), file.size());

        _frame.left = 0;
        _frame.top = 0;
        bin.read(&_frame.right);
        bin.read(&_frame.bottom);

        int pixel_size = _frame.right * _frame.bottom;
        _pixels.reset(new uint8_t[pixel_size]);
        bin.discard(0x1000);
        bin.read(_pixels.get(), pixel_size);

        assert(bin.bytes_read() == file.size());
    } else {
        fprintf(stderr, "Found %lu matches for %d\n", g.gl_pathc, id);
        exit(1);
    }
    globfree(&g);
}

void Picture::draw(const Rect& dst) {
    ClippedTransfer transfer(_frame, dst);
    transfer.ClipDestTo(gActiveWorld->bounds);

    for (int i = 0; i < transfer.Height(); ++i) {
        uint8_t* source_bytes =
            _pixels.get() + transfer.SourceColumn(0) + transfer.SourceRow(i) * _frame.right;
        SetPixelRow(transfer.DestColumn(0), transfer.DestRow(i), source_bytes, transfer.Width());
    }
}

}  // namespace antares
