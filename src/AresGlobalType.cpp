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

#include "AresGlobalType.hpp"

#include "ColorTable.hpp"
#include "InputSource.hpp"
#include "MessageScreen.hpp"
#include "Options.hpp"
#include "SpriteHandling.hpp"

static aresGlobalType* gAresGlobal;

aresGlobalType* globals() {
    return gAresGlobal;
}

void init_globals() {
    gAresGlobal = new aresGlobalType;
}

aresGlobalType::aresGlobalType() {
    for (int player = 0; player < kMaxPlayerNum; player++) {
        gActiveCheats[player] = 0;
    }
    gKeyMapBuffer = new KeyMap[kKeyMapBufferNum];
    gKeyMapBufferTop = 0;
    gKeyMapBufferBottom = 0;
    gBackWindow = nil;
    gForceDemoLevel = 1;
    gFrameCount = 0;
    gGameOver = 1;
    gPreferenceRefNum = 0;
    gOptions = kDefaultOptions;
    gWarpStars = false;
    gLastClipBottom = 0;
    gScrollStarNumber = -1;
    gGameTime = 0;
    gGameStartTime = 0;
    gClosestObject = 0;
    gFarthestObject = 0;
    gCenterScaleH = 0;
    gCenterScaleV = 0;
    gLastKeys = 0;
    gTheseKeys = 0;
    gPlayerShipNumber = 0;
    gSelectionLabel = -1;
    gDestKeyTime = 0;
    gZoomMode = kTimesTwoZoom;
    gDestinationLabel = -1;
    gAlarmCount = -1;
    gSendMessageLabel = -1;
    gDemoZoomOverride = false;
    gScenarioRotation = 0;
    gThisScenarioNumber = -1;
    gScenarioRefID = 0;
    gRadarCount = 0;
    gRadarSpeed = 30;
    gRadarRange = kRadarSize * 50;
    gWhichScaleNum = 0;
    gLastScale = SCALE_SCALE;
    gInstrumentTop = 0;
    gRightPanelLeftEdge = 608;
    gMouseActive = kMouseOff;
    gMessageTimeCount = 0;
    gMessageLabelNum = -1;
    gStatusLabelNum = -1;
    gTrueClipBottom = 0;
    gColorAnimationStep = 0;
    gColorAnimationInSpeed = -1;
    gColorAnimationOutSpeed = -1;
    gLastSoundTime = 0;
    gSoundVolume = 0;
    gSoundFileRefID = 0;
    gLastSelectedBuildPrice = 0;
    gAutoPilotOff = true;
    isQuitting = false;
    aeInited = false;
    gameRangerPending = false;
    gameRangerInProgress = false;
    useGameRanger = false;
    returnToMain = false;
    levelNum = 31;
    keyMask = 0;
    haveSeenRTNotice = false;
    ambrosia_Is_Registered = false;
    user_is_scum = false;
    gSerialNumerator = 0;
    gSerialDenominator = 0;
    okToOpenFile = true;
    externalFileRefNum = -1;
    externalFileSpec.vRefNum = 0;
    externalFileSpec.parID = 0;
    externalFileSpec.name[0] = 0;
    originalExternalFileSpec.vRefNum = 0;
    originalExternalFileSpec.parID = 0;
    originalExternalFileSpec.name[0] = 0;
    otherPlayerScenarioFileName[0] = 0;
    otherPlayerScenarioFileURL[0] = 0;
    otherPlayerScenarioFileVersion = 0;
    otherPlayerScenarioFileCheckSum = 0;

    internetConfigPresent = false;

    hotKeyDownTime = -1;
}

aresGlobalType::~aresGlobalType() {
}
