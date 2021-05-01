/**
 * @file
 * The <C>StringSet</C> body implements sets of string values with no
 * duplicates and only simple containment access and the ability to
 * add and remove elements.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#include "StringSet.h"

#include <cassert>
#include "StringUtil.h"

/*====================*/

using SoXPlugins::BaseTypes::Containers::StringSet;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
using StringUtil::listToStringExplicit;
using StringUtil::toPrintableString;

/*====================*/

StringSet::StringSet ()
    : set<String>()
{
}

/*--------------------*/

StringSet StringSet::makeFromList (IN StringList& list)
{
    StringSet result = StringSet();

    for (const String& st : list) {
        result.insert(st);
    }

    return result;
}


/*--------------------*/

StringSet::~StringSet ()
{
}

/*--------------------*/

String StringSet::toString () const
{
    return listToStringExplicit<StringSet,
                                String,
                                toPrintableString>("StringSet", *this);
}

/*--------------------*/

bool StringSet::contains (IN String& key) const
{
    return (find(key) != end());
}

/*--------------------*/

void StringSet::add (IN String& element)
{
    insert(element);
}

/*--------------------*/

void StringSet::remove (IN String& element)
{
    erase(element);
}

