/**
 * @file
 * The <C>RealList</C> specification defines sequences of real values
 * with zero-based arbitrary indexed access to positions in the
 * sequence.
 */

#pragma once

/*====================*/

#include "GenericSequence.h"
#include "Real.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Real;

/*====================*/

namespace SoXPlugins::BaseTypes::Containers {

    /**
     * A <C>RealList</C> object is a sequence of real values with
     * arbitrary indexed access to positions in the sequence.
     * Indexing starts at zero and is consecutive.  Lists also allow
     * duplicate elements.
     */
    struct RealList : public GenericSequence<Real, Real::toString>
    {

        /**
         * Initializes sequence of reals from array of values.
         *
         * @param count  count of element values
         * @return  sequence with values in order given
         */
        static RealList fromList (IN size_t count, ...);

        /*--------------------*/

        /**
         * Returns printable representation of list.
         *
         * @return string representation of list
         */
        String toString () const;

        /*--------------------*/

        /**
         * Fills list with <C>value</C>.
         *
         * @param value  value to be used for filling list
         */
        void fill (IN Real value);

        /*--------------------*/

        /**
         * Multiply list elements by <C>value</C>.
         *
         * @param value  value to be used for multiplying each element
         *               with
         */
        void multiply (IN Real value);

    };

};
