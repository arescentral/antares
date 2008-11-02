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

#define	BLACK_COLOR(color)	{ (color)->red = (color)->blue = (color)->green = 0; }
#define WHITE_COLOR(color)	{ (color)->red = (color)->blue = (color)->green = 65535; }
#define	GRAY_13(color)		{ (color)->red = (color)->blue = (color)->green = 85120; }
#define GRAY_33(color)		{ (color)->red = (color)->blue = (color)->green = 21627; }
#define	GRAY_53(color)		{ (color)->red = (color)->blue = (color)->green = 34724; }
#define	GRAY_73(color)		{ (color)->red = (color)->blue = (color)->green = 47841; }
#define	GRAY_86(color)		{ (color)->red = (color)->blue = (color)->green = 56360; }
#define	HILITE_COLOR(color)	{ (color)->red = 65535; (color)->blue = 0; (color)->green = 52429; }
