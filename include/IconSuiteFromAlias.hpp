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

#ifndef ANTARES_ICON_SUITE_FROM_ALIAS_HPP_
#define ANTARES_ICON_SUITE_FROM_ALIAS_HPP_

/******************************************\
|**| IconSuiteFromAlias.h
\******************************************/

#pragma mark **DEFINITIONS**
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/

#pragma mark _macros_
/* - macros
*******************************************/

#pragma mark **TYPEDEFS**
/******************************************\
|**| typedefs
\******************************************/

#pragma mark **PUBLIC PROTOTYPES**
/******************************************\
|**| public function prototypes
\******************************************/

void DrawAliasSmallIcon( AliasHandle alias, short left, short right);
void DrawFSpecSmallIcon( FSSpec *fspec, short left, short top);
Handle GetIconSuiteFromAlias( AliasHandle alias);
OSErr GetIconSuiteFromFSSpec(FSSpecPtr hfsObj, Handle *iconSuite);

#endif // ANTARES_ICON_SUITE_FROM_ALIAS_HPP_
