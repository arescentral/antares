// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include "game/starfield.hpp"

#include <sfz/sfz.hpp>

#include "data/space-object.hpp"
#include "drawing/color.hpp"
#include "game/globals.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
#include "math/random.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::range;

namespace antares {

const int kMinimumStarSpeed = 1;
const int kMaximumStarSpeed = 3;
const int kStarSpeedSpread = (kMaximumStarSpeed - kMinimumStarSpeed + 1);

const Fixed kSlowStarFraction   = 0x00000080;   // 0.500
const Fixed kMediumStarFraction = 0x000000c0;   // 0.750
const Fixed kFastStarFraction   = 0x00000100;   // 1.000

const uint8_t kStarColor = GRAY;


// This object is also used for the radar center, and for zoom to hostile and object modes.  It
// would be preferable for it to be entirely private to the starfield.
spaceObjectType* gScrollStarObject = NULL;

namespace {

inline int32_t RandomStarSpeed() {
    return Randomize(kStarSpeedSpread) + kMinimumStarSpeed;
}

}  // namespace

Starfield::Starfield():
        _last_clip_bottom(viewport.bottom),
        _warp_stars(false) {
    for (scrollStarType* star: range(_stars, _stars + kAllStarNum)) {
        star->speed = kNoStar;
    }
}

void Starfield::reset(int32_t which_object) {
    gScrollStarObject = mGetSpaceObjectPtr(which_object);

    if (gScrollStarObject == NULL) {
        return;
    }

    for (scrollStarType* star: range(_stars, _stars + kScrollStarNum)) {
        star->location.h = Randomize(play_screen.width()) + viewport.left;
        star->location.v = Randomize(play_screen.height()) + viewport.top;
        star->motionFraction.h = star->motionFraction.v = 0;

        star->speed = RandomStarSpeed();
        star->age = 0;
    }
    for (scrollStarType* star: range(_stars + kSparkStarOffset, _stars + kAllStarNum)) {
        star->age = 0;
    }
}

void Starfield::make_sparks(
        int32_t sparkNum, int32_t sparkSpeed, Fixed maxVelocity, uint8_t color, Point* location) {
    maxVelocity = evil_scale_by(maxVelocity, gAbsoluteScale);
    if (sparkNum <= 0) {
        return;
    }

    for (scrollStarType* spark: range(_stars + kSparkStarOffset, _stars + kAllStarNum)) {
        if (spark->speed == kNoStar) {
            spark->velocity.h = Randomize(maxVelocity << 2L) - maxVelocity;
            spark->velocity.v = Randomize(maxVelocity << 2L) - maxVelocity;
            spark->oldLocation.h = spark->location.h = location->h;
            spark->oldLocation.v = spark->location.v = location->v;
            spark->motionFraction.h = spark->motionFraction.v = 0;
            spark->age = kMaxSparkAge;
            spark->speed = sparkSpeed;
            spark->color = color;

            if (--sparkNum == 0) {
                return;
            }
        }
    }
}

// PrepareToMoveScrollStars:
//  We need to save the stars' last position since we may move them several times before they
//  are redrawn; the old positions have to be erased right after the new ones are drawn.

void Starfield::prepare_to_move() {
    for (scrollStarType* star: range(_stars, _stars + kAllStarNum)) {
        star->oldLocation = star->location;
    }
}

void Starfield::move(int32_t by_units) {
    if ((gScrollStarObject == NULL) || !gScrollStarObject->active) {
        return;
    }

    const fixedPointType slowVelocity = {
        scale_by(
                mMultiplyFixed(gScrollStarObject->velocity.h, kSlowStarFraction) * by_units,
                gAbsoluteScale),
        scale_by(
                mMultiplyFixed(gScrollStarObject->velocity.v, kSlowStarFraction) * by_units,
                gAbsoluteScale),
    };

    const fixedPointType mediumVelocity = {
        scale_by(
                mMultiplyFixed(gScrollStarObject->velocity.h, kMediumStarFraction) * by_units,
                gAbsoluteScale),
        scale_by(
                mMultiplyFixed(gScrollStarObject->velocity.v, kMediumStarFraction) * by_units,
                gAbsoluteScale),
    };

    const fixedPointType fastVelocity = {
        scale_by(
                mMultiplyFixed(gScrollStarObject->velocity.h, kFastStarFraction) * by_units,
                gAbsoluteScale),
        scale_by(
                mMultiplyFixed(gScrollStarObject->velocity.v, kFastStarFraction) * by_units,
                gAbsoluteScale),
    };

    for (scrollStarType* star: range(_stars, _stars + kScrollStarNum)) {
        const fixedPointType* velocity;
        switch (star->speed) {
            case kSlowStarSpeed:
                velocity = &slowVelocity;
                break;
            case kMediumStarSpeed:
                velocity = &mediumVelocity;
                break;
            case kFastStarSpeed:
                velocity = &fastVelocity;
                break;
            default:
            case kNoStar:
                continue;
        }

        star->motionFraction.h += velocity->h;
        star->motionFraction.v += velocity->v;

        int32_t h;
        if (star->motionFraction.h >= 0) {
            h = more_evil_fixed_to_long(star->motionFraction.h + mFloatToFixed(0.5));
        } else {
            h = more_evil_fixed_to_long(star->motionFraction.h - mFloatToFixed(0.5)) + 1;
        }
        star->location.h += h;
        star->motionFraction.h -= mLongToFixed(h);

        int32_t v;
        if (star->motionFraction.v >= 0) {
            v = more_evil_fixed_to_long(star->motionFraction.v + mFloatToFixed(0.5));
        } else {
            v = more_evil_fixed_to_long(star->motionFraction.v - mFloatToFixed(0.5)) + 1;
        }
        star->location.v += v;
        star->motionFraction.v -= mLongToFixed(v);

        if ((star->location.h < viewport.left) && (star->oldLocation.h < viewport.left)) {
            star->location.h += play_screen.width() - 1;
            star->location.v = Randomize(play_screen.height()) + viewport.top;
            star->motionFraction.h = star->motionFraction.v = 0;
            star->speed = RandomStarSpeed();
            star->age = 0;
        } else if ((star->location.h >= viewport.right) && (star->oldLocation.h >= viewport.right)) {
            star->location.h -= play_screen.width();
            star->location.v = Randomize(play_screen.height()) + viewport.top;
            star->motionFraction.h = star->motionFraction.v = 0;
            star->speed = RandomStarSpeed();
            star->age = 0;
        } else if ((star->location.v < viewport.top) && (star->oldLocation.v < viewport.top)) {
            star->location.h = Randomize(play_screen.width()) + viewport.left;
            star->location.v += play_screen.height() - 1;
            star->motionFraction.h = star->motionFraction.v = 0;
            star->speed = RandomStarSpeed();
            star->age = 0;
        } else if ((star->location.v >= play_screen.bottom)
                && (star->oldLocation.v >= play_screen.bottom)) {
            star->location.h = Randomize(play_screen.width()) + viewport.left;
            star->location.v -= play_screen.height();
            star->motionFraction.h = star->motionFraction.v = 0;
            star->speed = RandomStarSpeed();
            star->age = 0;
        }

        if (_warp_stars && (star->age == 0)) {
            switch (star->speed) {
                case kSlowStarSpeed:
                    velocity = &slowVelocity;
                    break;
                case kMediumStarSpeed:
                    velocity = &mediumVelocity;
                    break;
                case kFastStarSpeed:
                    velocity = &fastVelocity;
                    break;
                case kNoStar:
                    throw Exception("invalid value of star->speed.");
            }
            star->location.h -= mFixedToLong(velocity->h);
            star->location.v -= mFixedToLong(velocity->v);
        }
    }

    for (scrollStarType* star: range(_stars + kSparkStarOffset, _stars + kAllStarNum)) {
        if (star->speed == kNoStar) {
            continue;
        }
        star->age -= star->speed * by_units;

        star->motionFraction.h += star->velocity.h * by_units + slowVelocity.h;
        star->motionFraction.v += star->velocity.v * by_units + slowVelocity.v;

        int32_t h;
        if (star->motionFraction.h >= 0) {
            h = more_evil_fixed_to_long(star->motionFraction.h + mFloatToFixed(0.5));
        } else {
            h = more_evil_fixed_to_long(star->motionFraction.h - mFloatToFixed(0.5)) + 1;
        }
        star->location.h += h;
        star->motionFraction.h -= mLongToFixed(h);

        int32_t v;
        if (star->motionFraction.v >= 0) {
            v = more_evil_fixed_to_long(star->motionFraction.v + mFloatToFixed(0.5));
        } else {
            v = more_evil_fixed_to_long(star->motionFraction.v - mFloatToFixed(0.5)) + 1;
        }
        star->location.v += v;
        star->motionFraction.v -= mLongToFixed(v);
    }
}

void Starfield::draw() const {
    const RgbColor slowColor = GetRGBTranslateColorShade(kStarColor, MEDIUM);
    const RgbColor mediumColor = GetRGBTranslateColorShade(kStarColor, LIGHT);
    const RgbColor fastColor = GetRGBTranslateColorShade(kStarColor, LIGHTER);

    switch (gScrollStarObject->presenceState) {
      default:
        if (!_warp_stars) {
            // we're not warping in any way
            for (const scrollStarType* star: range(_stars, _stars + kScrollStarNum)) {
                if (star->speed != kNoStar) {
                    const RgbColor* color = &slowColor;
                    if (star->speed == kMediumStarSpeed) {
                        color = &mediumColor;
                    } else if (star->speed == kFastStarSpeed) {
                        color = &fastColor;
                    }

                    VideoDriver::driver()->draw_point(star->location, *color);
                }
            }
        }
        break;

      case kWarpInPresence:
      case kWarpOutPresence:
      case kWarpingPresence:
        for (const scrollStarType* star: range(_stars, _stars + kScrollStarNum)) {
            if (star->speed != kNoStar) {
                const RgbColor* color = &slowColor;
                if (star->speed == kMediumStarSpeed) {
                    color = &mediumColor;
                } else if (star->speed == kFastStarSpeed) {
                    color = &fastColor;
                }

                if (star->age > 1) {
                    VideoDriver::driver()->draw_line(
                            star->location, star->oldLocation, *color);
                }
            }
        }
        break;
    }

    for (const scrollStarType* star: range(_stars + kSparkStarOffset, _stars + kAllStarNum)) {
        if ((star->speed != kNoStar) && (star->age > 0)) {
            const RgbColor color = GetRGBTranslateColorShade(
                    star->color, (star->age >> kSparkAgeToShadeShift) + 1);
            VideoDriver::driver()->draw_point(star->location, color);
        }
    }
}

void Starfield::show() {
    if ((gScrollStarObject->presenceState != kWarpInPresence)
            && (gScrollStarObject->presenceState != kWarpOutPresence)
            && (gScrollStarObject->presenceState != kWarpingPresence)) {
        if (_warp_stars) {
            // we were warping but now are not; erase warped stars
            _warp_stars = false;
            for (scrollStarType* star: range(_stars, _stars + kScrollStarNum)) {
                if (star->speed != kNoStar) {
                    if (star->age < 2) {
                        ++star->age;
                    }
                }
            }
        }
    } else {
        // we're warping now
        _warp_stars = true;

        for (scrollStarType* star: range(_stars, _stars + kScrollStarNum)) {
            if (star->speed != kNoStar) {
                if (star->age < 2) {
                    ++star->age;
                }
            }
        }
    }

    for (scrollStarType* star: range(_stars + kScrollStarNum, _stars + kAllStarNum)) {
        if (star->speed != kNoStar) {
            if (star->age <= 0) {
                star->speed = kNoStar;
            }
        }
    }

    _last_clip_bottom = viewport.bottom;
}

}  // namespace antares
