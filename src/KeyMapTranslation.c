/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Key Map Translation.c
#ifndef __CONDITIONALMACROS__
#include "ConditionalMacros.h"
#endif // __CONDITIONALMACROS__

#if TARGET_OS_WIN32

    #ifndef __QUICKTIMEVR__
    #include <QuickTimeVR.h>
    #endif

    #ifndef __QTUtilities__
    #include "QTUtilities.h"
    #endif

    #ifndef __QTVRUtilities__
    #include "QTVRUtilities.h"
    #endif

    #include <TextUtils.h>
    #include <Script.h>
    #include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"

#include "Key Map Translation.h"
#include "Error.h"

#define kKeyMapError            "\pKEYM"

unsigned long   gKeyTranslateState = 0;

void GetKeyMapFromKeyNum( short keyNum, KeyMap keyMap)

{
    short       wmap, wbit, fixbit;

    keyNum--;
    for ( wmap = 0; wmap < 4; wmap++)
#if TARGET_OS_MAC
        keyMap[wmap] = 0;   
#else
        keyMap[wmap].bigEndianValue = 0;    
#endif TARGET_OS_MAC
    wbit = keyNum % 32;
    wmap = keyNum / 32;
    fixbit = wbit;
    wbit = 32 - ((fixbit / 8) * 8 + (8 - (fixbit % 8)));
#if TARGET_OS_MAC
    keyMap[wmap] = (unsigned long)0x00000001 << (unsigned long)wbit;
#else
    keyMap[wmap].bigEndianValue = EndianU32_NtoB((unsigned long)0x00000001 << (unsigned long)wbit);
#endif TARGET_OS_MAC
}

short GetKeyNumFromKeyMap( KeyMap keyMap)

{
    int         i, j, wmap, wbit, fixbit, keyispressed = 0;
        
    for ( i = 0; i < 4; i++)
    {
        for ( j = 0; j < 32; j++)
        {
#if TARGET_OS_MAC
            if ((keyMap[i] >> (unsigned long)j) & (unsigned long)0x00000001)
#else
            if ((EndianU32_BtoN(keyMap[i].bigEndianValue) >> (unsigned long)j) & (unsigned long)0x00000001)
#endif TARGET_OS_MAC
            {
                wmap = i;
                wbit = j; 
                i = 4;   
                j = 32;
                keyispressed = 1;
            }
        }
    }
    fixbit = wbit;
    wbit = 32 - ((fixbit / 8) * 8 + (8 - (fixbit % 8)));
    if ( keyispressed)
      return( 1 + wmap * 32 + wbit);
    else return (0);
}

Boolean DoesKeyMapContainKeyNum( KeyMap smap, short keyNum)
{
    KeyMap  dmap;
    short   i;
    
    GetKeyMapFromKeyNum( keyNum, dmap);
    for ( i = 0; i < 4; i++)
    {
#if TARGET_OS_MAC
        if ( (smap[i] & dmap[i]) != dmap[i]) return( false);
#else
        if ( (smap[i].bigEndianValue & dmap[i].bigEndianValue) != dmap[i].bigEndianValue) return( false);
#endif TARGET_OS_MAC
    }
    return( true);
}

void WaitForAnyEvent( void)

{
    KeyMap  keyMap;

    do
    {
        GetKeys(keyMap);
    } while (( !Button()) && ( !AnyRealKeyDown()));
    do {} while ( AnyEvent());
    FlushEvents(everyEvent, 0);
}

Boolean TimedWaitForAnyEvent( long time)

{
    long    starttime = TickCount();
    KeyMap  keyMap;
    Boolean result = false;
    
    do
    {
        GetKeys(keyMap);
    } while (( !Button()) && ( !AnyRealKeyDown()) && (( TickCount() - starttime) < time));
    if ( (Button()) || ( AnyRealKeyDown()))
    {
        do {} while ( AnyEvent());
        result = true;
    }
    FlushEvents(everyEvent, 0);
    return( result);
}

Boolean AnyEvent( void)

{
    KeyMap  keyMap;

    GetKeys(keyMap);
    if (( Button()) || (AnyRealKeyDown())) return ( TRUE);
    return( FALSE);
}

Boolean ControlKey( void)

{
    KeyMap  keyMap;
    
    GetKeys( keyMap);
#if TARGET_OS_MAC
    return( (keyMap[1] >> 3) & 0x01 );
#else
    return( (EndianU32_BtoN(keyMap[1].bigEndianValue) >> 3) & 0x01 );
#endif TARGET_OS_MAC
}

Boolean CommandKey( void)

{
    KeyMap  keyMap;
    
    GetKeys( keyMap);
#if TARGET_OS_MAC
    return( (keyMap[1] >> 15) & 0x01 );
#else
    return( (EndianU32_BtoN(keyMap[1].bigEndianValue) >> 15) & 0x01 );
#endif TARGET_OS_MAC
}

Boolean OptionKey( void)

{
    KeyMap  keyMap;
    
    GetKeys( keyMap);
#if TARGET_OS_MAC
    return( (keyMap[1] >> 2) & 0x01 );
#else
    return( (EndianU32_BtoN(keyMap[1].bigEndianValue) >> 2) & 0x01 );
#endif TARGET_OS_MAC
}

Boolean ShiftKey( void)

{
    KeyMap  keyMap;
    
    GetKeys( keyMap);
#if TARGET_OS_MAC
    return( keyMap[1] & 0x01 );
#else
    return( EndianU32_BtoN(keyMap[1].bigEndianValue) & 0x01 );
#endif TARGET_OS_MAC
}

Boolean EscapeKey( void)
{
    KeyMap  keyMap;
    
    GetKeys( keyMap);
#if TARGET_OS_MAC
    return( (keyMap[1] >> 13) & 0x01);
#else
    return( EndianU32_BtoN(keyMap[1].bigEndianValue) & 0x01 );
#endif TARGET_OS_MAC
}

Boolean PeriodKey( void)
{
    KeyMap  keyMap;
    
    GetKeys( keyMap);
#if TARGET_OS_MAC
    return( (keyMap[1] >> 23) & 0x01);
#else
    return( (EndianU32_BtoN(keyMap[1].bigEndianValue) >> 23) & 0x01);
#endif TARGET_OS_MAC
}

Boolean QKey( void)
{
    KeyMap  keyMap;
    
    GetKeys( keyMap);
    return (mQKey( keyMap));
}

Boolean AnyCancelKeys( void)
{
    return( EscapeKey() || (CommandKey() && ( PeriodKey() || QKey())));
}

void GetKeyNumName( StringPtr s, short keyNum)

{
    GetIndString( s, kKeyMapNameID, keyNum);
}

// returns true if any keys OTHER THAN POWER ON AND CAPS LOCK are down

Boolean AnyRealKeyDown( void)

{
    KeyMap  keyMap;
    
    GetKeys( keyMap);
    
#if TARGET_OS_MAC
    keyMap[3] &= ~0x80; // mask out power key
    keyMap[1] &= ~0x02; // mask out caps lock key
    
    if (( keyMap[0]) || ( keyMap[1]) || ( keyMap[2]) ||
        ( keyMap[3])) return ( TRUE);
#else
    keyMap[3].bigEndianValue &= EndianU32_NtoB(~0x80);  // mask out power key
    keyMap[1].bigEndianValue &= EndianU32_NtoB(~0x02);  // mask out caps lock key
    
    if (( keyMap[0].bigEndianValue) || ( keyMap[1].bigEndianValue) || ( keyMap[2].bigEndianValue) ||
        ( keyMap[3].bigEndianValue)) return ( TRUE);
#endif TARGET_OS_MAC
    else return ( FALSE);
}

Boolean AnyModifierKeyDown( void) // checks for shift, option, command, control
{
    if ( ControlKey()) return true;
    if ( OptionKey()) return true;
    if ( ShiftKey()) return true;
    if ( CommandKey()) return true;
    
    return false;
}

Boolean AnyKeyButThisOne( KeyMap keyMap, long whichWord, long whichBit)
{
    long    i;
    
    for ( i = 0; i < 4; i++)
    {
        if ( i != whichWord)
        {
#if TARGET_OS_MAC
            if ( keyMap[i] != 0) return true;
#else
            if ( keyMap[i].bigEndianValue != 0) return true;
#endif TARGET_OS_MAC
        } else
        {
#if TARGET_OS_MAC
            if ( (keyMap[i] & ~(0x00000001 << whichBit)) != 0) return true;
#else
            if ( (EndianU32_BtoN(keyMap[i].bigEndianValue) & ~(0x00000001 << whichBit)) != 0) return true;
#endif TARGET_OS_MAC
        }
    }
    
    return false;
}

long GetAsciiFromKeyMap( KeyMap sourceKeyMap, KeyMap previousKeyMap)
{
    short           whichKeyCode = 0, modifiers = 0, count;
    long            result;
    Ptr             KCHRPtr;
    KeyMap          keyMap;
    
    if ( previousKeyMap == nil)
    {
        for ( count = 0; count < 4; count++)
#if TARGET_OS_MAC
            keyMap[count] = sourceKeyMap[count];
    } else
    {
        keyMap[0] = sourceKeyMap[0] & (~previousKeyMap[0]);
        keyMap[1] = sourceKeyMap[1] & ((~previousKeyMap[1]) | kModifierKeyMask);
        keyMap[2] = sourceKeyMap[2] & (~previousKeyMap[2]);
        keyMap[3] = sourceKeyMap[3] & (~previousKeyMap[3]);
    }

    if ( keyMap[1] & 0x0008)
    {
        keyMap[1] &= ~0x0008;   
    }   // turn off control key
    
    if ( keyMap[1] & 0x8000)
    {
        modifiers |= cmdKey;
        keyMap[1] &= ~0x8000;   // turn off command key
    }
    
    if ( keyMap[1] & 0x0004)
    {
        modifiers |= optionKey;
        keyMap[1] &= ~0x0004;   // turn off option key
    }
    
    if ( keyMap[1] & 0x0001)
    {
        modifiers |= shiftKey;
        keyMap[1] &= ~0x0001;   // turn off shift key
    }
    
    if ( keyMap[1] & 0x0002)
    {
        modifiers |= alphaLock;
        keyMap[1] &= 0x0002;    // turn caps lock key
    }

#else
            keyMap[count].bigEndianValue = sourceKeyMap[count].bigEndianValue;
    } else
    {
        keyMap[0].bigEndianValue = sourceKeyMap[0].bigEndianValue & (~previousKeyMap[0].bigEndianValue);
        keyMap[1].bigEndianValue = sourceKeyMap[1].bigEndianValue & ((~previousKeyMap[1].bigEndianValue) | EndianU32_NtoB(kModifierKeyMask));
        keyMap[2].bigEndianValue = sourceKeyMap[2].bigEndianValue & (~previousKeyMap[2].bigEndianValue);
        keyMap[3].bigEndianValue = sourceKeyMap[3].bigEndianValue & (~previousKeyMap[3].bigEndianValue);
    }

    if ( keyMap[1].bigEndianValue & EndianU32_NtoB(0x0008))
    {
        keyMap[1].bigEndianValue &= EndianU32_NtoB(~0x0008);    
    }   // turn off control key
    
    if ( keyMap[1].bigEndianValue & EndianU32_NtoB(0x8000))
    {
        modifiers |= cmdKey;
        keyMap[1].bigEndianValue &= EndianU32_NtoB(~0x8000);    // turn off command key
    }
    
    if ( keyMap[1].bigEndianValue & EndianU32_NtoB(0x0004))
    {
        modifiers |= optionKey;
        keyMap[1].bigEndianValue &= EndianU32_NtoB(~0x0004);    // turn off option key
    }
    
    if ( keyMap[1].bigEndianValue & EndianU32_NtoB(0x0001))
    {
        modifiers |= shiftKey;
        keyMap[1].bigEndianValue &= EndianU32_NtoB(~0x0001);    // turn off shift key
    }
    
    if ( keyMap[1].bigEndianValue & EndianU32_NtoB(0x0002))
    {
        modifiers |= alphaLock;
        keyMap[1].bigEndianValue &= EndianU32_NtoB(0x0002); // turn caps lock key
    }

#endif TARGET_OS_MAC
    whichKeyCode = (GetKeyNumFromKeyMap( keyMap) - 1) | modifiers;
    KCHRPtr = (Ptr)GetScriptManagerVariable( smKCHRCache);
    result = KeyTranslate( KCHRPtr, whichKeyCode, &gKeyTranslateState);
    
    return( result);
}

long GetAsciiFromKeyNum( short keyNum)
{
    Ptr             KCHRPtr;
    
    KCHRPtr = (Ptr)GetScriptManagerVariable( smKCHRCache);
    return ( KeyTranslate( KCHRPtr, keyNum - 1, &gKeyTranslateState));
}
