/**
 * @file
 * The <C>Natural</C> specification and body provides a value object
 * type for naturals (positive integers with zero).
 */

#pragma once

/*====================*/

#include <cmath>

#include "GlobalMacros.h"
#include "GenericNumber.h"
#include "MyString.h"

/*====================*/

namespace SoXPlugins::BaseTypes::Primitives {

    /**
     * An <C>Natural</C> object wraps a value of the associated
     * primitive type in an object.  An object of type Natural
     * contains a single internal variable whose type is some kind of
     * natural (size_t, ...).
     *
     * An important benefit of this approach is that all free-floating
     * functions on naturals are gathered into a single class.
     */
    struct Natural : GenericNumber<Natural, size_t> {

        /*--------------------*/
        /* constructors       */
        /*--------------------*/

        /**
         * Initializes natural to an unspecified value.
         */
        Natural ()
        {
        }

        /*--------------------*/

        /**
         * Initializes natural from unsigned <C>n</C>.
         *
         * @param[in] n  unsigned value
         */
        Natural (IN size_t n)
        {
            _value = n;
        }

        /*--------------------*/
        /* type conversions   */
        /*--------------------*/

        /**
         * Returns string representation of natural with precision and
         * padding information.
         *
         * @param precision     minimum number of valid digits
         * @param padCharacter  character to use for left padding
         * @return  string representation
         */
        String toString (IN Natural precision = 0,
                         IN String padCharacter = "0") const
        {
            String result = std::to_string(_value);
            const Natural length{result.size()};
            Natural padCount =
                (precision > length ? precision - length : 0);

            while (padCount > 0) {
                result = padCharacter + result;
                padCount--;
            }

            return result;
        }

        /*--------------------*/

        /**
         * Returns the current value as a floating point value.
         *
         * @return current value as a floating point value
         */
        explicit operator double () const
        {
            return (double) _value;
        }

        /*--------------------*/

        /**
         * Returns the current value as an integer value.
         *
         * @return current value as an integer value
         */
        explicit operator int () const
        {
            return (int) _value;
        }

        /*--------------------*/
        /* operators          */
        /*--------------------*/

        /**
         * Returns the modulus of current value and <C>other</C>.
         *
         * @return  modulus of current and <C>other</C>.
         */
        Natural operator % (IN Natural other)
        {
            return Natural{_value % other._value};
        }

        /*--------------------*/

        /**
         * Postincrements current value.
         *
         * @return  previous value of natural
         */
        Natural operator ++ (int)
        {
            return Natural{_value++};
        }

        /*--------------------*/

        /**
         * Preincrements current value.
         *
         * @return  current value of natural
         */
        Natural operator ++ ()
        {
            return Natural{--_value};
        }

        /*--------------------*/

        /**
         * Postdecrements current value.
         *
         * @return  previous value of natural
         */
        Natural operator -- (int)
        {
            return Natural{_value--};
        }

        /*--------------------*/

        /**
         * Predecrements current value.
         *
         * @return  current value of natural
         */
        Natural operator -- ()
        {
            return Natural{--_value};
        }

        /*-----------------------------*/
        /* advanced functions (static) */
        /*-----------------------------*/

        /**
         * Returns the ceiling of real <C>r</C> as natural value.
         *
         * @param[in] r  some real value
         * @return  the least natural value that is greater or equal to
         *          <C>r</C>
         */
        static Natural ceiling (IN double r)
        {
            return Natural{(size_t) std::ceil(r)};
        }

        /*--------------------*/

        /**
         * Returns the floor of real <C>r</C> as natural value.
         *
         * @param[in] r  some real value
         * @return  the largest natural value that is smaller or equal to
         *          <C>r</C>
         */
        static Natural floor (IN double r)
        {
            return Natural{(size_t) std::floor(r)};
        }

        /*--------------------*/

        /**
          * Returns the natural modulus; brings result into the range
          * [0,i)
          *
          * @param[in] n  some natural value
          * @param[in] i  the divisor value
          * @return remainder of division in range [0,i)
          */
        static Natural mod (IN Natural n, IN Natural i)
        {
            size_t nv = n._value;
            size_t iv = i._value;
            return Natural{n >= 0 ? nv % iv : (iv - nv) % iv};
        }

        /*--------------------*/

        /**
         * Returns <C>base</C> to the <C>exponent</C> power.
         *
         * @param[in] base      some natural value
         * @param[in] exponent  some natural value
         * @return  base^exponent
         */
        static Natural power (IN Natural base, IN Natural exponent)
        {
            return Natural{(size_t) pow(base._value, exponent._value)};
        }

        /*--------------------*/

        /**
         * Rounds real <C>r</C> to the nearest natural value.
         *
         * @param[in] r  some real value
         * @return  the natural value with least distance to <C>r</C>
         */
        static Natural round (IN double r)
        {
            return (r >= 0 ? floor(r + 0.5) : ceiling(r - 0.5));
        }

        /*--------------------*/

        /**
         * Returns string representation of natural <C>n</C> with
         * default precision and padding information.
         *
         * @return  string representation
         */
        static String toString (IN Natural& n)
        {
            return n.toString();
        }

        /*--------------------*/
        /* advanced methods   */
        /*--------------------*/

        /**
          * Returns the natural modulus of current value and
          * <C>other</C>; brings result into the range
          * [0,<C>other</C>)
          *
          * @param[in] other  the divisor value
          * @return remainder of division in range [0,<C>other</C>)
          */
        Natural mod (IN Natural other)
        {
            return mod(*this, other);
        }

        /*--------------------*/

        /**
         * Returns current value to the <C>exponent</C> power.
         *
         * @param[in] exponent  some natural value
         * @return  current^exponent
         */
        Natural power (IN Natural exponent)
        {
            return power(*this, exponent);
        }

    };

};
