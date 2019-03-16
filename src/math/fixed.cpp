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

#include "math/fixed.hpp"

#include <stdlib.h>
#include <cmath>
#include <pn/file>

namespace antares {

// From scripts/generate-fixed-table.py.
static const char kFractions[][5] = {
        ".0",   ".004", ".008", ".01",  ".016", ".02",  ".023", ".027", ".03",  ".035", ".04",
        ".043", ".047", ".05",  ".055", ".06",  ".063", ".066", ".07",  ".074", ".08",  ".082",
        ".086", ".09",  ".094", ".098", ".1",   ".105", ".11",  ".113", ".117", ".12",  ".125",
        ".13",  ".133", ".137", ".14",  ".145", ".15",  ".152", ".156", ".16",  ".164", ".168",
        ".17",  ".176", ".18",  ".184", ".188", ".19",  ".195", ".2",   ".203", ".207", ".21",
        ".215", ".22",  ".223", ".227", ".23",  ".234", ".24",  ".242", ".246", ".25",  ".254",
        ".258", ".26",  ".266", ".27",  ".273", ".277", ".28",  ".285", ".29",  ".293", ".297",
        ".3",   ".305", ".31",  ".313", ".316", ".32",  ".324", ".33",  ".332", ".336", ".34",
        ".344", ".348", ".35",  ".355", ".36",  ".363", ".367", ".37",  ".375", ".38",  ".383",
        ".387", ".39",  ".395", ".4",   ".402", ".406", ".41",  ".414", ".418", ".42",  ".426",
        ".43",  ".434", ".438", ".44",  ".445", ".45",  ".453", ".457", ".46",  ".465", ".47",
        ".473", ".477", ".48",  ".484", ".49",  ".492", ".496", ".5",   ".504", ".508", ".51",
        ".516", ".52",  ".523", ".527", ".53",  ".535", ".54",  ".543", ".547", ".55",  ".555",
        ".56",  ".563", ".566", ".57",  ".574", ".58",  ".582", ".586", ".59",  ".594", ".598",
        ".6",   ".605", ".61",  ".613", ".617", ".62",  ".625", ".63",  ".633", ".637", ".64",
        ".645", ".65",  ".652", ".656", ".66",  ".664", ".668", ".67",  ".676", ".68",  ".684",
        ".688", ".69",  ".695", ".7",   ".703", ".707", ".71",  ".715", ".72",  ".723", ".727",
        ".73",  ".734", ".74",  ".742", ".746", ".75",  ".754", ".758", ".76",  ".766", ".77",
        ".773", ".777", ".78",  ".785", ".79",  ".793", ".797", ".8",   ".805", ".81",  ".813",
        ".816", ".82",  ".824", ".83",  ".832", ".836", ".84",  ".844", ".848", ".85",  ".855",
        ".86",  ".863", ".867", ".87",  ".875", ".88",  ".883", ".887", ".89",  ".895", ".9",
        ".902", ".906", ".91",  ".914", ".918", ".92",  ".926", ".93",  ".934", ".938", ".94",
        ".945", ".95",  ".953", ".957", ".96",  ".965", ".97",  ".973", ".977", ".98",  ".984",
        ".99",  ".992", ".996", ".999",
};

pn::string stringify(Fixed fixed) {
    pn::string s;
    if (fixed < Fixed::zero()) {
        s += "-";
    }
    int64_t       value    = llabs(fixed.val());
    const int32_t integral = (value & 0xffffff00) >> 8;
    s.open("a").format("{0}", integral);
    value &= 0xff;
    s += kFractions[value];
    return s;
}

}  // namespace antares
