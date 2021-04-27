/**
 * @file
 * The <C>StringUtil</C> body implements several string routines not
 * included in the standard C++ library (like, for example, prefix
 * calculation, whitespace stripping etc).
 *
 * @author Dr. Thomas Tensi
 */

/*====================*/

#include <cassert>
#include <cctype>
#include <cstdarg>
#include "StringUtil.h"

/*====================*/

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;

/*====================*/

static const size_t _notFound = std::string::npos;
static const char _minusCharacter = '-';
static const char _decimalPointCharacter = '.';

static const Real _undefinedReal = NAN;

/*--------------------*/
/*--------------------*/

char StringUtil::at (IN String& st, IN Natural position)
{
    return st[(size_t) position];
}

/*--------------------*/

bool StringUtil::endsWith (IN String& st, IN String& suffix)
{
    const Natural stLength = st.size();
    const Natural suffixLength = suffix.size();
    bool result = false;

    if (stLength >= suffixLength) {
        result = (substring(st, stLength - suffixLength) == suffix);
    }

    return result;
}

/*--------------------*/

String StringUtil::expand (IN String& st,
                           IN String& replacement01,
                           IN String& replacement02,
                           IN String& replacement03,
                           IN String& replacement04,
                           IN String& replacement05,
                           IN String& replacement06,
                           IN String& replacement07,
                           IN String& replacement08,
                           IN String& replacement09,
                           IN String& replacement10)
{
    String result = st;
    replace(result, "%1", replacement01);
    replace(result, "%2", replacement02);
    replace(result, "%3", replacement03);
    replace(result, "%4", replacement04);
    replace(result, "%5", replacement05);
    replace(result, "%6", replacement06);
    replace(result, "%7", replacement07);
    replace(result, "%8", replacement08);
    replace(result, "%9", replacement09);
    replace(result, "%A", replacement10);
    return result;
}

/*--------------------*/

bool StringUtil::isNatural (IN String& st)
{
    const Natural stringLength = st.length();
    bool result = (stringLength > 0);

    for (Natural i = 0;  i < stringLength;  i++) {
        if (!isdigit(at(st, i))) {
            result = false;
            break;
        }
    }

    return result;
}

/*--------------------*/

bool StringUtil::isInt (IN String& st)
{
    bool result = false;

    if (st > "") {
        Natural firstIndex = (at(st, 0) == _minusCharacter ? 1 : 0);
        result = isNatural(substring(st, firstIndex));
    }

    return result;
}

/*--------------------*/

bool StringUtil::isReal (IN String& st)
{
    bool result;
    const Natural stringLength = st.length();

    if (stringLength == 0) {
        result = false;
    } else if (stringLength == 1 && at(st, 0) == _minusCharacter) {
        result = false;
    } else {
        result = true;
        bool isBeforeDecimalPoint = true;
        const Natural firstIndex = (at(st, 0) == _minusCharacter ? 1 : 0);

        for (Natural i = firstIndex; i < stringLength;  i++) {
            const char ch = at(st, i);

            if (ch == _decimalPointCharacter && isBeforeDecimalPoint) {
                isBeforeDecimalPoint = false;
            } else if (!isdigit(ch)) {
                result = false;
                break;
            }
        }
    }

    return result;
}

/*--------------------*/

char StringUtil::lastChar(IN String& st)
{
    return st[st.length() - 1];
}

/*--------------------*/

String StringUtil::prefix (IN String& st, IN Natural count)
{
    return substring(st, 0, count);
}

/*--------------------*/

void StringUtil::replace (INOUT String& st,
                          IN String& pattern,
                          IN String& replacement)
{
    const Natural patternLength = pattern.length();
    const Natural replacementLength = replacement.length();
    Natural position = 0;

    while (true) {
        position = st.find(pattern, (size_t) position);

        if (position == _notFound) {
            break;
        }

        st.replace((size_t) position, (size_t) patternLength,
                   replacement);
        position += replacementLength;
    }
}

/*--------------------*/

bool StringUtil::splitAt (IN String& st, IN String& separator,
                          OUT String& prefix, OUT String& suffix)
{
    const Natural position = st.find(separator);
    const Natural separatorLength = separator.length();
    const bool isFound = (position != _notFound);

    if (!isFound) {
        prefix = st;
        suffix = "";
    } else {
        prefix = StringUtil::prefix(st, position);
        suffix = substring(st, position + separatorLength);
    }

    return isFound;
}

/*--------------------*/

bool StringUtil::startsWith (IN String& st, IN String& prefix)
{
    const Natural stLength = st.size();
    const Natural prefixLength = prefix.size();
    bool result = false;

    if (stLength >= prefixLength) {
        result = (StringUtil::prefix(st, prefixLength) == prefix);
    }

    return result;
}

/*--------------------*/

String StringUtil::strip (IN String& st)
{
    const String whiteSpace = " \t\r\n";
    const Natural stringLength = st.length();
    Natural i;

    for (i = stringLength;  i > 0; i--) {
        if (whiteSpace.find(at(st, i - 1)) == _notFound) {
            break;
        }
    }

    const Natural lastRelevantIndex = i - Natural{1};

    for (i = 0;  i < stringLength;  i++) {
        if (whiteSpace.find(at(st, i)) == _notFound) {
            break;
        }
    }

    const Natural firstRelevantIndex = i;
    const Natural length =
        lastRelevantIndex - firstRelevantIndex + Natural{1};

    return substring(st, firstRelevantIndex, length);
}

/*--------------------*/

String StringUtil::substring (IN String& st,
                              IN Natural firstIndex, IN Natural count)
{
    return st.substr((size_t) firstIndex, (size_t) count);
}

/*--------------------*/

Integer StringUtil::toInteger (IN String& st,
                               IN Integer defaultValue)
{
    return (isInt(st) ? Integer{std::stoi(st)} : defaultValue);
}

/*--------------------*/

Natural StringUtil::toNatural (IN String& st)
{
    return toNatural(st, 1000000);
}

/*--------------------*/

Natural StringUtil::toNatural (IN String& st, IN Natural defaultValue)
{
    return (isNatural(st) ? (Natural) std::stoi(st) : defaultValue);
}

/*--------------------*/

Real StringUtil::toReal (IN String& st)
{
    return toReal(st, _undefinedReal);
}

/*--------------------*/

Real StringUtil::toReal (IN String& st, IN Real defaultValue)
{
    return Real{isReal(st) ? std::stod(st) : (double) defaultValue};
}

/*--------------------*/

String StringUtil::toString (IN bool& b)
{
    return (b ? "true" : "false");
}

/*--------------------*/

String StringUtil::toString (IN Real& r)
{
    return std::to_string((double) r);
}

/*--------------------*/

String StringUtil::toString (IN Integer& i) {
    return toString(i, 0);
}

/*--------------------*/

String StringUtil::toString (IN Integer& i,
                             IN Natural precision,
                             IN String padCharacter) {
    return i.toString(precision, padCharacter);
}

/*--------------------*/

String StringUtil::toString (IN Natural& n) {
    return toString(n, 0);
}

/*--------------------*/

String StringUtil::toString (IN Natural& n,
                             IN Natural precision,
                             IN String padCharacter) {
    return n.toString(precision, padCharacter);
}

/*--------------------*/

String StringUtil::toPrintableString (IN String& st) {
    const String doubleQuote = "\"";
    String result = st;
    replace(result, doubleQuote, doubleQuote + doubleQuote);
    result = doubleQuote + result + doubleQuote;
    return result;
}
