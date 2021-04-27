/**
 * @file
 * The <C>StringList</C> body implements sequences of string values
 * with zero-based arbitrary indexed access to positions in the
 * sequence.
 */

/*====================*/

#include "StringList.h"

#include "Assertion.h"
#include "StringUtil.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::BaseTypes::Containers::StringList;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
using StringUtil::listToStringExplicit;
using StringUtil::prefix;
using StringUtil::substring;

/*====================*/

String StringList::toString () const
{
    return _toString("StringList");
}

/*--------------------*/

String StringList::join (IN String& separator) const
{
    String result;
    Natural length = size();

    for (Natural i = 0;  i < length;  i++) {
        result += (i > 0 ? separator : "") + at(i);
    }

    return result;
}

/*--------------------*/

StringList StringList::makeBySplit (IN String& st,
                                    IN String& separator)
{
    const Natural undefined = std::string::npos;
    const Natural separatorLength = separator.length();
    Assertion_pre(separatorLength > 0, "separator must be non-empty");

    StringList result;
    String remainder = st;
    Natural position;

    do {
        position = remainder.find(separator);

        if (position == undefined) {
            result.append(remainder);
        } else {
            result.append(prefix(remainder, position));
            remainder = substring(remainder, position + separatorLength);
        }
    } while (position != undefined);

    return result;
}
