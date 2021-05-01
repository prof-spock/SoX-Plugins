/**
 * @file
 * The <C>Percentage</C> specification and body provides a simple
 * percentage type.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Real.h"

/*====================*/

namespace SoXPlugins::BaseTypes::Primitives {

    /**
     * A <C>Percentage</C> object is real-valued and represents a
     * percentage (normalized to the 0 to 100 range).
     */
    struct Percentage : Real {

        /*--------------------*/
        /* constructors       */
        /*--------------------*/

        /**
         * Initializes percentage to an unspecified value.
         */
        Percentage ()
        {
        }

        /*--------------------*/

        /**
         * Initializes percentage from double <C>d</C>.
         *
         * @param[in] d  floating point value
         */
        Percentage (IN double d)
        {
            _value = d;
        }

        /*--------------------*/
        /* type conversions   */
        /*--------------------*/

        /**
         * Returns string representation of percentage with precision
         * and padding information.
         *
         * @param precision              minimum number of valid digits
         * @param fractionalDigitCount   number of decimal digits
         * @param padCharacter           character to use for left
         *                               padding
         * @return  string representation
         */
        String toString (IN Natural precision = 0,
                         IN Natural fractionalDigitCount = 0,
                         IN String padCharacter = "0") const
        {
            String result = std::to_string(_value);
            Natural padCount = precision - result.size();

            while (padCount-- > 0) {
                result = padCharacter + result;
            }

            return result;
        }

        /*--------------------*/
        /* advanced functions */
        /*--------------------*/

        /**
         * Forces value <C>x</C> to a percentage between 0 and 100
         *
         * @param[in] x   some percentage value
         * @return  value forced to range [0, 100]
         */
        static Percentage forceToPercentage (IN Percentage x)
        {
            return Percentage{x < 0.0 ? 0.0 : (x > 100.0 ? 100.0 : x)};
        }

        /*--------------------*/

        /**
         * Returns <C>percentage</C> of real <C>value</C>.
         *
         * @param[in] percentage  the percentage to be found
         * @param[in] value       the real value to be scaled
         * @return  adjusted value by percentage
         */
        static Real percentOf (IN Percentage percentage,
                               IN Real value)
        {
            return Real{(double) percentage * (double) value / 100.0};
        }

        /*--------------------*/

        /**
         * Returns <C>percentage</C> of integer <C>value</C>.
         *
         * @param[in] percentage  the percentage to be found
         * @param[in] value       the integer value to be scaled
         * @return  adjusted value by percentage
         */
        static Integer percentOf (IN Percentage percentage,
                                  IN Integer value)
        {
            return Integer{(int) ((double) percentage
                                  * (int) value / 100.0)};
        }

        /*--------------------*/

        /**
         * Returns <C>percentage</C> of natural <C>value</C>.
         *
         * @param[in] percentage  the percentage to be found
         * @param[in] value       the natural value to be scaled
         * @return  adjusted value by percentage
         */
        static Natural percentOf (IN Percentage percentage,
                                  IN Natural value)
        {
            return Natural::round((double) percentage / 100.0
                                  * (double) value);
        }

        /*--------------------*/
        /* advanced methods   */
        /*--------------------*/

        /**
         * Forces current value to a value between 0 and 100
         *
         * @return  current value forced to range [0, 100]
         */
        Percentage forceToPercentage () const
        {
            return forceToPercentage(*this);
        }

        /*--------------------*/

        /**
         * Returns percentage of <C>value</C> from current percentage.
         *
         * @param[in] value       the value to be scaled
         * @return  adjusted value by percentage
         */
        template <typename T>
        T of (IN T value) const
        {
            return percentOf(*this, value);
        }

    };

};
