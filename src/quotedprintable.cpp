/*
  Copyright (c) 2011-2012 - Tőkés Attila
  Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.
*/

#include "quotedprintable.h"

using namespace SimpleMail;

// ASCII character values used throughout
const unsigned char MinPrintableRange = 0x20;
const unsigned char MaxPrintableRange = 0x7e;
const unsigned char HorizontalTab = 0x09;
//const unsigned char LineFeed = 0x0a;
const unsigned char FormFeed = 0x0c;
//const unsigned char CarriageReturn = 0x0d;
const unsigned char Space = 0x20;
const unsigned char Equals = 0x3d;
const unsigned char ExclamationMark = 0x21;
const unsigned char Asterisk = 0x2a;
const unsigned char Plus = 0x2b;
const unsigned char Minus = 0x2d;
const unsigned char Slash = 0x2f;
const unsigned char Underscore = 0x5f;

static inline bool requiresEscape(unsigned char input, bool rfc2047)
{
    // For both, we need to escape '=' and anything unprintable
    bool escape = ((input > MaxPrintableRange) ||
                   ((input < MinPrintableRange) && (input != HorizontalTab) && (input != FormFeed)) ||
                   (input == Equals));

    // For RFC 2047, we need to escape '?', '_', ' ' & '\t'
    // In fact, since the output may be used in a header field 'word', then the only characters
    // that can be used un-escaped are: alphanumerics, '!', '*', '+' '-', '/' and '_'
    if (!escape && rfc2047)
    {
        // We can also ignore space, since it will become an underscore
        if ((input != ExclamationMark) && (input != Asterisk) && (input != Plus) &&
                (input != Minus) && (input != Slash) && (input != Underscore) && (input != Space))
        {
            escape = !isalnum(input);
        }
    }

    return escape;
}

QByteArray QuotedPrintable::encode(const QByteArray &input, bool rfc2047, int *printable, int *encoded)
{
    QByteArray output;

    quint8 byte;
    static const char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    for (int i = 0; i < input.length() ; ++i) {
        byte = quint8(input[i]);

        if (requiresEscape(byte, rfc2047)) {
            output.append('=');
            output.append(hex[((byte >> 4) & 0x0F)]);
            output.append(hex[(byte & 0x0F)]);
            if (encoded) {
                ++(*encoded);
            }
        } else {
            output.append(char(byte));
            if (printable) {
                ++(*printable);
            }
        }
    }

    return output;
}

QByteArray QuotedPrintable::decode(const QByteArray &input)
{
    //                           0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  @  A   B   C   D   E   F
    static const int hexVal[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};

    QByteArray output;

    int len = input.length();
    for (int i = 0; i < len-2; ++i) {
        if (input.at(i) == '=') {
            int x = input.at(i+1) - '0';
            int y = input.at(i+2) - '0';
            if (x >= 0 && y >= 0 && x < 23 && y < 23) {
                output.append(char((hexVal[x] << 4) + hexVal[y]));
            }
            else {
                output.append('=').append(char(x + '0')).append(char(y + '0'));
            }

            i += 2;
        } else {
            output.append(input.at(i));
        }
    }

    return output;
}
