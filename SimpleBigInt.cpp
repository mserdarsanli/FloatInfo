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

#include <stdint.h>
#include <vector>
#include <iostream>
#include <charconv>

// Stores base-10 digits in uint32, ignoring overflow mid operation
// Addition and multiplication only
// No negative numbers
// No divisions, except 1/2 is hardcoded as "0.5" so pwers of two can be computed
// Should be enough for stringifying floats

template <uint32_t Base>
struct SimpleNumberBase
{
    using Self = SimpleNumberBase<Base>;

    struct DigitStore
    {
        std::vector<uint32_t> _digits;
        int _minExpo = 0;

        void allocate(int minExpo, int numDigits)
        {
            _digits.assign(numDigits, 0);
            _minExpo = minExpo;
        }

        const uint32_t* getDigitPtr(int expo) const
        {
            int idx = expo - _minExpo;
            if (idx < 0 || idx >= _digits.size()) {
                return nullptr;
            }
            return &_digits[idx];
        }

        uint32_t getDigit(int expo) const
        {
            const uint32_t *p = getDigitPtr(expo);
            return p ? *p : 0;
        }

        void addToDigit(int expo, uint32_t val)
        {
            const uint32_t *p = getDigitPtr(expo);
            if (!p)
            {
                std::cerr << "error\n";
                return;
            }
            *const_cast<uint32_t*>(p) += val;
        }

        void removeLeadingZeroes()
        {
            while (_digits.size() && _digits[_digits.size() - 1] == 0)
            {
                _digits.pop_back();
            }
        }

        void normalize()
        {
            uint32_t carry = 0;
            for (int i = 0; i < _digits.size(); ++i)
            {
                uint32_t val = carry + _digits[i];
                carry = val / Base;
                _digits[i] = val % Base;
            }
        }
    };


    DigitStore _store;

    SimpleNumberBase()
    {
        static_assert(Base == 2 || Base == 10);
    }

    SimpleNumberBase(std::string_view num)
    {
        Parse(num);
    }

    void Parse(std::string_view num)
    {
        bool error = false;
        int dotPos = -1;
        for (int i = 0; i < num.size(); ++i)
        {
            if (num[i] >= '0' && num[i] < '0' + Base) continue;

            if (num[i] == '.')
            {
                if (dotPos != -1)
                {
                    error = true;
                    break;
                }
                dotPos = i;
                continue;
            }

            error = true;
            break;
        }
        if (error)
        {
            std::cerr << "error parsing number [" << num << "]\n";
            _store.allocate(0, 0);
            return;
        }

        if (dotPos == -1)
        {
            _store.allocate(0, num.size());
            dotPos = num.size();
        }
        else
        {
            _store.allocate( dotPos - (int)num.size() + 1, num.size() - 1);
        }

        for (int i = 0; i < num.size(); ++i)
        {
            if (i == dotPos) continue;
            uint32_t val = num[i]-'0';
            int exp;

            exp = dotPos - i - (i<dotPos);
            _store.addToDigit(exp, val);
        }
    }

    Self operator+(const Self &ot) const
    {
        Self res;

        int minExpo = std::min(_store._minExpo, ot._store._minExpo);
        int maxExpo = std::max(_store._minExpo + (int)_store._digits.size() - 1, ot._store._minExpo + (int)ot._store._digits.size() - 1) + 1;
        int numDigits = (maxExpo - minExpo) + 1;
        res._store.allocate(minExpo, numDigits);

        uint32_t carry = 0;
        for (int i = minExpo; i <= maxExpo; ++i)
        {
            res._store.addToDigit(i, _store.getDigit(i) + ot._store.getDigit(i));
        }

        res._store.normalize();
        res._store.removeLeadingZeroes();
        return res;
    }

    Self operator*(const Self &ot) const
    {
        Self res;

        int minExpo = _store._minExpo + ot._store._minExpo;
        int numDigits = _store._digits.size() + ot._store._digits.size();
        res._store.allocate(minExpo, numDigits);

        for (int i = 0; i < _store._digits.size(); ++i)
        {
            int exp1 = _store._minExpo + i;
            uint32_t dig1 = _store.getDigit(exp1);

            for (int j = 0; j < ot._store._digits.size(); ++j)
            {
                int exp2 = ot._store._minExpo + j;
                uint32_t dig2 = ot._store.getDigit(exp2);

                res._store.addToDigit(exp1+exp2, dig1 * dig2);
            }
        }

        res._store.normalize();
        res._store.removeLeadingZeroes();
        return res;
    }

    static Self pow2(int p)
    {
        const char *half = (Base == 10 ? "0.5" : "0.1");
        const char *two = (Base == 10 ? "2" : "10");

        Self res("1");
        Self cur(p < 0 ? half : two);
        p = (p < 0 ? -p : p);

        for (int bit = 0; (1 << bit) <= p; ++bit)
        {
            if ((1 << bit) & p)
            {
                res = res * cur;
            }

            cur = cur * cur;
        }
        return res;
    }

    std::string render(int numFractionDigits = -1) const
    {
        std::string res;
        int minExpo = std::min(_store._minExpo, 0);
        int maxExpo = std::max(0, minExpo + (int)_store._digits.size() - 1);

        if (numFractionDigits != -1)
        {
            minExpo = -numFractionDigits;
        }

        for (int i = maxExpo; i >= minExpo; --i)
        {
            res.push_back(_store.getDigit(i) + '0');
            if (i == 0 && minExpo < 0)
            {
                res.push_back('.');
            }
        }
        return res;
    }

    void Dump()
    {
        std::cerr << "Num = " << render() << "\n";
        for (int i = 0; i < _store._digits.size(); ++i)
        {
            std::cerr << _store._digits[i] << " * " << Base << "**(" << i + _store._minExpo << ")\n";
        }
    }
};

// Rather than trying to convert base2 and base10 bigints, just calculate both of them with this class.
struct SimpleNumber
{
    SimpleNumber()
    {
    }

    static const SimpleNumber& One();
    static const SimpleNumber& Two();

    SimpleNumber(uint64_t num)
    {
        char buf[100];
        {
            auto res = std::to_chars(buf, buf+99, num, 10);
            _base10.Parse(std::string_view(buf, res.ptr-buf));
        }
        {
            auto res = std::to_chars(buf, buf+99, num, 2);
            _base2.Parse(std::string_view(buf, res.ptr-buf));
        }
    }

    static SimpleNumber pow2(int p)
    {
        SimpleNumber res;
        res._base2 = SimpleNumberBase<2>::pow2(p);
        res._base10 = SimpleNumberBase<10>::pow2(p);
        return res;
    }

    SimpleNumber operator+(const SimpleNumber &ot) const
    {
        SimpleNumber res;
        res._base2 = _base2 + ot._base2;
        res._base10 = _base10 + ot._base10;
        return res;
    }

    SimpleNumber operator*(const SimpleNumber &ot) const
    {
        SimpleNumber res;
        res._base2 = _base2 * ot._base2;
        res._base10 = _base10 * ot._base10;
        return res;
    }

    std::string render10(int numFractionDigits = -1) const
    {
        return (_isNegative ? "-" : "") + _base10.render(numFractionDigits);
    }

    std::string render2(int numFractionDigits = -1) const
    {
        return (_isNegative ? "-" : "") + _base2.render(numFractionDigits);
    }

    void Dump()
    {
        std::cerr << "Dual number=============\n";
        _base2.Dump();
        _base10.Dump();
    }

    bool _isNegative = false; // Only use for results, has no effect on behavior
    SimpleNumberBase<10> _base10;
    SimpleNumberBase<2> _base2;
};

SimpleNumber gOne(1ull);
SimpleNumber gTwo(2ull);
const SimpleNumber& SimpleNumber::One()
{
    return gOne;
}
const SimpleNumber& SimpleNumber::Two()
{
    return gTwo;
}


#ifdef RUN_TEST

#include <iostream>

int main()
{
    std::cerr << "Running tests...\n";

    SimpleNumberBase<10> addres("18147058729308407518989319541809371258139651463598036716927901216626501245785858394401987848885213785337769020486144791828237410508640772841862755928882267772678835630856434120266080093302878837874566334544365776702956526247791353499740754826713163964695697345234744147245128874318979464106491464934501");
    SimpleNumberBase<10> mulres("81369458867056388866901934520381444450629826768232073221651092640098489107163381248917379304458926780689047756979682514316609661113609420046632334393434365895797382063664703923874375591424753771003800508211716093209781555311822227369248588154584233940023334651862948264456804918823671481668553226534584785242895523377614023132583278539557933533222895049049889880899928348422176104260072311741631252677178358038373341385588660277857400367476114660281516305123546078476952596992372200762104218876036562260905052805734044631100874939607090469329033491948282258704139439851194056182428550517190728371565494");
    SimpleNumberBase<10> a("8094000771816950400149620195067932501401605744596371765723909060332939034523122198329375846139854323214793571617729921814045958609043811659877675975987108146371827389586515803354695786046642407688168818728896851610471357719917773827422626439471345328308554801542413154576618484677856740751938367615603");
    SimpleNumberBase<10> b("10053057957491457118839699346741438756738045719001664951203992156293562211262736196072612002745359462122975448868414870014191451899596961181985079952895159626307008241269918316911384307256236430186397515815468925092485168527873579672318128387241818636387142543692330992668510389641122723354553097318898");

    SimpleNumberBase<10> two("2");
    SimpleNumberBase<10> p("2854495385411919762116571938898990272765493248");

    std::cerr << "test add = " << (addres.render() == (a + b).render()) << "\n";
    std::cerr << "test mul = " << (mulres.render() == (a * b).render()) << "\n";
    std::cerr << "test pow = " << (p.render() == (SimpleNumberBase<10>::pow2(151)).render()) << "\n";
    std::cerr << "test pow = " << ("0.125" == (SimpleNumberBase<10>::pow2(-3)).render()) << "\n";


    SimpleNumberBase<10> s;
    s.Parse("1441431"); s.Dump();

    s.Parse("144.1431"); s.Dump();

    (SimpleNumberBase<10>("111229.733") + SimpleNumberBase<10>("229.833444")).Dump();
    (SimpleNumberBase<10>("9") + SimpleNumberBase<10>("9")).Dump();

    (SimpleNumberBase<10>("3.1415") * SimpleNumberBase<10>("11000001.0000002")).Dump();
    (SimpleNumberBase<10>("111111111") * SimpleNumberBase<10>("111111111")).Dump();
    (SimpleNumberBase<10>("1111111111") * SimpleNumberBase<10>("1111111111")).Dump();

    {
        SimpleNumberBase<10> a("0.00000035762786865234375");
        a._store.removeLeadingZeroes();
        SimpleNumberBase<10> b("1");
        SimpleNumberBase<10> c = a+b;
        a.Dump();
        b.Dump();
        c.Dump();
    }

    {
        SimpleNumberBase<10> a("101.001");
        SimpleNumberBase<10> b("1001.001");
        a.Dump();
        b.Dump();
        (a+b).Dump();

        SimpleNumberBase<10>::pow2(-2).Dump();
        SimpleNumberBase<10>::pow2(-1).Dump();
        SimpleNumberBase<10>::pow2(0).Dump();
        SimpleNumberBase<10>::pow2(1).Dump();
        SimpleNumberBase<10>::pow2(2).Dump();
    }

    {
        SimpleNumber(35ull).Dump();
        SimpleNumber((uint64_t)-1).Dump();
    }

    return 0;
}

#endif
