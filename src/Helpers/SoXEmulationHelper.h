/**
 * @file
 * The <C>SoXEmulationHelper</C> specification and body provides the
 * information whether strict SoX emulation is required and also
 * defines a helper routine for converting a real value accordingly.
 *
 * @author Dr. Thomas Tensi
 * @date   2026-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "Real.h"

/*--------------------*/

using BaseTypes::Primitives::Real;

/*====================*/

namespace SoXPlugins::Helpers {

    /**
     * The package <C>SoXEmulationHelper</C> provides the information
     * whether strict SoX emulation is required and also defines a
     * helper routine for converting a real value accordingly.
     */
    struct SoXEmulationHelper {

        /**
         * Returns information whether strict SoX compatibility is
         * required (that means that e.g delay line indices are
         * rounded from real to natural).
         *
         * @return  information about strict SoX emulation
         */
        static Boolean soxIsStrictlyEmulated ()
        {
            #ifdef EMULATE_SOX_STRICTLY
                return true;
            #else
                return false;
            #endif
        }

        /*--------------------*/

        /**
         * Adapts <C>value</C> to some integer value when SoX is
         * emulated strictly.  The kind of adaptation (floor, ceiling,
         * round) is given by a single character
         * <C>adaptationKind</C>.
         *
         * @param[in] value           value to be adapted conditionally
         * @param[in] adaptationKind  kind of adaptation: 'c' is ceiling,
         *                            'f' is floor, 'r' is round
         * @return  adapted value when SoX is emulated strictly
         *          otherwise unchanged value
         */
        static Real adaptValueForSoXEmulation (IN Real value,
                                               IN Character adaptationKind)
        {
            Real result = value;

            if (soxIsStrictlyEmulated()) {
                if (adaptationKind == 'c') {
                    result = result.ceiling();
                } else if (adaptationKind == 'f') {
                    result = result.floor();
                } else {
                    result = result.round();
                }
            }

            return result;
        }

    };
}
