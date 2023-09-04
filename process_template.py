#!/usr/bin/env python3

# Copyright 2023 Mustafa Serdar Sanli
#
# This file is part of FloatInfo.
#
# FloatInfo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# FloatInfo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with FloatInfo.  If not, see <https://www.gnu.org/licenses/>.

import sys

tmpl = None

def dump_enum(names):
    global tmpl
    for i, name in enumerate(names):
        tmpl = tmpl.replace('{' + name + '}', f'{i+1}')

def main():
    global tmpl
    tmpl = sys.stdin.read()

    dump_enum([
        'TMPL_BITTYPE_MANTISSA',
        'TMPL_BITTYPE_EXPONENT',
        'TMPL_BITTYPE_SIGN',
        'TMPL_BITTYPE_REGIME',
        'TMPL_BITTYPE_OOB',
    ])


    dump_enum([
        # 'TMPL_TYPE_BINARY8',
        'TMPL_TYPE_BINARY16',
        'TMPL_TYPE_BINARY32',
        'TMPL_TYPE_BINARY64',
        'TMPL_TYPE_POSIT8',
        'TMPL_TYPE_POSIT16',
        'TMPL_TYPE_POSIT32',
        'TMPL_TYPE_POSIT64',

        'TMPL_TYPE_MAX',
    ])

    dump_enum([
        'TMPL_IDENTIFIER_SIGN',
        'TMPL_IDENTIFIER_EXPONENT',
        'TMPL_IDENTIFIER_MANTISSA',
        'TMPL_IDENTIFIER_MBITS',
        'TMPL_IDENTIFIER_REGIME',
        'TMPL_IDENTIFIER_NORMALIZED',
        'TMPL_IDENTIFIER_EXPBIAS',
        'TMPL_IDENTIFIER_MAX',

        'TMPL_STRCODE_BITSTRING',
        'TMPL_STRCODE_BYTES_PRETTY',
        'TMPL_STRCODE_URLHASH',
        'TMPL_STRCODE_TYPENAME',
        'TMPL_STRCODE_TYPENAME_LONG',
        'TMPL_STRCODE_EXACT_BASE10',
        'TMPL_STRCODE_EXACT_BASE2',
        'TMPL_STRCODE_MATH',
        'TMPL_STRCODE_MAX',
    ])

    dump_enum([ 'TMPL_BOOL_IS_NORMAL', 'TMPL_BOOL_IS_DENORMAL',
        'TMPL_BOOL_IS_FRACTION',
        'TMPL_BOOL_IS_INTEGER',
        'TMPL_BOOL_IS_IEEE754',
        'TMPL_BOOL_IS_POSIT',
        'TMPL_BOOL_IS_ANY',
    ] + [
        f'TMPL_INT_BITTYPE_{i}' for i in range(64)
    ])

    dump_enum([
        'TMPL_SET_ZERO',
        'TMPL_SET_ONE',
        'TMPL_SET_NAR',
        'TMPL_SET_INF',
        'TMPL_SET_QNAN',
        'TMPL_SET_SNAN',
        'TMPL_SET_MIN',
        'TMPL_SET_MAX',
        'TMPL_SET_EPS',
        'TMPL_SET_DENORM_MIN',
        'TMPL_SET_NEGATE',
        'TMPL_SET_PREV',
        'TMPL_SET_NEXT',
        'TMPL_SET_MANTISSA_INCREMENT',
        'TMPL_SET_MANTISSA_DECREMENT',
        'TMPL_SET_EXPONENT_INCREMENT',
        'TMPL_SET_EXPONENT_DECREMENT',
        'TMPL_SET_REGIME_INCREMENT',
        'TMPL_SET_REGIME_DECREMENT',
        'TMPL_SET_REPRSTR',

    ] + [
        f'TMPL_SET_BIT_FLIP_{i}' for i in range(64)
    ])

    sys.stdout.write(tmpl)

if __name__ == '__main__':
    main()
