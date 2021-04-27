/**
 * @file
 * The <C>RealList</C> body implements sequences of real values
 * with zero-based arbitrary indexed access to positions in the
 * sequence.
 */

/*====================*/

#include <cstdarg>
#include "RealList.h"

/*====================*/

using SoXPlugins::BaseTypes::Containers::RealList;

/*====================*/

RealList RealList::fromList (IN size_t count, ...)
{
    RealList result{};
    std::va_list argumentList;
    va_start(argumentList, count);

    for (size_t i = 0;  i < count;  ++i) {
        result.append(va_arg(argumentList, Real));
    }
            
    va_end(argumentList);
    return result;
}

/*--------------------*/

String RealList::toString () const
{
    return _toString("RealList");
}

/*--------------------*/

void RealList::fill (IN Real value)
{
    for (Real& entry : *this) {
        entry = value;
    }
}

/*--------------------*/

void RealList::multiply (IN Real value)
{
    for (Real& entry : *this) {
        entry *= value;
    }
}
