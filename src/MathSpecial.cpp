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

// MATH SPECIAL.C

// Special Math Functions

#include "MathSpecial.hpp"

#include "sfz/BinaryReader.hpp"

using sfz::BinaryReader;

namespace antares {

//      FOR THE SQUARE ROOT CODE:
//        Saturday, May 14, 1994 11:01:56 AM
//        mac.programmer Item
// From:          Ron Hunsinger,Ron_Hunsinger@bmug.org,Internet
// Subject:       Re: Faster Square Root Algorithm
// To:            mac.programmer
// christer@cs.umu.se (Christer Ericson) writes:
//
// >In <00148DC5.fc@bmug.org> Ron_Hunsinger@bmug.org (Ron Hunsinger) writes:
// >>
// >>parkb@bigbang.Stanford.EDU (Brian Park) writes:
// >>>[...suggests using Newton's method...]
// >>
// >>You can do square root much faster than this.
// >
// >I beg to differ. In article <CpA23t.DwM@cs.umu.se> I posted an 68000-
// >based implementation of Newton's method (with quirks) which is something
// >like 20-30% faster than the equivalent hand-coded optimized assembly
// >version of the routine you posted in another article. It all depends on
// >how good your initial guess for Newton's method is.
//
// You've got me mixed up with somebody else.  I did say you can do square
// root much faster, and explained how, but I didn't post any assembly
// version.  I posted the C version (which therefore has the advantage
// that it can be compiled native for PowerPC, or ported easily to another
// machine that doesn't even have to be binary, although of course it is
// unlikely that shifting will be fast on a non-binary machine).
//
// I saw two assembly versions.  One, which was quite long, very tightly
// optimized, and attributed to Motorola, was indeed faster than my short
// simple C version, but not by all that much.  Also, it always truncated,
// where mine would produce the correctly rounded result (or the truncated
// result by commenting out one line if that's really what you wanted).
//
// The other, very short assembler version was hopelessly flawed.  It
// computed an initial guess that could be off by a factor of two, and
// then did *ONE* iteration of Newton's method, and stopped there, producing
// a value that was not even close (in absolute terms), even when it did
// not lose all significance due to overflow.  If asked to take the square
// root of zero, it would divide by zero.  It probably was fast (when it
// didn't crash), but who cares how quickly you get the wrong answer?
//
// Which version was yours?
//
// I'll be charitable and assume it was the Motorola version.  I no longer
// have the original message, but I seem to recall the max running time
// was something like 1262 cycles on a 68000, and (quite a bit) faster
// on small values.  I modified my C version to special-case small values
// too, and unwind the iterations of the main loop up to the iteration that
// finds the high bit of the root.  The main benefit of special-casing small
// values is so I don't have to worry about that boundary in the remaining
// code -- I am then guaranteed that the (unrounded) root is at least two
// bits long.  And although I am not displeased by the improved efficiency,
// my main reason for unwinding the first iterations was to remove the
// restriction in the prior version that it would only work on machines in
// which the number of bits in an unsigned long was even.  This code will
// now work even on machines with odd integer sizes.  (Yes, Virginia, such
// machines do exist!  An example is the Unisys A-series computers, in which
// ALL arithmetic is done in floating point, and integers are just the special
// case where the exponent is zero and the integer value is stored in the
// 39-bit mantissa of a 48-bit word.)
//
// My improved C version appears at the end of this message.
//
// I compiled it with MPW C with no special optimizations, tested it
// thoroughly, and then disassembled the code and calculated the running
// time on a 68000.  (On later machines, the presence of the cache makes
// accurate timings much more difficult to do by hand.)  I did not make
// any attempt to "improve" the C-generated code, although there are some
// easy and obvious optimizations available.  The running time for my
// version (the one that correctly rounds), counting the RTS and the
// (quite unnecessary) LINK/MOVEM.L/.../MOVE.L <rslt>,D0/MOVEM.L/UNLK is:
//
//    If N <= 12 then 204+4N, else 620+46u+32z+6r, where
//         N = input argument
//         u = Number of one-bits in the unrounded root
//         z = Number of non-leading zero-bits in the unrounded root
//         r = 1 if rounding increases the root, else 0
//
//    The actual running time for various values of N is:
//
//                  N   Time  Comments
//          ---------  -----  -------------------------
//                  0    204  Best time
//                 13    718  Best time not using lookup table
//                625    822  This is a cut-point for the Motorola code
//           0..65535    935  Average* time over inputs that fit in 16 bits
//              65535    994  Worst time if N fits in 16 bits
//      0..0xFFFFFFFF   1247  Average* time over all 32-bit inputs
//         0xFFFFFFFF   1362  Worst case
//        *average times computed assuming uniform distrubution of outputs
//
// If my memory serves me right, this is only about 8% slower than the
// hand-crafted Motorola code, which I think is a cheap price to pay for
// the portability.  Especially considering that most of that difference
// is due to the fact that they split the general loop into a main loop
// that could get away with word-size operations for some variables, and
// a few unwound iterations that had to use long-sized operands.  Notice
// that this is counterproductive on 68020 and up, where long operations
// are just as fast as word operations when the operands are in registers,
// and the only effect of unwinding the last two iterations is to
// guarantee that they won't be in the cache.
//
// The compiled 68K code fits in 104 bytes (decimal). (96 bytes without
// the debugger symbol. This really was a vanilla compile.)  How big was
// your code?  How big is the code cache on your machine?  How much of
// your caller's code do you push out of the cache?
//

#define lsqrt_max4pow (1UL << 30)
// lsqrt_max4pow is the (machine-specific) largest power of 4 that can
// be represented in an unsigned long.

uint32_t lsqrt(uint32_t n) {
    // Compute the integer square root of the integer argument n
    // Method is to divide n by x computing the quotient x and remainder r
    // Notice that the divisor x is changing as the quotient x changes

    // Instead of shifting the dividend/remainder left, we shift the
    // quotient/divisor right.  The binary point starts at the extreme
    // left, and shifts two bits at a time to the extreme right.

    // The residue contains n-x^2.  (Within these comments, the ^ operator
    // signifies exponentiation rather than exclusive or.  Also, the /
    // operator returns fractions, rather than truncating, so 1/4 means
    // one fourth, not zero.)

    // Since (x + 1/2)^2 == x^2 + x + 1/4,
    //   n - (x + 1/2)^2 == (n - x^2) - (x + 1/4)
    // Thus, we can increase x by 1/2 if we decrease (n-x^2) by (x+1/4)

    uint32_t residue;           // n - x^2
    uint32_t root;              // x + 1/4
    uint32_t half;              // 1/2

    residue = n;                // n - (x = 0)^2, with suitable alignment

    // if the correct answer fits in two bits, pull it out of a magic hat
#ifndef lsqrt_truncate
    if (residue <= 12)
        return (0x03FFEA94 >> (residue *= 2)) & 3;
#else
    if (residue <= 15)
        return (0xFFFEAA54 >> (residue *= 2)) & 3;
#endif
    root = lsqrt_max4pow;       // x + 1/4, shifted all the way left
//  half = root + root;         // 1/2, shifted likewise

    // Unwind iterations corresponding to leading zero bits
    while (root > residue) root >>= 2;

    // Unwind the iteration corresponding to the first one bit
    // Operations have been rearranged and combined for efficiency
    // Initialization of half is folded into this iteration
    residue -= root;            // Decrease (n-x^2) by (0+1/4)
    half = root >> 2;           // 1/4, with binary point shifted right 2
    root += half;               // x=1.  (root is now (x=1)+1/4.)
    half += half;               // 1/2, properly aligned

    // Normal loop (there is at least one iteration remaining)
    do {
        if (root <= residue) {  // Whenever we can,
            residue -= root;        // decrease (n-x^2) by (x+1/4)
            root += half;           // increase x by 1/2
        }
        half >>= 2;             // Shift binary point 2 places right
        root -= half;           // x{+1/2}+1/4 - 1/8 == x{+1/2}+1/8
        root >>= 1;             // 2x{+1}+1/4, shifted right 2 places
    } while (half);         // When 1/2 == 0, bin. point is at far right

#ifndef lsqrt_truncate
    if (root < residue) ++root;  // round up if (x+1/2)^2 < n
#endif

    return root;        // Guaranteed to be correctly rounded (or truncated)
}

Fixed MyFixRatio(int16_t numer, int16_t denom) {
    int32_t longdenom, result;

    longdenom = denom;
    if (!longdenom) {
        goto label1;
    }

    result = numer;
    result &= 0x0000ffff;
    if (numer == denom) {
        goto label3;
    }

    result = ((result >> 16L) & 0x0000ffff) | ((result << 16L) & 0xffff0000);
    result /= longdenom;
    return result;

label1:
    result = 0x7fffffff;
    if (numer >= 0) {
        goto label2;
    }
    result = -result;

label2:
    return result;

label3:
    result = 0x00000001;
    result = (( result >> 16L) & 0x0000ffff) | (( result << 16L) & 0xffff0000);
    return result;
}

void fixedPointType::read(BinaryReader* bin) {
    bin->read(&h);
    bin->read(&v);
}

}  // namespace antares
