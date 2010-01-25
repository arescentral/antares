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

#include "DebriefingScreen.hpp"

#include "rezin/MacRoman.hpp"
#include "sfz/Formatter.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "PlayerInterfaceDrawing.hpp"
#include "Resource.hpp"
#include "RetroText.hpp"
#include "SoundFX.hpp"
#include "StringList.hpp"
#include "Time.hpp"

using rezin::mac_roman_encoding;
using sfz::Bytes;
using sfz::String;
using sfz::FormatItem;
using sfz::dec;

namespace antares {

extern long CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM;
extern PixMap* gRealWorld;

namespace {

const double kTypingDelay = 1.0 / 20.0;
const int kScoreTableHeight = 120;
const int kTextWidth = 300;

void string_replace(String* s, const String& in, const FormatItem& out) {
    size_t index = s->find(in);
    while (index != String::kNone) {
        String out_string;
        out.print_to(&out_string);
        s->replace(index, in.size(), out_string);
        index = s->find(in, index + 1);
    }
}

interfaceItemType interface_item(const Rect& text_bounds) {
    interfaceItemType result;
    result.bounds = text_bounds;
    result.color = GOLD;
    result.kind = kLabeledRect;
    result.style = kLarge;
    result.item.labeledRect.label.stringID = 2001;
    result.item.labeledRect.label.stringNumber = 29;
    return result;
}

Rect pix_bounds(const Rect& text_bounds) {
    Rect r;
    GetAnyInterfaceItemGraphicBounds(interface_item(text_bounds), &r);
    return r;
}

int score_low_target(double yours, double par, double value) {
    if (par == 0) {
        return (yours == 0) ? value : 0;
    }
    const double ratio = yours / par;
    if (ratio <= 1.0) {
        return value * std::min(2.0, (3 - (2 * ratio)));
    } else {
        return value * std::max(0.0, 2 - ratio);
    }
}

int score_high_target(double yours, double par, double value) {
    if (par == 0) {
        return (yours == 0) ? value : (value * 2);
    }
    const double ratio = yours / par;
    if (ratio >= 1.0) {
        return value * std::min(2.0, ratio);
    } else {
        return value * std::max(0.0, ratio * 2 - 1);
    }
}

int score(
        int your_length, int par_length, int your_loss, int par_loss, int your_kill,
        int par_kill) {
    int score = 0;
    score += score_low_target(your_length, par_length, 50);
    score += score_low_target(your_loss, par_loss, 30);
    score += score_high_target(your_kill, par_kill, 20);
    return score;
}

RetroText* score_text(
        int your_length, int par_length, int your_loss, int par_loss, int your_kill,
        int par_kill) {
    Resource rsrc('TEXT', 6000);
    String text(rsrc.data(), mac_roman_encoding());

    StringList strings;
    strings.load(6000);

    const int your_mins = your_length / 60;
    const int your_secs = your_length % 60;
    const int par_mins = par_length / 60;
    const int par_secs = par_length % 60;
    const int your_score = score(your_length, par_length, your_loss, par_loss, your_kill, par_kill);
    const int par_score = 100;

    string_replace(&text, strings.at(0), your_mins);
    string_replace(&text, strings.at(1), dec(your_secs, 2));
    if (par_length > 0) {
        string_replace(&text, strings.at(2), par_mins);
        String secs_string;
        format(&secs_string, ":{0}", dec(par_secs, 2));
        string_replace(&text, strings.at(3), secs_string);
    } else {
        StringList data_strings;
        data_strings.load(6002);
        string_replace(&text, strings.at(2), data_strings.at(8));  // = "N/A"
        string_replace(&text, strings.at(3), "");
    }
    string_replace(&text, strings.at(4), your_loss);
    string_replace(&text, strings.at(5), par_loss);
    string_replace(&text, strings.at(6), your_kill);
    string_replace(&text, strings.at(7), par_kill);
    string_replace(&text, strings.at(8), your_score);
    string_replace(&text, strings.at(9), par_score);

    RgbColor fore_color;
    GetRGBTranslateColorShade(&fore_color, GOLD, VERY_LIGHT);
    RgbColor back_color;
    GetRGBTranslateColorShade(&back_color, GOLD, DARKEST);
    return new RetroText(text, kButtonFontNum, fore_color, back_color);
}

}  // namespace

/*
void DoMissionDebriefing(
        Rect *destRect, long yourlength, long parlength, long yourloss, long parloss,
        long yourkill, long parkill) {
    Rect            clipRect, boundsRect, tlRect;
    Rect                tRect;
    long                height, waitTime, score = 0;
    retroTextSpecType   retroTextSpec;
    Str255              tempString, numString;

    // ** CALCULATE THE SCORE
    //  for time you get a max of 100 points
    //  50 points for par
    //  0-50 for par -> 2 x par
    //  50-100 for par -> 1/2 par
    //

    if ( yourlength < parlength)
    {
        if ( yourlength > ( parlength / 2))
        {
            score += (((kTimePoints * 2) * parlength - (kTimePoints * 2) * yourlength) / parlength) + kTimePoints;
        } else score += kTimePoints * 2;
    } else if ( yourlength > parlength)
    {
        if ( yourlength < ( parlength * 2))
        {
            score += ((kTimePoints * 2) * parlength - kTimePoints * yourlength) / parlength;
        } else score += 0;
    } else score += kTimePoints;

    if ( yourloss < parloss)
    {
        if ( yourloss > ( parloss / 2))
        {
            score += (((kLossesPoints * 2) * parloss - (kLossesPoints * 2) * yourloss) / parloss) + kLossesPoints;
        } else score += kLossesPoints * 2;
    } else if ( yourloss > parloss)
    {
        if ( yourloss < ( parloss * 2))
        {
            score += ((kLossesPoints * 2) * parloss - kLossesPoints * yourloss) / parloss;
        } else score += 0;
    } else score += kLossesPoints;

    if ( yourkill < parkill)
    {
        if ( yourkill > ( parkill / 2))
        {
            score += (((kKillsPoints * 2) * parkill - (kKillsPoints * 2) * yourkill) / parkill);
        } else score += 0;
    } else if ( yourkill > parkill)
    {
        if ( yourkill < ( parkill * 2))
        {
            score += ((kKillsPoints * 2) * parkill - kKillsPoints * yourkill) / parkill + kKillsPoints;
        } else score += kKillsPoints * 2;
    } else score += kKillsPoints;

    retroTextSpec.text.reset(new std::string(Resource::get_data('TEXT', kSummaryTextID)));
    if (retroTextSpec.text.get() != nil) {
        // *** Replace place-holders in text with real data, using the fabulous Munger routine
        // your minutes
        NumToString( yourlength / 60, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourMinStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your seconds
        NumToString( yourlength % 60, numString);
//      WriteDebugLine((char *)numString);
//      WriteDebugLong( yourlength % 60);

        mDoubleDigitize( numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourSecStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par minutes
        if ( parlength > 0)
            NumToString( parlength / 60, numString);
        else GetIndString( numString, 6002, 9); // = N/A
            GetIndString( tempString, kSummaryKeyStringID, kParMinStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par seconds
        if ( parlength > 0)
        {
            NumToString( parlength % 60, tempString);
            mDoubleDigitize( tempString);
            CopyPString( numString, "\p:");
            ConcatenatePString( numString, tempString);
            GetIndString( tempString, kSummaryKeyStringID, kParSecStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        } else
        {
            GetIndString( tempString, kSummaryKeyStringID, kParSecStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, 0);
        }

        // your loss
        NumToString( yourloss, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourLossStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par loss
        if ( parlength > 0)
            NumToString( parloss, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParLossStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your kill
        NumToString( yourkill, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourKillStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par kill
        if ( parlength > 0)
            NumToString( parkill, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParKillStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your score
        if ( parlength > 0)
            NumToString( score, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kYourScoreStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par score
        if ( parlength > 0)
            CopyPString( numString, "\p100");
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParScoreStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

        retroTextSpec.textLength = retroTextSpec.text->size();

        mSetDirectFont( kButtonFontNum);
        retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
        retroTextSpec.tabSize = 60;
        GetRGBTranslateColorShade(&retroTextSpec.color, GOLD, VERY_LIGHT);
        GetRGBTranslateColorShade(&retroTextSpec.backColor, GOLD, DARKEST);
        retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
        retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;

        retroTextSpec.topBuffer = 2;
        retroTextSpec.bottomBuffer = 0;

        height = DetermineDirectTextHeightInWidth( &retroTextSpec, destRect->right - destRect->left);

        boundsRect.left = destRect->left + (((destRect->right - destRect->left) / 2) -
                            ( retroTextSpec.autoWidth / 2));
        boundsRect.right = boundsRect.left + retroTextSpec.autoWidth;
        boundsRect.top = destRect->top + (((destRect->bottom - destRect->top) / 2) -
                            ( retroTextSpec.autoHeight / 2));
        boundsRect.bottom = boundsRect.top + retroTextSpec.autoHeight;
        retroTextSpec.xpos = boundsRect.left;
        retroTextSpec.ypos = boundsRect.top + mDirectFontAscent();

        clipRect.left = 0;
        clipRect.right = clipRect.left + WORLD_WIDTH;
        clipRect.top = 0;
        clipRect.bottom = clipRect.top + WORLD_HEIGHT;
        clipRect = *destRect;
        tlRect = boundsRect;
        tlRect.left -= 2;
        tlRect.top -= 2;
        tlRect.right += 2;
        tlRect.bottom += 2;
        DrawNateVBracket( gOffWorld, tlRect, clipRect, 0, 0, retroTextSpec.color);
        tRect = tlRect;
        CopyOffWorldToRealWorld(tRect);
    }
}
*/

DebriefingScreen::DebriefingScreen(int text_id)
        : _state(DONE),
          _message(Resource('TEXT', text_id).data()),
          _next_update(0.0),
          _typed_chars(0) {
    initialize(false);
}

DebriefingScreen::DebriefingScreen(
        int text_id, int your_length, int par_length, int your_loss, int par_loss,
        int your_kill, int par_kill)
        : _state(TYPING),
          _message(Resource('TEXT', text_id).data()),
          _next_update(0.0),
          _typed_chars(0) {
    initialize(true);

    Rect score_area = _message_bounds;
    score_area.top = score_area.bottom - kScoreTableHeight;

    _score.reset(score_text(your_length, par_length, your_loss, par_loss, your_kill, par_kill)),
    _score->set_tab_width(60);
    _score->wrap_to(_message_bounds.width(), 2);
    _score_bounds = Rect(0, 0, _score->auto_width(), _score->height());
    _score_bounds.center_in(score_area);

    RgbColor bracket_color;
    GetRGBTranslateColorShade(&bracket_color, GOLD, VERY_LIGHT);
    Rect bracket_bounds = _score_bounds;
    bracket_bounds.inset(-2, -2);
    DrawNateVBracket(_pix.get(), bracket_bounds, _pix->bounds(), 0, 0, bracket_color);
}

void DebriefingScreen::become_front() {
    _typed_chars = 0;
    if (_state == TYPING) {
        _next_update = now_secs() + kTypingDelay;
    } else {
        _next_update = 0;
    }
    gRealWorld->view(_pix_bounds).copy(*_pix);
}

void DebriefingScreen::resign_front() {
}

bool DebriefingScreen::mouse_down(int button, const Point& where) {
    static_cast<void>(button);
    static_cast<void>(where);
    stack()->pop(this);
    return true;
}

bool DebriefingScreen::key_down(int key) {
    static_cast<void>(key);
    stack()->pop(this);
    return true;
}

double DebriefingScreen::next_timer() {
    return _next_update;
}

void DebriefingScreen::fire_timer() {
    check(_state == TYPING, "DebriefingScreen::fire_timer() called but _state is %d", _state);
    PlayVolumeSound(kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
    double now = now_secs();
    while (_next_update <= now) {
        if (_typed_chars < _score->size()) {
            _score->draw_char(_pix.get(), _score_bounds, _typed_chars);
            _next_update += kTypingDelay;
            ++_typed_chars;
        } else {
            _next_update = 0;
            _state = DONE;
            break;
        }
    }
    gRealWorld->view(_pix_bounds).copy(*_pix);
}

void DebriefingScreen::initialize(bool do_score) {
    int text_height = GetInterfaceTextHeightFromWidth(
            reinterpret_cast<const unsigned char*>(_message.data()), _message.size(),
            kLarge, kTextWidth);
    Rect text_bounds(0, 0, kTextWidth, text_height);
    if (do_score) {
        text_bounds.bottom += kScoreTableHeight;
    }
    text_bounds.center_in(Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM));

    _pix_bounds = pix_bounds(text_bounds);
    _message_bounds = text_bounds;
    _message_bounds.offset(-_pix_bounds.left, -_pix_bounds.top);
    _pix.reset(new ArrayPixMap(_pix_bounds.width(), _pix_bounds.height()));

    DrawAnyInterfaceItem(interface_item(_message_bounds), _pix.get());
    DrawInterfaceTextInRect(
            _message_bounds, reinterpret_cast<const unsigned char*>(_message.data()),
            _message.size(), kLarge, GOLD, _pix.get(), NULL);
}

}  // namespace antares
