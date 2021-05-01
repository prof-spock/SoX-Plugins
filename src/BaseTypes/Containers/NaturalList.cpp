/**
 * @file
 * The <C>NaturalList</C> body implements sequences of non-negative
 * integer values with zero-based arbitrary indexed access to
 * positions in the sequence.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#include <cstdarg>
#include "NaturalList.h"

/*====================*/

using SoXPlugins::BaseTypes::Containers::NaturalList;

/*====================*/

NaturalList NaturalList::fromList (IN size_t count, ...)
{
    NaturalList result{};
    std::va_list argumentList;
    va_start(argumentList, count);

    for (size_t i = 0;  i < count;  ++i) {
        result.append(va_arg(argumentList, Natural));
    }
            
    va_end(argumentList);
    return result;
}

/*--------------------*/

String NaturalList::toString () const
{
    return _toString("NaturalList");
}

/*--------------------*/

Natural NaturalList::maximum () const
{
    Natural maximumValue = 0;

    for (Natural i = 0;  i < size();  i++) {
        const Natural value = at(i);
        maximumValue = (maximumValue < value ? value : maximumValue);
    }

    return maximumValue;
}
