/**
 * @file
 * The <C>Configuration</C> specification defines configuration
 * values (like e.g. the maximum element count used when logging
 * lists).
 *
 * @author Dr. Thomas Tensi
 * @date   2026-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "Natural.h"

/*--------------------*/

using BaseTypes::Primitives::Natural;

/*====================*/

namespace BaseModules {


    struct Configuration {

        /*--------------------*/
        /* property access    */
        /*--------------------*/

        /**
         * Returns count of list elements presented in string
         * representations.
         *
         * @return count of list elements presented in string
         *         representations
         */
        static Natural presentedListElementCount ();

        /*--------------------*/
        /* property change    */
        /*--------------------*/

        /**
         * Sets count of list elements presented in string
         * representations to <C>count</C>
         *
         * @param[in]  count  count of list elements presented in
         *             string representations
         */
        static void setPresentedListElementCount (IN Natural count);

    };

}
