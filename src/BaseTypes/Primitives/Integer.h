/**
 * @file
 * The <C>Integer</C> specification and body provides a value object
 * type for ints.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include <cmath>

#include "GlobalMacros.h"
#include "GenericNumber.h"
#include "Natural.h"
#include "MyString.h"

/*====================*/

namespace SoXPlugins::BaseTypes::Primitives {

    /**
     * An <C>Integer</C> object wraps a value of the associated
     * primitive type in an object.  An object of type Integer
     * contains a single internal variable whose type is some kind of
     * integer (int, long, ...).
     *
     * An important benefit of this approach is that all free-floating
     * functions on integers are gathered into a single class.
     */
    struct Integer : GenericNumber<Integer, int> {

        /*--------------------*/
        /* constructors       */
        /*--------------------*/

        /**
         * Initializes integer to an unspecified value.
         */
        Integer ()
        {
        }

        /*--------------------*/

        /**
         * Initializes integer from primitive int <C>i</C>.
         *
         * @param[in] i  int value
         */
        Integer (IN int i)
        {
            _value = i;
        }

        /*--------------------*/

        /**
         * Initializes integer from natural <C>n</C>.
         *
         * @param[in] n  natural value
         */
        Integer (IN Natural n)
        {
            _value = (int) (size_t) n;
        }

        /*--------------------*/
        /* type conversions   */
        /*--------------------*/

        /**
         * Returns string representation of integer with precision and
         * padding information.
         *
         * @param precision     minimum number of valid digits
         * @param padCharacter  character to use for left padding
         * @return  string representation
         */
        String toString (IN Natural precision = Natural{0},
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
         * Returns the current value as a size_t value.
         *
         * @return current value as a size_t value
         */
        explicit operator size_t () const
        {
            return (size_t) _value;
        }

        /*--------------------*/
        /* operators          */
        /*--------------------*/

        /**
         * Returns the modulus of current value and <C>other</C>.
         *
         * @return  modulus of current and <C>other</C>.
         */
        Integer operator % (IN Integer other)
        {
            return Integer{_value % other._value};
        }

        /*--------------------*/

        /**
         * Postincrements current value.
         *
         * @return  previous value of integer
         */
        Integer operator ++ (int)
        {
            return Integer{_value++};
        }

        /*--------------------*/

        /**
         * Preincrements current value.
         *
         * @return  current value of integer
         */
        Integer operator ++ ()
        {
            return Integer{--_value};
        }

        /*--------------------*/

        /**
         * Postdecrements current value.
         *
         * @return  previous value of integer
         */
        Integer operator -- (int)
        {
            return Integer{_value--};
        }

        /*--------------------*/

        /**
         * Predecrements current value.
         *
         * @return  current value of integer
         */
        Integer operator -- ()
        {
            return Integer{--_value};
        }

        /*-----------------------------*/
        /* advanced functions (static) */
        /*-----------------------------*/

        /**
         * Returns the ceiling of <C>x</C> as integer value.
         *
         * @param[in] x  some real value
         * @return  the least integer value that is greater or equal to
         *          <C>x</C>
         */
        static Integer ceiling (IN double x)
        {
            return Integer{(int) std::ceil(x)};
        }

        /*--------------------*/

        /**
         * Returns the floor of <C>x</C> as integer value.
         *
         * @param[in] x  some real value
         * @return  the largest integer value that is smaller or equal to
         *          <C>x</C>
         */
        static Integer floor (IN double x)
        {
            return Integer{(int) std::floor(x)};
        }

        /*--------------------*/

        /**
          * Returns the integer modulus of <C>i</C> and <C>other</C>;
          * brings result into the range [0,<C>other</C>)
          *
          * @param[in] i      some integer value
          * @param[in] other  the dividend value
          * @return remainder of division in range [0,<C>other</C>)
          */
        static Integer mod (IN Integer i, IN Integer other)
        {
            int iv = i._value;
            int ov = other._value;
            return Integer{i >= Integer{0} ? iv % ov : (ov - iv) % ov};
        }

        /*--------------------*/

        /**
         * Returns <C>base</C> to the <C>exponent</C> power.
         *
         * @param[in] base      some integer value
         * @param[in] exponent  some integer value
         * @return  base^exponent
         */
        static Integer power (IN Integer base, IN Integer exponent)
        {
            return Integer{(int) pow(base._value, exponent._value)};
        }

        /*--------------------*/

        /**
         * Rounds <C>x</C> to the nearest integer value.
         *
         * @param[in] x  some real value
         * @return  the integer value with least distance to <C>x</C>
         */
        static Integer round (IN double x)
        {
            return (x >= 0 ? floor(x + 0.5) : ceiling(x - 0.5));
        }

        /*--------------------*/

        /**
         * Returns string representation of integer <C>i</C> with
         * default precision and padding information.
         *
         * @return  string representation
         */
        static String toString (IN Integer& i)
        {
            return i.toString();
        }

        /*--------------------*/
        /* advanced methods   */
        /*--------------------*/

        /**
          * Returns the integer modulus of current value and
          * <C>other</C>; brings result into the range
          * [0,<C>other</C>)
          *
          * @param[in] other  the dividend value
          * @return remainder of division in range [0,<C>other</C>)
          */
        Integer mod (IN Integer other)
        {
            return mod(*this, other);
        }

        /*--------------------*/

        /**
         * Returns current value to the <C>exponent</C> power.
         *
         * @param[in] exponent  some integer value
         * @return  current^exponent
         */
        Integer power (IN Integer exponent)
        {
            return power(*this, exponent);
        }

    };

};
