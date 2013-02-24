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

#include "game/beam.hpp"

#include <algorithm>
#include <cmath>
#include <sfz/sfz.hpp>

#include "data/space-object.hpp"
#include "drawing/color.hpp"
#include "game/globals.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
#include "lang/casts.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/units.hpp"
#include "video/driver.hpp"

using sfz::range;
using sfz::scoped_array;
using std::abs;
using std::max;

namespace antares {

namespace {

const int kBeamNum          = 256;
const int kBoltChangeTime   = 0;

void DetermineBeamRelativeCoordFromAngle(spaceObjectType *beamObject, int16_t angle) {
    Fixed range = mLongToFixed(beamObject->frame.beam.beam->range);

    mAddAngle(angle, -90);
    Fixed fcos, fsin;
    GetRotPoint(&fcos, &fsin, angle);

    beamObject->frame.beam.beam->toRelativeCoord = Point(
            mFixedToLong(mMultiplyFixed(0, -fcos) - mMultiplyFixed(range, -fsin)),
            mFixedToLong(mMultiplyFixed(0, -fsin) + mMultiplyFixed(range, -fcos)));
}

template <typename T>
void clear(T& t) {
    using std::swap;
    T u;
    swap(t, u);
}

int32_t scale(int32_t value, int32_t scale) {
    return (value * scale) >> SHIFT_SCALE;
}

}  // namespace

beamType::beamType():
        killMe(false),
        active(false) { }

void InitBeams() {
    globals()->gBeamData.reset(new beamType[kBeamNum]);
}

void ResetBeams() {
    beamType* const beams = globals()->gBeamData.get();
    SFZ_FOREACH(beamType* beam, range(beams, beams + kBeamNum), {
        clear(*beam);
    });
}

beamType *AddBeam(
        coordPointType* location, uint8_t color, beamKindType kind, int32_t accuracy,
        int32_t beam_range, int32_t* whichBeam) {
    beamType* const beams = globals()->gBeamData.get();
    SFZ_FOREACH(beamType* beam, range(beams, beams + kBeamNum), {
        if (!beam->active) {
            beam->lastGlobalLocation = *location;
            beam->objectLocation = *location;
            beam->lastApparentLocation = *location;
            beam->killMe = false;
            beam->active = true;
            beam->color = color;

            const int32_t h = scale(location->h - gGlobalCorner.h, gAbsoluteScale);
            const int32_t v = scale(location->v - gGlobalCorner.v, gAbsoluteScale);
            beam->thisLocation = Rect(0, 0, 0, 0);
            beam->thisLocation.offset(h + viewport.left, v + viewport.top);

            beam->lastLocation = beam->thisLocation;

            beam->beamKind = kind;
            beam->accuracy = accuracy;
            beam->range = beam_range;
            beam->fromObjectNumber = beam->fromObjectID = -1;
            beam->fromObject = NULL;
            beam->toObjectNumber = beam->toObjectID = -1;
            beam->toObject = NULL;
            beam->toRelativeCoord = Point(0, 0);
            beam->boltRandomSeed = 0;
            beam->boltCycleTime = 0;
            beam->boltState = 0;

            *whichBeam = beam - beams;
            return beam;
        }
    });

    *whichBeam = -1;
    return NULL;
}

void SetSpecialBeamAttributes(spaceObjectType* beamObject, spaceObjectType* sourceObject) {
    beamType& beam = *beamObject->frame.beam.beam;
    beam.fromObjectNumber = sourceObject->entryNumber;
    beam.fromObjectID = sourceObject->id;
    beam.fromObject = sourceObject;

    if (sourceObject->targetObjectNumber >= 0) {
        spaceObjectType* target = gSpaceObjectData.get() + sourceObject->targetObjectNumber;

        if ((target->active) && (target->id == sourceObject->targetObjectID)) {
            const int32_t h = abs(implicit_cast<int32_t>(
                        target->location.h - beamObject->location.h));
            const int32_t v = abs(implicit_cast<int32_t>(
                        target->location.v - beamObject->location.v));

            if ((((h * h) + (v * v)) > (beam.range * beam.range))
                    || (h > kMaximumRelevantDistance)
                    || (v > kMaximumRelevantDistance)) {
                if (beam.beamKind == eStaticObjectToObjectKind) {
                    beam.beamKind = eStaticObjectToRelativeCoordKind;
                } else if (beam.beamKind == eBoltObjectToObjectKind) {
                    beam.beamKind = eBoltObjectToRelativeCoordKind;
                }
                DetermineBeamRelativeCoordFromAngle(beamObject, sourceObject->targetAngle);
            } else {
                if ((beam.beamKind == eStaticObjectToRelativeCoordKind)
                        || (beam.beamKind == eBoltObjectToRelativeCoordKind)) {
                    beam.toRelativeCoord.h = target->location.h - sourceObject->location.h
                        - beam.accuracy
                        + RandomSeeded(beam.accuracy << 1, &(beamObject->randomSeed), 'beam', 1);
                    beam.toRelativeCoord.v = target->location.v - sourceObject->location.v
                        - beam.accuracy
                        + RandomSeeded(beam.accuracy << 1, &(beamObject->randomSeed), 'beam', 1);
                } else {
                    beam.toObjectNumber = target->entryNumber;
                    beam.toObjectID = target->id;
                    beam.toObject = target;
                }
            }
        } else { // target not valid
            if (beam.beamKind == eStaticObjectToObjectKind) {
                beam.beamKind = eStaticObjectToRelativeCoordKind;
            } else if (beam.beamKind == eBoltObjectToObjectKind) {
                beam.beamKind = eBoltObjectToRelativeCoordKind;
            }
            DetermineBeamRelativeCoordFromAngle(beamObject, sourceObject->direction);
        }
    } else { // target not valid
        if (beam.beamKind == eStaticObjectToObjectKind) {
            beam.beamKind = eStaticObjectToRelativeCoordKind;
        } else if (beam.beamKind == eBoltObjectToObjectKind) {
            beam.beamKind = eBoltObjectToRelativeCoordKind;
        }
        DetermineBeamRelativeCoordFromAngle(beamObject, sourceObject->direction);
    }
}

void update_beams() {
    beamType* const beams = globals()->gBeamData.get();
    SFZ_FOREACH(beamType* beam, range(beams, beams + kBeamNum), {
        if (beam->active) {
            if (beam->lastApparentLocation != beam->objectLocation) {
                beam->thisLocation = Rect(
                        scale(beam->objectLocation.h - gGlobalCorner.h, gAbsoluteScale),
                        scale(beam->objectLocation.v - gGlobalCorner.v, gAbsoluteScale),
                        scale(beam->lastApparentLocation.h - gGlobalCorner.h, gAbsoluteScale),
                        scale(beam->lastApparentLocation.v - gGlobalCorner.v, gAbsoluteScale));
                beam->thisLocation.offset(viewport.left, viewport.top);
                beam->lastApparentLocation = beam->objectLocation;
            }

            if ((!beam->killMe) && (beam->active != kObjectToBeFreed)) {
                if (beam->color) {
                    if (beam->beamKind != eKineticBeamKind) {
                        beam->boltState++;
                        if (beam->boltState > 24) beam->boltState = -24;
                        uint8_t currentColor = GetRetroIndex(beam->color);
                        currentColor &= 0xf0;
                        if (beam->boltState < 0)
                            currentColor += (-beam->boltState) >> 1;
                        else
                            currentColor += beam->boltState >> 1;
                        beam->color = GetTranslateIndex(currentColor);
                    }
                    if ((beam->beamKind == eBoltObjectToObjectKind)
                            || (beam->beamKind == eBoltObjectToRelativeCoordKind)) {
                        beam->boltCycleTime++;
                        if (beam->boltCycleTime > kBoltChangeTime) {
                            beam->boltCycleTime = 0;
                            beam->thisBoltPoint[0].h = beam->thisLocation.left;
                            beam->thisBoltPoint[0].v = beam->thisLocation.top;
                            beam->thisBoltPoint[kBoltPointNum - 1].h = beam->thisLocation.right;
                            beam->thisBoltPoint[kBoltPointNum - 1].v = beam->thisLocation.bottom;

                            int32_t inaccuracy = max(
                                    abs(beam->thisLocation.width()),
                                    abs(beam->thisLocation.height()))
                                / kBoltPointNum / 2;

                            SFZ_FOREACH(int j, range(1, kBoltPointNum - 1), {
                                beam->thisBoltPoint[j].h = beam->thisLocation.left
                                    + ((beam->thisLocation.width() * j) / kBoltPointNum)
                                    - inaccuracy + Randomize(inaccuracy * 2);
                                beam->thisBoltPoint[j].v = beam->thisLocation.top
                                    + ((beam->thisLocation.height() * j) / kBoltPointNum)
                                    - inaccuracy + Randomize(inaccuracy * 2);
                            });
                        }
                    }
                }
            }
        }
    });
}

void draw_beams() {
    beamType* const beams = globals()->gBeamData.get();
    SFZ_FOREACH(beamType* beam, range(beams, beams + kBeamNum), {
        if (beam->active) {
            if ((!beam->killMe) && (beam->active != kObjectToBeFreed)) {
                if (beam->color) {
                    if ((beam->beamKind == eBoltObjectToObjectKind)
                            || (beam->beamKind == eBoltObjectToRelativeCoordKind)) {
                        SFZ_FOREACH(int j, range(1, kBoltPointNum), {
                            VideoDriver::driver()->draw_line(
                                    beam->thisBoltPoint[j-1], beam->thisBoltPoint[j],
                                    GetRGBTranslateColor(beam->color));
                        });
                    } else {
                        VideoDriver::driver()->draw_line(
                                Point(beam->thisLocation.left, beam->thisLocation.top),
                                Point(beam->thisLocation.right, beam->thisLocation.bottom),
                                GetRGBTranslateColor(beam->color));
                    }
                }
            }
        }
    });
}

void ShowAllBeams() {
    beamType* const beams = globals()->gBeamData.get();
    SFZ_FOREACH(beamType* beam, range(beams, beams + kBeamNum), {
        if (beam->active) {
            if ((beam->killMe) || (beam->active == kObjectToBeFreed)) {
                beam->active = false;
            }
            if (beam->color) {
                if ((beam->beamKind == eBoltObjectToObjectKind)
                        || (beam->beamKind == eBoltObjectToRelativeCoordKind)) {
                    SFZ_FOREACH(int j, range(kBoltPointNum), {
                        beam->lastBoltPoint[j] = beam->thisBoltPoint[j];
                    });
                }
            }
            beam->lastLocation = beam->thisLocation;
        }
    });
}

void CullBeams() {
    beamType* const beams = globals()->gBeamData.get();
    SFZ_FOREACH(beamType* beam, range(beams, beams + kBeamNum), {
        if (beam->active) {
                if ((beam->killMe) || (beam->active == kObjectToBeFreed)) {
                    beam->active = false;
                }
                beam->lastLocation = beam->thisLocation;
        }
    });
}

}  // namespace antares
