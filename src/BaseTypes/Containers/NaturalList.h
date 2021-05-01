/**
 * @file
 * The <C>NaturalList</C> specification defines sequences of
 * non-negative integer values with zero-based arbitrary indexed
 * access to positions in the sequence.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include <initializer_list>
#include "GenericSequence.h"
#include "Natural.h"

/*====================*/

using std::initializer_list;
using SoXPlugins::BaseTypes::Primitives::Natural;

/*====================*/

namespace SoXPlugins::BaseTypes::Containers {

    /**
     * A <C>NaturalList</C> object is a sequence of non-negative
     * integer values with arbitrary indexed access to positions in
     * the sequence.  Indexing starts at zero and is consecutive.
     * Lists also allow duplicate elements.
     */
    struct NaturalList : public GenericSequence<Natural, Natural::toString>
    {

        /**
         * Initializes sequence of naturals from array of values.
         *
         * @param count  count of element values
         * @return  sequence with values in order given
         */
        static NaturalList fromList (IN size_t count, ...);

        /*--------------------*/

        /**
         * Returns printable representation of list.
         *
         * @return string representation of list
         */
        String toString () const;

        /*--------------------*/

        /**
         * Returns the maximum value in list.
         *
         * @return  maximum value
         */
        Natural maximum () const;

    };

};
