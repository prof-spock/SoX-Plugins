/**
 * @file
 * The <C>MyArray</C> specification and body defines a
 * generic array type for stack allocated arrays represented
 * by a pointer to the first element.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-07
 */

#pragma once

/*====================*/

#include "Natural.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Natural;

/*====================*/

namespace SoXPlugins::BaseTypes::Containers {

    /**
     * Allocates an array of <C>count</C> elements of type <C>T</C> on
     * the stack.
     *
     * @param T      type of element
     * @param count  count of elements to be allocated
     * @return array with <C>count</C> entries of type <C>T</C>
     */
    #define makeLocalArray(T, count) \
        (T*) alloca(((int) (count)) * sizeof(T))

    /*--------------------*/

    /**
     * Copies <C>count</C> elements from array pointed to by
     * <C>sourcePtr</C> to array pointed to by <C>targetPtr</C>.
     * Increments both pointers by <C>count</C>.
     *
     * @param sourcePtr       pointer to first source data element
     * @param targetPtr       pointer to first target data element
     * @param count           count of elements to be copied
     */
    template<typename T>
    void copyArray (T*& targetPtr, T*& sourcePtr, Natural count)
    {
        for (int i = (int) count;  i > 0;  i--) {
            *targetPtr++ = *sourcePtr++;
        }
    }

}
