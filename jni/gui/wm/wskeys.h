/*
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef MPLAYER_GUI_WSKEYS_H
#define MPLAYER_GUI_WSKEYS_H

#define wsKeyNone -1

#define wsosbrackets '['
#define wscsbrackets ']'

#define wsq 'q'
#define wsa 'a'
#define wsz 'z'
#define wsw 'w'
#define wss 's'
#define wsx 'x'
#define wse 'e'
#define wsd 'd'
#define wsr 'r'
#define wsf 'f'
#define wsv 'v'
#define wst 't'
#define wsg 'g'
#define wsb 'b'
#define wsy 'y'
#define wsh 'h'
#define wsn 'n'
#define wsu 'u'
#define wsj 'j'
#define wsm 'm'
#define wsi 'i'
#define wsk 'k'
#define wso 'o'
#define wsl 'l'
#define wsp 'p'
#define wsc 'c'

#define wsQ 'Q'
#define wsA 'A'
#define wsZ 'Z'
#define wsW 'W'
#define wsS 'S'
#define wsX 'X'
#define wsE 'E'
#define wsD 'D'
#define wsR 'R'
#define wsF 'F'
#define wsV 'V'
#define wsT 'T'
#define wsG 'G'
#define wsB 'B'
#define wsY 'Y'
#define wsH 'H'
#define wsN 'N'
#define wsU 'U'
#define wsJ 'J'
#define wsM 'M'
#define wsI 'I'
#define wsK 'K'
#define wsO 'O'
#define wsL 'L'
#define wsP 'P'
#define wsC 'C'

#define ws0 '0'
#define ws1 '1'
#define ws2 '2'
#define ws3 '3'
#define ws4 '4'
#define ws5 '5'
#define ws6 '6'
#define ws7 '7'
#define ws8 '8'
#define ws9 '9'

#define wsSpace ' '
#define wsMinus '-'
#define wsPlus  '+'
#define wsMul   '*'
#define wsDiv   '/'
#define wsLess  '<'
#define wsMore  '>'

#define wsUp            0x52 + 256
#define wsDown          0x54 + 256
#define wsLeft          0x51 + 256
#define wsRight         0x53 + 256
#define wsLeftCtrl      0xe3 + 256
#define wsRightCtrl     0xe4 + 256
#define wsLeftAlt       0xe9 + 256
#define wsRightAlt      0x7e + 256
#define wsLeftShift     0xe1 + 256
#define wsRightShift    0xe2 + 256
#define wsEnter         0x0d + 256
#define wsBackSpace     0x08 + 256
#define wsCapsLock      0xe5 + 256
#define wsTab           0x09 + 256
#define wsF1            0xbe + 256
#define wsF2            0xbf + 256
#define wsF3            0xc0 + 256
#define wsF4            0xc1 + 256
#define wsF5            0xc2 + 256
#define wsF6            0xc3 + 256
#define wsF7            0xc4 + 256
#define wsF8            0xc5 + 256
#define wsF9            0xc6 + 256
#define wsF10           0xc7 + 256
#define wsF11           0xc8 + 256
#define wsF12           0xc9 + 256
#define wsInsert        0x63 + 256
#define wsDelete        0xff + 256
#define wsHome          0x50 + 256
#define wsEnd           0x57 + 256
#define wsPageUp        0x55 + 256
#define wsPageDown      0x56 + 256
#define wsNumLock       0x7f + 256
#define wsEscape        0x1b + 256
#define wsGrayEnter     0x8d + 256
#define wsGrayPlus      0xab + 256
#define wsGrayMinus     0xad + 256
#define wsGrayMul       0xaa + 256
#define wsGrayDiv       0xaf + 256

#define wsGrayInsert    0xb0 + 256
#define wsGrayDelete    0xae + 256
#define wsGrayEnd       0xb1 + 256
#define wsGrayDown      0xb2 + 256
#define wsGrayPageDown  0xb3 + 256
#define wsGrayLeft      0xb4 + 256
#define wsGray5         0xb5 + 256
#define wsGrayRight     0xb6 + 256
#define wsGrayHome      0xb7 + 256
#define wsGrayUp        0xb8 + 256
#define wsGrayPageUp    0xb9 + 256

//Keys for multimedia keyboard

#define wsXF86LowerVolume 0x11 + 256
#define wsXF86RaiseVolume 0x13 + 256
#define wsXF86Mute        0x12 + 256
#define wsXF86Play        0x14 + 256
#define wsXF86Stop        0x15 + 256
#define wsXF86Prev        0x16 + 256
#define wsXF86Next        0x17 + 256
#define wsXF86Media       0x32 + 256

#define wsXFMMPrev        0x98
#define wsXFMMStop        0xb1
#define wsXFMMPlay	  0x95
#define wsXFMMNext        0x93
#define wsXFMMVolUp       0xad
#define wsXFMMVolDown     0xa6
#define wsXFMMMute        0x99

#define wsKeyNumber 130

typedef struct
{
 int    code;
 const char * name;
} TwsKeyNames;

extern const TwsKeyNames wsKeyNames[ wsKeyNumber ];

#endif /* MPLAYER_GUI_WSKEYS_H */
