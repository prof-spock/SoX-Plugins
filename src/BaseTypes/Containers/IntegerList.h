/**
 * @file
 * The <C>IntegerList</C> specification defines sequences of integer
 * values with zero-based arbitrary indexed access to positions in the
 * sequence.
 */

#pragma once

/*====================*/

#include "GenericSequence.h"
#include "Integer.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Integer;

/*====================*/

namespace SoXPlugins::BaseTypes::Containers {

    /**
     * An <C>IntegerList</C> object is a sequence of integer values
     * with arbitrary indexed access to positions in the sequence.
     * Indexing starts at zero and is consecutive.  Lists also allow
     * duplicate elements.
     */
    struct IntegerList : public GenericSequence<Integer, Integer::toString>
    {

        /**
         * Initializes sequence of integers from array of values.
         *
         * @param count  count of element values
         * @return  sequence with values in order given
         */
        static IntegerList fromList (IN size_t count, ...);

        /*--------------------*/

        /**
         * Returns printable representation of list.
         *
         * @return string representation of list
         */
        String toString () const;

    };

};
