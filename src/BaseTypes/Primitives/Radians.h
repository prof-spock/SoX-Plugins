/**
 * @file
 * The <C>Radians</C> specification and body provides a simple
 * radians type.
 */

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Real.h"

/*====================*/

namespace SoXPlugins::BaseTypes::Primitives {

    /**
     * A <C>Radians</C> object is a real-valued and represents the
     * range from 0 to 2pi.
     */
    struct Radians : public Real {

        /*--------------------*/
        /* constructors       */
        /*--------------------*/

        /**
         * Initializes radians object to an unspecified value.
         */
        Radians ()
        {
        }

        /*--------------------*/

        /**
         * Initializes radians from double <C>d</C>.
         *
         * @param[in] d  floating point value
         */
        Radians (IN double d)
        {
            _value = d;
        }

        /*--------------------*/

        /**
         * Initializes radians from real <C>r</C>.
         *
         * @param[in] r  real value
         */
        Radians (IN Real r)
        {
            _value = (double) r;
        }

        /*--------------------*/
        /* type conversions   */
        /*--------------------*/

        /* /\** */
        /*  * Returns string representation of radians with precision */
        /*  * and padding information. */
        /*  * */
        /*  * @param precision     minimum number of valid digits */
        /*  * @param padCharacter  character to use for left padding */
        /*  * @return  string representation */
        /*  *\/ */
        /* String toString (IN Natural precision = 0, */
        /*                  IN Natural fractionalDigitCount = 0, */
        /*                  IN String padCharacter = "0") const */
        /* { */
        /*     String result = std::to_string(_value); */
        /*     Natural padCount = precision - result.size(); */

        /*     while (padCount-- > 0) { */
        /*         result = padCharacter + result; */
        /*     } */

        /*     return result; */
        /* } */

    };

};
