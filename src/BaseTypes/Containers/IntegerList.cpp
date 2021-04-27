/**
 * @file
 * The <C>IntegerList</C> body implements sequences of integer values
 * with zero-based arbitrary indexed access to positions in the
 * sequence.
 */

/*====================*/

#include <cstdarg>
#include "IntegerList.h"

/*====================*/

using SoXPlugins::BaseTypes::Containers::IntegerList;

/*====================*/

IntegerList IntegerList::fromList (IN size_t count, ...)
{
    IntegerList result{};
    std::va_list argumentList;
    va_start(argumentList, count);

    for (size_t i = 0;  i < count;  ++i) {
        result.append(va_arg(argumentList, Integer));
    }
            
    va_end(argumentList);
    return result;
}

/*--------------------*/

String IntegerList::toString () const
{
    return _toString("IntegerList");
}
