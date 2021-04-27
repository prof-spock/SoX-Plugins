/**
 * @file
 * The <C>Dictionary</C> body implements maps from string keys to
 * string values.
 */

/*====================*/

#include "Dictionary.h"

#include "Assertion.h"
#include "StringUtil.h"

/*====================*/

using std::pair;
using SoXPlugins::BaseTypes::Containers::Dictionary;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
using StringUtil::stringMapToStringExplicit;
using StringUtil::toPrintableString;

/*====================*/

Dictionary::Dictionary ()
    : map()
{
}

/*--------------------*/

Dictionary Dictionary::makeFromList (IN StringList& list)
{
    Dictionary result;

    Assertion_pre(list.size() % 2 == 0, "list must have even length");
    bool isKey = true;
    String key;

    for (const String& st : list) {
        if (isKey) {
            key = st;
        } else {
            result[key] = st;
        }

        isKey = !isKey;
    }

    return result;
}


/*--------------------*/

Dictionary::~Dictionary ()
{
}

/*--------------------*/

String Dictionary::toString () const
{
    return stringMapToStringExplicit<Dictionary,
                                     String,
                                     toPrintableString>("Dictionary", *this);
}

/*--------------------*/

String Dictionary::atWithDefault (IN String& key,
                                  IN String& defaultValue) const
{
    return (contains(key) ? at(key) : defaultValue);
}

/*--------------------*/

bool Dictionary::contains (IN String& key) const
{
    return (find(key) != end());
}
