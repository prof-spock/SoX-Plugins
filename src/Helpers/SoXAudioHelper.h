/**
 * @file
 * The <C>SoXAudioHelper</C> specification defines several audio
 * helper routines (like, for example, converting decibels to linear
 * factor).
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include <cmath>
#include "Real.h"

/*--------------------*/

using BaseTypes::Primitives::Real;

/*====================*/

namespace SoXPlugins::Helpers {

    /**
     * The package <C>SoXAudioHelper</C> defines several audio helper
     * routines (like, for example, converting decibels to linear
     * factor).
     */
    struct SoXAudioHelper {

        /**
         * Returns linear factor for <C>dBValue</C> with given
         * <C>quotient</C>.
         *
         * @param[in] dBValue   the decibel value to be converted
         * @param[in] quotient  the factor to be applied to
         *                      log value
         * @return  linear factor representing decibel value
         */
        inline static Real dBToLinear (IN Real dBValue,
                                       IN Real quotient = 20.0)
        {
            return Real::power(10.0, dBValue / quotient);
        }

    };
}
