// Copyright 2023 Mustafa Serdar Sanli
//
// This file is part of FloatInfo.
//
// FloatInfo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// FloatInfo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with FloatInfo.  If not, see <https://www.gnu.org/licenses/>.

#include <algorithm>
#include <string.h>
#include <string>
#include <string_view>
#include <iostream>

#include "SimpleBigInt.cpp"

using namespace std::literals;


struct IEEE754Float16Traits
{
    static constexpr const char* TypeName = "binary16";
    static constexpr const char* TypeNameLong = "IEEE 754 16-bit Float (binary16)";
    static constexpr int NumBits = 16;
    static constexpr int NumExponentBits = 5;
    static constexpr int NumMantissaBits = 10;
};

struct IEEE754BFloat16Traits
{
    static constexpr const char* TypeName = "bfloat16";
    static constexpr const char* TypeNameLong = "Brain floating-point";
    static constexpr int NumBits = 16;
    static constexpr int NumExponentBits = 8;
    static constexpr int NumMantissaBits = 7;
};

struct IEEE754MinifloatTraits
{
    static constexpr const char* TypeName = "minifloat";
    static constexpr const char* TypeNameLong = "Minifloat";
    static constexpr int NumBits = 8;
    static constexpr int NumExponentBits = 4;
    static constexpr int NumMantissaBits = 3;
};

struct IEEE754Float32Traits
{
    static constexpr const char* TypeName = "binary32";
    static constexpr const char* TypeNameLong = "IEEE 754 32-bit Float (binary32)";
    static constexpr int NumBits = 32;
    static constexpr int NumExponentBits = 8;
    static constexpr int NumMantissaBits = 23;
};

struct IEEE754Float64Traits
{
    static constexpr const char* TypeName = "binary64";
    static constexpr const char* TypeNameLong = "IEEE 754 64-bit Float (binary64)";
    static constexpr int NumBits = 64;
    static constexpr int NumExponentBits = 11;
    static constexpr int NumMantissaBits = 52;
};

struct Posit8Traits
{
    static constexpr const char* TypeName = "posit8";
    static constexpr const char* TypeNameLong = "8-bit Posit (Type III Unum) (posit8)";
    static constexpr int NumBits = 8;
};

struct Posit16Traits
{
    static constexpr const char* TypeName = "posit16";
    static constexpr const char* TypeNameLong = "16-bit Posit (Type III Unum) (posit16)";
    static constexpr int NumBits = 16;
};

struct Posit32Traits
{
    static constexpr const char* TypeName = "posit32";
    static constexpr const char* TypeNameLong = "32-bit Posit (Type III Unum) (posit32)";
    static constexpr int NumBits = 32;
};

struct Posit64Traits
{
    static constexpr const char* TypeName = "posit64";
    static constexpr const char* TypeNameLong = "64-bit Posit (Type III Unum) (posit64)";
    static constexpr int NumBits = 64;
};

struct Editor {
    virtual std::string GetStringImpl(int code) const = 0;
    virtual int GetInt(int code) const = 0;
    virtual void SetValue(int code, const char *) = 0;

    const char* GetString(int code) const
    {
        auto &c = _cachedStrings[code];
        if (c.version != _version)
        {
            c.value = GetStringImpl(code);
            c.version = _version;
        }
        return c.value.c_str();
    }

    struct CachedString
    {
        uint64_t version = 0;
        std::string value;
    };

    uint64_t _version = 1; // for caching
    mutable CachedString _cachedStrings[{TMPL_STRCODE_MAX}];
};

template <typename TraitsType>
struct CommonRepr
{
    // Code that can be shared by all float types
    static constexpr int NumBits = TraitsType::NumBits;
    static_assert(NumBits % 8 == 0);

    static constexpr int NumBytes = NumBits / 8;

    static constexpr const char ToHex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    static uint32_t FromHex(char c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return 16;
    }

    static std::string GetBitString(uint64_t val)
    {
        std::string res;
        for (int i = 0; i < NumBits; ++i)
        {
            res.push_back((val & (1ull << i)) ? '1' : '0');
        }
        return res;
    }

    static std::string GetByteString(uint64_t val)
    {
        uint8_t bytes[8];
        memcpy(bytes, &val, 8);
        std::string res;
        for (int i = 0; i < NumBytes; ++i)
        {
            res.push_back(ToHex[bytes[i]/16]);
            res.push_back(ToHex[bytes[i]%16]);
        }
        return res;
    }

    static std::string GetBytesPretty(uint64_t val)
    {
        uint8_t bytes[8];
        memcpy(bytes, &val, 8);
        std::string res;
        for (int i = 0; i < NumBytes; ++i)
        {
            res.push_back('0');
            res.push_back('x');
            res.push_back(ToHex[bytes[i]/16]);
            res.push_back(ToHex[bytes[i]%16]);
            if (i < NumBytes - 1)
            {
                res.push_back(' ');
            }
        }
        if (NumBytes != 1)
        {
            res += " (little endian)";
        }
        return res;
    }

    static std::string ToReprString(uint64_t val)
    {
        return "hex:" + GetByteString(val);
    }

    static uint64_t FromReprString(std::string_view valstr)
    {
        uint8_t bytes[8] = {};
        if (valstr.size() == 4 + 2 * NumBytes && valstr.substr(0, 4) == "hex:")
        {
            for (int i = 0; i < NumBytes; ++i)
            {
                uint32_t h1 = FromHex(valstr[4+2*i]);
                uint32_t h2 = FromHex(valstr[5+2*i]);
                if (h1 == 16 || h2 == 16) return 0;
                bytes[i] = h1 << 4 | h2;
            }
        }
        uint64_t res;
        memcpy(&res, bytes, 8);
        return res;
    }
};

template <typename TraitsType>
struct IEEE754FloatRepresentation : CommonRepr<TraitsType>
{
    using Self = IEEE754FloatRepresentation<TraitsType>;

    static constexpr int NumMantissaBits = TraitsType::NumMantissaBits;
    static constexpr int NumExponentBits = TraitsType::NumExponentBits;
    static_assert(NumMantissaBits + NumExponentBits + 1 == Self::NumBits);

    static constexpr int ExponentBias = (1ull << (NumExponentBits - 1)) - 1;
    static constexpr int ExponentForEpsilon = ExponentBias - NumMantissaBits;
    static constexpr int ExponentForULP1 = ExponentBias + NumMantissaBits;

    static constexpr int SignShift = NumMantissaBits + NumExponentBits;
    static constexpr int64_t SignMask = 1ull;

    static constexpr int ExponentShift = NumMantissaBits;
    static constexpr int64_t ExponentMask = (1ull << NumExponentBits) - 1ull;

    static constexpr int MantissaShift = 0;
    static constexpr int64_t MantissaMask = (1ull << NumMantissaBits) - 1ull;

    static uint64_t GetSign(    uint64_t val) { return (val >>     SignShift) &     SignMask; }
    static uint64_t GetExponent(uint64_t val) { return (val >> ExponentShift) & ExponentMask; }
    static uint64_t GetMantissa(uint64_t val) { return (val >> MantissaShift) & MantissaMask; }

    static uint64_t Construct(uint64_t sign, uint64_t exponent, uint64_t mantissa)
    {
        return ((sign     &     SignMask) <<     SignShift)
             | ((exponent & ExponentMask) << ExponentShift)
             | ((mantissa & MantissaMask) << MantissaShift);
    }

    static bool IsNanOrInf(uint64_t val)
    {
        return GetExponent(val) == ExponentMask;
    }

    static uint64_t PositiveInfinity() { return Construct(0,   ExponentMask,         0                               ); }
    static uint64_t NegativeInfinity() { return Construct(1,   ExponentMask,         0                               ); }
    static uint64_t QuietNan()         { return Construct(0,   ExponentMask,         1ull << (NumMantissaBits - 1)   ); }
    static uint64_t SignalingNan()     { return Construct(0,   ExponentMask,         1ull << (NumMantissaBits - 2)   ); }
    static uint64_t MinNormalized()    { return Construct(0,   1,                    0                               ); }
    static uint64_t MaxFinite()        { return Construct(0,   ExponentMask - 1,     MantissaMask                    ); }
    static uint64_t MinDenormalized()  { return Construct(0,   0,                    1                               ); }
    static uint64_t Zero()             { return Construct(0,   0,                    0                               ); }
    static uint64_t One()              { return Construct(0,   ExponentBias,         0                               ); }
    static uint64_t Epsilon()          { return Construct(0,   ExponentForEpsilon,   0                               ); }

    static uint64_t DecrementMantissa(uint64_t val) { return Construct( GetSign(val), GetExponent(val),   GetMantissa(val)-1 ); }
    static uint64_t IncrementMantissa(uint64_t val) { return Construct( GetSign(val), GetExponent(val),   GetMantissa(val)+1 ); }
    static uint64_t DecrementExponent(uint64_t val) { return Construct( GetSign(val), GetExponent(val)-1, GetMantissa(val)   ); }
    static uint64_t IncrementExponent(uint64_t val) { return Construct( GetSign(val), GetExponent(val)+1, GetMantissa(val)   ); }

    static uint64_t Negate(uint64_t val)
    {
        if (IsNanOrInf(val))
        {
            return val;
        }
        return Construct(1-GetSign(val), GetExponent(val), GetMantissa(val));
    }

    static uint64_t Next(uint64_t val)
    {
        uint64_t mantissa = GetMantissa(val);
        uint64_t exponent = GetExponent(val);
        uint64_t sign = GetSign(val);

        if (sign == 0)
        {
            // Positive number, increment value
            if (IsNanOrInf(val))
            {
                return val; // no more room
            }

            if (mantissa == MantissaMask)
            {
                mantissa = 0;
                exponent += 1;
            }
            else
            {
                mantissa += 1;
            }

            return Construct(sign, exponent, mantissa);
        }
        else
        {
            // Negative, if zero, set to positive zero, if inf set to -MaxFinite, else decrement

            if (IsNanOrInf(val))
            {
                if (mantissa == 0) // Inf
                {
                    return Construct(1ull, ExponentMask-1, MantissaMask);
                }
                else
                {
                    return val; // NaN
                }
            }

            if (exponent == 0 && mantissa == 0)
            {
                return Zero();
            }

            if (mantissa == 0)
            {
                mantissa = MantissaMask;
                exponent--;
            }
            else
            {
                mantissa--;
            }

            return Construct(sign, exponent, mantissa);
        }
    }

    static uint64_t Prev(uint64_t val)
    {
        val ^= (SignMask << SignShift);
        val = Next(val);
        val ^= (SignMask << SignShift);
        return val;
    }

    static SimpleNumber GetValue(uint64_t val)
    {
        SimpleNumber res;

        if (IsNanOrInf(val))
        {
            return res; // N/A
        }

        bool isDenormal = (GetExponent(val) == 0);

        SimpleNumber expTerm = SimpleNumber::pow2((int)GetExponent(val) - ExponentBias + isDenormal);

        SimpleNumber mTerm = SimpleNumber(GetMantissa(val)) * SimpleNumber::pow2(-TraitsType::NumMantissaBits);
        if (!isDenormal)
        {
            mTerm = mTerm + SimpleNumber::One();
        }

        res = expTerm * mTerm;
        res._isNegative = GetSign(val);
        return res;
    }
};

template <typename TraitsType>
struct PositRepresentation : CommonRepr<TraitsType>
{
    using Self = PositRepresentation<TraitsType>;

    static constexpr uint64_t BitMask = (~0ull) >> (sizeof(uint64_t)*8 - Self::NumBits);
    static constexpr int MinRegime = -Self::NumBits + 1;
    static constexpr int MaxRegime = Self::NumBits - 2;

    static uint64_t Zero() { return 0; }
    static uint64_t One() { return 1ull << (Self::NumBits - 2); }
    static uint64_t MinPositive() { return 1; }
    static uint64_t MaxFinite() { return BitMask >> 1; }
    static uint64_t NaR() { return 1ull << (Self::NumBits - 1); }
    static uint64_t Epsilon() { return 5ull << (Self::NumBits - 1 - 2 - Self::NumBits / 4); }

    static uint64_t Next(uint64_t val) { return (val + 1) & BitMask; }
    static uint64_t Prev(uint64_t val) { return (val - 1) & BitMask; }

    static uint64_t Negate(uint64_t val)
    {
        if (val == NaR() || val == Zero())
        {
            return val;
        }

        return (0ull - val) & BitMask;
    }

    static uint64_t DecrementRegime(uint64_t val)
    {
        int regime = GetRegime(val);
        if (regime == MinRegime)
        {
            return val;
        }
        if (regime > 0)
        {
            return ((val << 1) & (BitMask >> 1)) | (GetSignBit(val) << (Self::NumBits - 1));
        }
        if (regime == 0)
        {
            return val ^ (3ull << (Self::NumBits - 3));
        }
        else
        {
            // regime < 0
            return ((val >> 1) & (BitMask >> 2)) | (GetSignBit(val) << (Self::NumBits - 1));
        }
    }

    static uint64_t IncrementRegime(uint64_t val)
    {
        int regime = GetRegime(val);
        if (regime == MaxRegime)
        {
            return val;
        }
        if (regime == MinRegime)
        {
            return val | 1ull;
        }
        if (regime >= 0)
        {
            return ((val >> 1) & (BitMask >> 2)) | (1ull << (Self::NumBits - 2))  | (GetSignBit(val) << (Self::NumBits - 1));
        }
        if (regime == -1)
        {
            return val ^ (3ull << (Self::NumBits - 3));
        }
        else
        {
            // regime < -1
            return ((val << 1) & (BitMask >> 1)) | (GetSignBit(val) << (Self::NumBits - 1));
        }
    }

    static uint64_t DecrementMantissa(uint64_t val)
    {
        int mb = NumMantissaBits(val);
        return (val >> mb << mb) | ((val-1ull) & ((1ull << mb) - 1));
    }
    static uint64_t IncrementMantissa(uint64_t val)
    {
        int mb = NumMantissaBits(val);
        return (val >> mb << mb) | ((val+1ull) & ((1ull << mb) - 1));
    }

    static uint64_t DecrementExponent(uint64_t val)
    {
        int eb = NumExponentBits(val);
        int mb = NumMantissaBits(val);
        if (eb == 0) return val;
        if (eb == 1) return val ^ 1ull;

        uint64_t newexpo = (GetExponent(val) - 1ull) & 3ull;
        return (val >> (eb + mb) << (eb + mb)) | (newexpo << mb) | GetMantissa(val);
    }

    static uint64_t IncrementExponent(uint64_t val)
    {
        int eb = NumExponentBits(val);
        int mb = NumMantissaBits(val);
        if (eb == 0) return val;
        if (eb == 1) return val ^ 1ull;

        uint64_t newexpo = (GetExponent(val) + 1ull) & 3ull;
        return (val >> (eb + mb) << (eb + mb)) | (newexpo << mb) | GetMantissa(val);
    }




    static uint64_t GetBit(uint64_t val, int bitIdx)
    {
        if (bitIdx < 0) return 0; // Implicit for exponent
        return (val >> bitIdx) & 1ull;
    }

    static int NumRegimeBits(uint64_t val)
    {
        int numRegimeBits = 0;
        uint32_t regimeFirstBit = GetRegimeFirstBit(val);
        for (int i = Self::NumBits - 2; i >= 0; --i)
        {
            uint32_t regimeBit = (val >> i) & 1ull;
            ++numRegimeBits;

            if (regimeBit != regimeFirstBit)
            {
                break;
            }
        }
        return numRegimeBits;
    }

    static int NumExponentBits(uint64_t val)
    {
        return std::min(2, Self::NumBits - 1 - NumRegimeBits(val));
    }

    static int NumMantissaBits(uint64_t val)
    {
        return std::max(0, Self::NumBits - 1 - NumRegimeBits(val) - 2);
    }


    static uint64_t GetSignBit(uint64_t val)
    {
        return GetBit(val, Self::NumBits - 1);
    }

    static uint32_t GetRegimeFirstBit(uint64_t val)
    {
        return GetBit(val, Self::NumBits - 2);
    }

    static int GetRegime(uint64_t val)
    {
        int regBits = NumRegimeBits(val);
        uint32_t reg0 = GetRegimeFirstBit(val);
        if (reg0 == GetBit(val, Self::NumBits - 1 - regBits))
        {
            regBits += 1;
        }
        regBits -= 1;

        if (reg0 == 1)
        {
            return regBits-1;
        }
        else
        {
            return -regBits;
        }
    }

    static int GetExponent(uint64_t val)
    {
        int numRegimeBits = NumRegimeBits(val);
        return (GetBit(val, Self::NumBits - 2 - numRegimeBits) << 1) | GetBit(val, Self::NumBits - 3 - numRegimeBits);
    }

    static uint64_t GetMantissa(uint64_t val)
    {
        int mBits = NumMantissaBits(val);
        uint64_t mask = (1ull << mBits) - 1;
        return val & mask;
    }

    static SimpleNumber GetValue(uint64_t val)
    {
        int regime = GetRegime(val);
        if (regime == -Self::NumBits+1)
        {
            return SimpleNumber();
        }

        bool isPositive = (GetSignBit(val) == 0);

        // implicit term can be 1 or -2, since bigint does not support negative numbers or substraction,
        // if it is -2, compute it by complementing bigint and adding it to one

        SimpleNumber fractionTerm;
        if (isPositive)
        {
            SimpleNumber fraction = SimpleNumber(GetMantissa(val)) * SimpleNumber::pow2(-NumMantissaBits(val));
            fractionTerm = SimpleNumber::One() + fraction;
        }
        else
        {
            uint64_t mantissa = GetMantissa(val);
            if (mantissa == 0)
            {
                fractionTerm = SimpleNumber::Two();
            }
            else
            {
                int numMantissaBits = NumMantissaBits(val);
                uint64_t mantissaComplement = ((~mantissa) + 1ull) & ((1ull << numMantissaBits) - 1);
                SimpleNumber fraction = SimpleNumber(mantissaComplement) * SimpleNumber::pow2(-numMantissaBits);
                fractionTerm = SimpleNumber::One() + fraction;
            }
        }

        int p = (1 - 2 * (int)GetSignBit(val)) * (4 * GetRegime(val) + GetExponent(val) + (int)GetSignBit(val));
        SimpleNumber powTerm = SimpleNumber::pow2(p);

        SimpleNumber res = fractionTerm * powTerm;
        res._isNegative = !isPositive;
        return res;
    }
};

template <typename TraitsType>
struct IEEE754FloatEditor : Editor {

    using ReprType = IEEE754FloatRepresentation<TraitsType>;

    static constexpr int NumBits = TraitsType::NumBits;
    static constexpr int NumMantissaBits = TraitsType::NumMantissaBits;
    static constexpr int NumExponentBits = TraitsType::NumExponentBits;


    // "Final" form of float is (sign) A * 2 ** B

    std::string GetStringImpl(int code) const override
    {
        switch (code)
        {
        case {TMPL_IDENTIFIER_SIGN}: return std::to_string(ReprType::GetSign(_repr));
        case {TMPL_IDENTIFIER_EXPONENT}: return std::to_string(ReprType::GetExponent(_repr));
        case {TMPL_IDENTIFIER_EXPBIAS}: return std::to_string(ReprType::ExponentBias);
        case {TMPL_IDENTIFIER_MANTISSA}: return std::to_string(ReprType::GetMantissa(_repr));
        case {TMPL_IDENTIFIER_MBITS}: return std::to_string(NumMantissaBits);
        case {TMPL_IDENTIFIER_NORMALIZED}:
        {
            if (ReprType::IsNanOrInf(_repr)) return "N/A";
            return ReprType::GetExponent(_repr) == 0 ? "0" : "1";
        }
        case {TMPL_STRCODE_TYPENAME}: return TraitsType::TypeName;
        case {TMPL_STRCODE_TYPENAME_LONG}: return TraitsType::TypeNameLong;
        case {TMPL_STRCODE_BITSTRING}:  return ReprType::GetBitString(_repr);
        case {TMPL_STRCODE_BYTES_PRETTY}: return ReprType::GetBytesPretty(_repr);

        case {TMPL_STRCODE_EXACT_BASE10}:
        case {TMPL_STRCODE_EXACT_BASE2}:
        {
            if (ReprType::IsNanOrInf(_repr))
            {
                if (ReprType::GetMantissa(_repr) == 0)
                {
                    return ReprType::GetSign(_repr) ? "-inf" : "inf";
                }

                if (ReprType::GetMantissa(_repr) >> (ReprType::NumMantissaBits - 1))
                {
                    // when float is quiet nan with all other mantisa bits zero, and sign bit is set,
                    // Microsoft (and clang as they seem to use same code for charconv) prints it as "nan(ind)"
                    return "Quiet NaN";
                }
                else
                {
                    return "Signaling NaN";
                }
            }

            constexpr uint32_t MaxDigitsAfterDot = ReprType::ExponentForULP1 - 1;
            int digitsAfterDot = (ReprType::ExponentForULP1 - ReprType::GetExponent(_repr)) - (ReprType::GetExponent(_repr) == 0);
            if (digitsAfterDot <= 0)
            {
                digitsAfterDot = 0;
            }
            // Nice thing is digitsAfterDot is same for both base2 and base10

            if (code == {TMPL_STRCODE_EXACT_BASE10})
            {
                return _value.render10(digitsAfterDot);
            }
            else
            {
                return _value.render2(digitsAfterDot);
            }
        }
        case {TMPL_STRCODE_URLHASH}: return "#"s + TraitsType::TypeName + "=" + ReprType::ToReprString(_repr);
        case {TMPL_STRCODE_MATH}: return _math;
        }
        return "error";
    }

    int GetInt(int code) const override
    {
        switch (code)
        {
            case {TMPL_BOOL_IS_IEEE754}:
                return 1;
            case {TMPL_BOOL_IS_ANY}:
                return 1;
            case {TMPL_BOOL_IS_POSIT}:
                return 0;
            case {TMPL_BOOL_IS_NORMAL}:
                return ReprType::GetExponent(_repr) > 0 && !ReprType::IsNanOrInf(_repr);
            case {TMPL_BOOL_IS_DENORMAL}:
                return ReprType::GetExponent(_repr) == 0;
            case {TMPL_BOOL_IS_FRACTION}:
                return ReprType::GetExponent(_repr) < ReprType::ExponentForULP1;
            case {TMPL_BOOL_IS_INTEGER}:
                return ReprType::GetExponent(_repr) >= ReprType::ExponentForULP1;
            case {TMPL_INT_BITTYPE_0} ... {TMPL_INT_BITTYPE_63}:
            {
                uint32_t bitIdx = code - {TMPL_INT_BITTYPE_0};
                if (bitIdx < NumMantissaBits)
                {
                    return {TMPL_BITTYPE_MANTISSA};
                }

                if (bitIdx < NumMantissaBits + NumExponentBits)
                {
                    return {TMPL_BITTYPE_EXPONENT};
                }

                if (bitIdx < NumMantissaBits + NumExponentBits + 1)
                {
                    return {TMPL_BITTYPE_SIGN};
                }

                return {TMPL_BITTYPE_OOB};
            }
        }
        return -1;
    }

    void SetValue(int code, const char *valstr) override
    {
        ++_version;
        switch (code)
        {
            case {TMPL_SET_ZERO}:       _repr = ReprType::Zero();                 break;
            case {TMPL_SET_ONE}:        _repr = ReprType::One();                    break;
            case {TMPL_SET_INF}:        _repr = ReprType::PositiveInfinity(); break;
            case {TMPL_SET_QNAN}:       _repr = ReprType::QuietNan(); break;
            case {TMPL_SET_SNAN}:       _repr = ReprType::SignalingNan(); break;
            case {TMPL_SET_MIN}:        _repr = ReprType::MinNormalized(); break;
            case {TMPL_SET_MAX}:        _repr = ReprType::MaxFinite(); break;
            case {TMPL_SET_EPS}:        _repr = ReprType::Epsilon();  break;
            case {TMPL_SET_DENORM_MIN}: _repr = ReprType::MinDenormalized(); break;
            case {TMPL_SET_NEGATE}:     _repr = ReprType::Negate(_repr); break;
            case {TMPL_SET_PREV}:       _repr = ReprType::Prev(_repr); break;
            case {TMPL_SET_NEXT}:       _repr = ReprType::Next(_repr); break;
            case {TMPL_SET_MANTISSA_DECREMENT}: _repr = ReprType::DecrementMantissa(_repr); break;
            case {TMPL_SET_MANTISSA_INCREMENT}: _repr = ReprType::IncrementMantissa(_repr); break;
            case {TMPL_SET_EXPONENT_DECREMENT}: _repr = ReprType::DecrementExponent(_repr); break;
            case {TMPL_SET_EXPONENT_INCREMENT}: _repr = ReprType::IncrementExponent(_repr); break;
            case {TMPL_SET_BIT_FLIP_0} ... {TMPL_SET_BIT_FLIP_63}:
            {
                uint32_t bitIdx = code - {TMPL_SET_BIT_FLIP_0};
                if (bitIdx < NumBits)
                {
                    _repr ^= (1ull << bitIdx);
                }
                break;
            }
            case {TMPL_SET_REPRSTR}: _repr = ReprType::FromReprString(valstr); break;
        }

        recompute();
    }

    void recompute()
    {
        _value = ReprType::GetValue(_repr);

        _math.clear();

        if (ReprType::IsNanOrInf(_repr))
        {
            return;
        }
        else
        {
            for (int rep = 0; rep <= 1; ++rep)
            {

                _math += "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\">";
                _math +=  "<mrow>";
                _math +=    "<mi>value</mi>";
                _math +=    "<mo>=</mo>";
                _math +=    "<mrow>";
                _math +=     "<msup>";
                _math +=      "<mrow>";
                _math +=       "<mo>(</mo>";
                _math +=       "<mrow>";
                _math +=        "<mrow>";
                _math +=         "<mo>−</mo>";
                _math +=         "<mn>1</mn>";
                _math +=        "</mrow>";
                _math +=       "</mrow>";
                _math +=       "<mo>)</mo>";
                _math +=      "</mrow>";
                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_SIGN}\">sign</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_SIGN}\">" + std::to_string(ReprType::GetSign(_repr)) + "</mn>";
                _math +=     "</msup>";
                _math +=     "<mo>×</mo>";
                _math +=     "<msup>";
                _math +=      "<mn>2</mn>";
                _math +=      "<mrow>";
                _math +=       "<mo>(</mo>";
                _math +=       "<mrow>";
                _math +=        "<mrow>";
                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_EXPONENT}\">exponent</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_EXPONENT}\">" + std::to_string(ReprType::GetExponent(_repr)) + "</mn>";
                _math +=         "<mo>−</mo>";
                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_EXPBIAS}\">expbias</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_EXPBIAS}\">" + std::to_string(ReprType::ExponentBias) + "</mn>";
                _math +=         "<mo>+</mo>";
                _math +=         "<mn>1</mn>";
                _math +=         "<mo>-</mo>";
                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_NORMALIZED}\">N</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_NORMALIZED}\">" + (ReprType::GetExponent(_repr) == 0 ? "0"s : "1"s) + "</mn>";
                _math +=        "</mrow>";
                _math +=       "</mrow>";
                _math +=       "<mo>)</mo>";
                _math +=      "</mrow>";
                _math +=     "</msup>";
                _math +=     "<mo>×</mo>";
                _math +=     "<mrow>";
                _math +=      "<mo>(</mo>";
                _math +=      "<mrow>";
                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_NORMALIZED}\">N</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_NORMALIZED}\">" + (ReprType::GetExponent(_repr) == 0 ? "0"s : "1"s) + "</mn>";
                _math +=       "<mo>+</mo>";
                _math +=       "<mfrac>";
                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_MANTISSA}\">mantissa</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_MANTISSA}\">" + std::to_string(ReprType::GetMantissa(_repr)) + "</mn>";
                _math +=        "<msup>";
                _math +=         "<mn>2</mn>";
                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_MBITS}\">mbits</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_MBITS}\">" + std::to_string(ReprType::NumMantissaBits) + "</mn>";
                _math +=        "</msup>";
                _math +=       "</mfrac>";
                _math +=      "</mrow>";
                _math +=      "<mo>)</mo>";
                _math +=     "</mrow>";
                _math +=    "</mrow>";
                _math +=   "</mrow>";
                _math += "</math>";
            }


            bool isNeg = ReprType::GetSign(_repr);

            SimpleNumber finalEqInt(ReprType::GetMantissa(_repr));
            if (ReprType::GetExponent(_repr) != 0)
            {
                finalEqInt = finalEqInt + SimpleNumber::pow2(NumMantissaBits);
            }

            int finalEqExp = ((int)ReprType::GetExponent(_repr) - ReprType::ExponentBias - (int)NumMantissaBits + (ReprType::GetExponent(_repr) == 0));

            SimpleNumber finalEqDenom;
            if (finalEqExp < 0)
            {
                finalEqDenom = SimpleNumber::pow2(-finalEqExp);
            }


            _math += "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\">";
            _math +=  "<mrow>";
            _math +=    "<mo>=</mo>";
            if (isNeg) _math += "<mo>-</mo>";
            _math +=    "<mn>" + finalEqInt.render10() + "</mn>";
            _math +=    "<mo>×</mo>";
            _math +=    "<msup>";
            _math +=     "<mn>2</mn>";
            _math +=     "<mn>" + std::to_string(finalEqExp) + "</mn>";
            _math +=    "</msup>";
            _math +=  "</mrow>";
            _math += "</math>";

            if (finalEqExp < 0)
            {
                _math += "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\">";
                _math +=  "<mrow>";
                _math +=   "<mo>=</mo>";
                if (isNeg) _math += "<mo>-</mo>";
                _math +=   "<mfrac>";
                _math +=    "<mn>" + finalEqInt.render10() + "</mn>";
                _math +=    "<mn>" + finalEqDenom.render10() + "</mn>";
                _math +=   "</mfrac>";
                _math +=  "</mrow>";
                _math += "</math>";
            }
        }
    }

    uint64_t _repr;
    SimpleNumber _value;
    std::string _math;
};

template <typename TraitsType>
struct PositEditor : Editor
{
    using ReprType = PositRepresentation<TraitsType>;

    static constexpr int NumBits = TraitsType::NumBits;

    std::string GetStringImpl(int code) const override
    {
        switch (code)
        {
        case {TMPL_IDENTIFIER_SIGN}: return std::to_string(ReprType::GetSignBit(_repr));
        case {TMPL_IDENTIFIER_EXPONENT}: return std::to_string(ReprType::GetExponent(_repr));
        case {TMPL_IDENTIFIER_MANTISSA}: return std::to_string(ReprType::GetMantissa(_repr));
        case {TMPL_IDENTIFIER_MBITS}: return std::to_string(ReprType::NumMantissaBits(_repr));
        case {TMPL_IDENTIFIER_REGIME}: return std::to_string(ReprType::GetRegime(_repr));

        case {TMPL_STRCODE_BITSTRING}:  return ReprType::GetBitString(_repr);
        case {TMPL_STRCODE_BYTES_PRETTY}: return ReprType::GetBytesPretty(_repr);
        case {TMPL_STRCODE_URLHASH}: return "#"s + TraitsType::TypeName + "=" + ReprType::ToReprString(_repr);
        case {TMPL_STRCODE_TYPENAME}: return TraitsType::TypeName;
        case {TMPL_STRCODE_TYPENAME_LONG}: return TraitsType::TypeNameLong;
        case {TMPL_STRCODE_EXACT_BASE10}:
        case {TMPL_STRCODE_EXACT_BASE2}:
        {
            if (_repr == (1ull << (NumBits - 1))) return "NaR";

            int sign = ReprType::GetSignBit(_repr);
            int mbits = ReprType::NumMantissaBits(_repr);
            int p = (1-2*sign) * (4*ReprType::GetRegime(_repr) + ReprType::GetExponent(_repr) + sign) - mbits;
            int digitsAfterDot = -p;
            if (digitsAfterDot <= 0)
            {
                digitsAfterDot = 0;
            }

            if (code == {TMPL_STRCODE_EXACT_BASE10})
            {
                return _value.render10(digitsAfterDot);
            }
            else
            {
                return _value.render2(digitsAfterDot);
            }
        }
        case {TMPL_STRCODE_MATH}: return _math;
        }

        return "error";
    }

    int GetInt(int code) const override
    {
        switch (code)
        {
            case {TMPL_BOOL_IS_IEEE754}:
                return 0;
            case {TMPL_BOOL_IS_ANY}:
                return 1;
            case {TMPL_BOOL_IS_POSIT}:
                return 1;
            case {TMPL_INT_BITTYPE_0} ... {TMPL_INT_BITTYPE_63}:
            {
                uint32_t bitIdx = code - {TMPL_INT_BITTYPE_0};
                if (bitIdx >= NumBits)
                {
                    return {TMPL_BITTYPE_OOB};
                }

                if (bitIdx == NumBits - 1)
                {
                    return {TMPL_BITTYPE_SIGN};
                }

                int numRegimeBits = ReprType::NumRegimeBits(_repr);

                if (bitIdx >= NumBits - 1 - numRegimeBits)
                {
                    return {TMPL_BITTYPE_REGIME};
                }

                if ((int)bitIdx >= (int)NumBits - 1 - (int)numRegimeBits - 2)
                {
                    return {TMPL_BITTYPE_EXPONENT};
                }

                return {TMPL_BITTYPE_MANTISSA};
            }
        }
        return -1;
    }

    void SetValue(int code, const char *valstr) override
    {
        ++_version;
        switch (code)
        {
            case {TMPL_SET_ZERO}:       _repr = ReprType::Zero(); break;
            case {TMPL_SET_ONE}:        _repr = ReprType::One(); break;
            case {TMPL_SET_MIN}:        _repr = ReprType::MinPositive(); break;
            case {TMPL_SET_MAX}:        _repr = ReprType::MaxFinite(); break;
            case {TMPL_SET_NAR}:        _repr = ReprType::NaR(); break;
            case {TMPL_SET_NEGATE}:     _repr = ReprType::Negate(_repr); break;
            case {TMPL_SET_PREV}:       _repr = ReprType::Prev(_repr); break;
            case {TMPL_SET_NEXT}:       _repr = ReprType::Next(_repr); break;
            case {TMPL_SET_EPS}:        _repr = ReprType::Epsilon();  break;
            case {TMPL_SET_BIT_FLIP_0} ... {TMPL_SET_BIT_FLIP_63}:
            {
                uint32_t bitIdx = code - {TMPL_SET_BIT_FLIP_0};
                if (bitIdx < NumBits)
                {
                    _repr ^= (((uint64_t)1ul) << bitIdx);
                }
                break;
            }
            case {TMPL_SET_REGIME_DECREMENT}:   _repr = ReprType::DecrementRegime(   _repr ); break;
            case {TMPL_SET_REGIME_INCREMENT}:   _repr = ReprType::IncrementRegime(   _repr ); break;
            case {TMPL_SET_MANTISSA_DECREMENT}: _repr = ReprType::DecrementMantissa( _repr ); break;
            case {TMPL_SET_MANTISSA_INCREMENT}: _repr = ReprType::IncrementMantissa( _repr ); break;
            case {TMPL_SET_EXPONENT_DECREMENT}: _repr = ReprType::DecrementExponent( _repr ); break;
            case {TMPL_SET_EXPONENT_INCREMENT}: _repr = ReprType::IncrementExponent( _repr ); break;
            case {TMPL_SET_REPRSTR}: _repr = ReprType::FromReprString(valstr); break;
        }

        recompute();
    }

    void recompute()
    {
        _value = ReprType::GetValue(_repr);


        _math.clear();
        if (_repr == ReprType::NaR() || _repr == ReprType::Zero())
        {
            _math = "";
        }
        else
        {
            for (int rep = 0; rep <= 1; ++rep)
            {

                _math += R"(<math xmlns="http://www.w3.org/1998/Math/MathML" display="block">)";
                _math +=    "<mrow>";
                _math +=      "<mi>value</mi>";
                _math +=      "<mo>=</mo>";
                _math +=      "<mo>(</mo>";
                _math +=       "<mo>(</mo>";
                _math +=        "<mn>1</mn>";
                _math +=        "<mo>-</mo>";
                _math +=        "<mn>3</mn>";
                _math +=        "<mo>×</mo>";

                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_SIGN}\">s</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_SIGN}\">" + std::to_string(ReprType::GetSignBit(_repr)) + "</mn>";

                _math +=       "<mo>)</mo>";
                _math +=       "<mo>+</mo>";
                _math +=       "<mfrac>";

                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_MANTISSA}\">mantissa</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_MANTISSA}\">" + std::to_string(ReprType::GetMantissa(_repr)) + "</mn>";

                _math +=        "<msup>";
                _math +=         "<mn>2</mn>";

                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_MBITS}\">mbits</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_MBITS}\">" + std::to_string(ReprType::NumMantissaBits(_repr)) + "</mn>";

                _math +=        "</msup>";
                _math +=       "</mfrac>";
                _math +=      "<mo>)</mo>";
                _math +=      "<mo>×</mo>";
                _math +=      "<msup>";
                _math +=       "<mn>2</mn>";
                _math +=       "<mrow>";
                _math +=        "<mo>(</mo>";
                _math +=         "<mn>1</mn>";
                _math +=         "<mo>-</mo>";
                _math +=         "<mn>2</mn>";
                _math +=         "<mo>×</mo>";

                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_SIGN}\">s</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_SIGN}\">" + std::to_string(ReprType::GetSignBit(_repr)) + "</mn>";

                _math +=        "<mo>)</mo>";
                _math +=        "<mo>×</mo>";
                _math +=        "<mo>(</mo>";
                _math +=         "<mn>4</mn>";
                _math +=         "<mo>×</mo>";

                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_REGIME}\">r</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_REGIME}\">" + std::to_string(ReprType::GetRegime(_repr)) + "</mn>";

                _math +=         "<mo>+</mo>";

                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_EXPONENT}\">e</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_EXPONENT}\">" + std::to_string(ReprType::GetExponent(_repr)) + "</mn>";

                _math +=         "<mo>+</mo>";

                if (rep == 0) _math += "<mi class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_SIGN}\">s</mi>";
                else          _math += "<mn class=\"identifier\" data-ic=\"{TMPL_IDENTIFIER_SIGN}\">" + std::to_string(ReprType::GetSignBit(_repr)) + "</mn>";

                _math +=        "<mo>)</mo>";
                _math +=       "</mrow>";
                _math +=      "</msup>";
                _math +=     "</mrow>";
                _math += R"(</math>)";
            }


            int signImplicitTerm = 1 - 3 * (int)ReprType::GetSignBit(_repr);
            int finalEqPow = (1 - 2 * (int)ReprType::GetSignBit(_repr)) * (4 * ReprType::GetRegime(_repr) + ReprType::GetExponent(_repr) + (int)ReprType::GetSignBit(_repr));

            _math += "<div class=\"large-content\">";
            _math +=  "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\">";
            _math +=   "<mrow>";
            _math +=     "<mo>=</mo>";
            _math +=     "<mo>(</mo>";
            _math +=      "<mn>" + std::to_string(signImplicitTerm) + "</mn>";
            _math +=      "<mo>+</mo>";
            _math +=       "<mfrac>";
            _math +=        "<mn>" + std::to_string(ReprType::GetMantissa(_repr)) + "</mn>";
            _math +=        "<msup>";
            _math +=         "<mn>2</mn>";
            _math +=          "<mn>" + std::to_string(ReprType::NumMantissaBits(_repr)) + "</mn>";
            _math +=        "</msup>";
            _math +=       "</mfrac>";
            _math +=     "<mo>)</mo>";
            _math +=     "<mo>×</mo>";
            _math +=     "<msup>";
            _math +=      "<mn>2</mn>";
            _math +=      "<mn>" + std::to_string(finalEqPow) + "</mn>";
            _math +=     "</msup>";
            _math +=   "</mrow>";
            _math +=  "</math>";
            _math += "</div>";


            int64_t topn = ReprType::GetMantissa(_repr) + signImplicitTerm * (1ll << ReprType::NumMantissaBits(_repr));
            bool isNeg = topn < 0;
            if (isNeg)
            {
                topn = -topn;
            }
            SimpleNumber top(topn);
            int finalPow = finalEqPow - ReprType::NumMantissaBits(_repr);

            _math += "<div class=\"large-content\">";
            _math +=  "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\">";
            _math +=   "<mrow>";
            _math +=     "<mo>=</mo>";
            if (isNeg) _math += "<mo>-</mo>";
            _math +=     "<mn>" + top.render10() + "</mn>";
            _math +=     "<mo>×</mo>";
            _math +=      "<msup>";
            _math +=       "<mn>2</mn>";
            _math +=       "<mn>" + std::to_string(finalPow) + "</mn>";
            _math +=      "</msup>";
            _math +=   "</mrow>";
            _math +=  "</math>";
            _math += "</div>";

            if (finalPow < 0)
            {
                _math += "<div class=\"large-content\">";
                _math +=  "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\">";
                _math +=   "<mrow>";
                _math +=     "<mo>=</mo>";
                if (isNeg) _math += "<mo>-</mo>";
                _math +=     "<mfrac>";
                _math +=      "<mn>" + top.render10() + "</mn>";
                _math +=      "<mn>" + SimpleNumber::pow2(-finalPow).render10() + "</mn>";
                _math +=     "</mfrac>";
                _math +=   "</mrow>";
                _math +=  "</math>";
                _math += "</div>";
            }
            else
            {
                _math += "<div class=\"large-content\">";
                _math +=  "<math xmlns=\"http://www.w3.org/1998/Math/MathML\" display=\"block\">";
                _math +=   "<mrow>";
                _math +=     "<mo>=</mo>";
                if (isNeg) _math += "<mo>-</mo>";
                _math +=     "<mn>" + (top * SimpleNumber::pow2(finalPow)).render10() + "</mn>";
                _math +=   "</mrow>";
                _math +=  "</math>";
                _math += "</div>";
            }
        }
    }

    uint64_t _repr;
    SimpleNumber _value;
    std::string _math;
};

extern "C" {

const char* e_get_string(Editor *e, int code)
{
    return e->GetString(code);
}

int e_get_int(Editor *e, int code)
{
    return e->GetInt(code);
}

void e_set_value(Editor *e, int code, const char *valstr)
{
    e->SetValue(code, valstr);
}

Editor* get_fe(int code)
{
    static IEEE754FloatEditor<IEEE754Float16Traits> gBinary16;
    static IEEE754FloatEditor<IEEE754BFloat16Traits> gBFloat16;
    static IEEE754FloatEditor<IEEE754MinifloatTraits> gMinifloat;
    static IEEE754FloatEditor<IEEE754Float32Traits> gBinary32;
    static IEEE754FloatEditor<IEEE754Float64Traits> gBinary64;

    static PositEditor<Posit8Traits> gPosit8;
    static PositEditor<Posit16Traits> gPosit16;
    static PositEditor<Posit32Traits> gPosit32;
    static PositEditor<Posit64Traits> gPosit64;

    switch (code)
    {
    case {TMPL_TYPE_BINARY16}: return &gBinary16;
    case {TMPL_TYPE_BFLOAT16}: return &gBFloat16;
    case {TMPL_TYPE_MINIFLOAT}: return &gMinifloat;
    case {TMPL_TYPE_BINARY32}: return &gBinary32;
    case {TMPL_TYPE_BINARY64}: return &gBinary64;
    case {TMPL_TYPE_POSIT8}: return &gPosit8;
    case {TMPL_TYPE_POSIT16}: return &gPosit16;
    case {TMPL_TYPE_POSIT32}: return &gPosit32;
    case {TMPL_TYPE_POSIT64}: return &gPosit64;
    }

    return nullptr;
};

}
