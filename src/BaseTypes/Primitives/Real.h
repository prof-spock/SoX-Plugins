/**
 * @file
 * The <C>Real</C> specification defines a value object type for real
 * values.
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
#include "Integer.h"
#include "MyString.h"

/*====================*/

namespace SoXPlugins::BaseTypes::Primitives {

    /**
     * An <C>Real</C> object wraps a value of the associated primitive
     * type double in an object.  An object of type Real contains a
     * single internal variable whose type is some kind of floating
     * point value.
     *
     * An important benefit of this approach is that all free-floating
     * functions on reals are gathered into a single class.
     */
    struct Real : GenericNumber<Real, double> {

        /*--------------------*/
        /* constructors       */
        /*--------------------*/

        /**
         * Initializes real to an unspecified value.
         */
        Real ()
        {
        }

        /*--------------------*/

        /**
         * Initializes real from double <C>d</C>.
         *
         * @param[in] d  floating point value
         */
        Real (IN double d)
        {
            _value = d;
        }

        /*--------------------*/

        /**
         * Initializes real from integer <C>i</C>.
         *
         * @param[in] i  integer value
         */
        Real (IN Integer i)
        {
            _value = (int) i;
        }

        /*--------------------*/
        /* type conversions   */
        /*--------------------*/

        /**
         * Returns string representation of real with precision and
         * padding information.
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
            const Natural length{result.size()};
            Natural padCount =
                (precision > length ? precision - length : 0);

            while (padCount > 0) {
                result = padCharacter + result;
                padCount--;
            }

            return result;
        }

        /*-----------------------------*/
        /* advanced functions (static) */
        /*-----------------------------*/

        /**
         * Returns the absolute value of real <C>r</C> as real value.
         *
         * @param[in] r  some real value
         * @return  the absolute value of <C>r</C>
         */
        static Real abs (IN Real r)
        {
            return Real{std::abs(r._value)};
        }

        /*--------------------*/

        /**
         * Returns the ceiling of real <C>r</C> as real value.
         *
         * @param[in] r  some real value
         * @return  the least integer value that is greater or equal to
         *          <C>r</C>
         */
        static Real ceiling (IN Real r)
        {
            return Real{std::ceil(r._value)};
        }

        /*--------------------*/

        /**
          * Returns the cosine of <C>x</C>.
          *
          * @param[in] x  real value
          * @return  the cosine of x
          */
        static Real cos (IN Real x)
        {
            return Real{std::cos(x._value)};
        }

        /*--------------------*/

        /**
          * Returns e^<C>x</C>.
          *
          * @param[in] x  real value
          * @return  e^x
          */
        static Real exp (IN Real x)
        {
            return Real{std::exp(x._value)};
        }

        /*--------------------*/

        /**
         * Returns the floor of real <C>r</C> as real value.
         *
         * @param[in] r  some real value
         * @return  the largest integer value that is smaller or equal to
         *          <C>r</C>
         */
        static Real floor (IN Real r)
        {
            return Real{std::floor(r._value)};
        }

        /*--------------------*/

        /**
         * Forces value <C>x</C> to range between <C>lowBound</C> and
         * <C>highBound</C>.
         *
         * @param[in] x          some percentage value
         * @param[in] lowBound   minimum acceptable percentage value
         * @param[in] highBound  maximum acceptable percentage value
         * @return value forced to range [<C>lowBound</C>,
         *                                <C>highBound</C>]
         */
        static Real forceToRange (IN Real x,
                                  IN Real lowBound,
                                  IN Real highBound)
        {
            return (x < lowBound ? lowBound
                    : (x > highBound ? highBound : x));
        }

        /*--------------------*/

        /**
         * Strips off integer part from <C>x</C> and only returns digits
         * after the decimal point; for negative <C>x</C> also a positive
         * result is produced
         *
         * @param[in] x  some real value
         * @return  the positive fractional part after the decimal point
         */
        static Real fractionalPart (IN Real x)
        {
            double result = std::abs(x._value);
            result -= std::floor(result);
            result = copysign(result, x._value);
            return Real{result};
        }

        /*--------------------*/

        /**
          * Returns the natural logarithm of <C>x</C>.
          *
          * @param[in] x  real value
          * @return  ln(x)
          */
        static Real log (IN Real x)
        {
            return Real{std::log(x._value)};
        }

        /*--------------------*/

        /**
          * Returns the real modulus; brings result into the range
          * [0,i)
          *
          * @param[in] x  some real value
          * @param[in] d  the real divisor value
          * @return remainder of division in range [0,i)
          */
        static Real mod (IN Real x, IN Real d)
        {
            const double xv = x._value;
            const double dv = d._value;
            return (x >= 0.0 ? fmod(xv, dv) : fmod(dv - xv, dv));
        }

        /*--------------------*/

        /**
         * Returns <C>base</C> to the <C>exponent</C> power.
         *
         * @param[in] base      some real value
         * @param[in] exponent  some real value
         * @return  base^exponent
         */
        static Real power (IN Real base, IN Real exponent)
        {
            return Real{pow(base._value, exponent._value)};
        }

        /*--------------------*/

        /**
         * Rounds <C>x</C> to <C>decimalPlaceCount</C> decimal places
         * and returns result.
         *
         * @param[in] x                  the value to be rounded
         * @param[in] decimalPlaceCount  the number of decimal places to keep
         *                               by rounding

         * @return  the value <C>x</C> rounded to <C>decimalPlaceCount</C>
         *          decimal places
         */
        static Real round (IN Real x,
                           IN Natural decimalPlaceCount = 0)
        {
            const double factor = std::pow(10.0, (double) decimalPlaceCount);
            return ((int) Integer::round(x._value * factor) / factor);
        }

        /*--------------------*/

        /**
          * Returns the sine of <C>x</C>.
          *
          * @param[in] x  real value
          * @return  the sine of x
          */
        static Real sin (IN Real x)
        {
            return Real{std::sin(x._value)};
        }

        /*--------------------*/

        /**
          * Returns the square of <C>x</C>.
          *
          * @param[in] x  real value to be squared
          * @return x^2
          */
        static Real sqr (IN Real x)
        {
            return x * x;
        }

        /*--------------------*/

        /**
          * Returns the square root of <C>x</C>.
          *
          * @param[in] x  real value to be rooted
          * @return x^{1/2}
          */
        static Real sqrt (IN Real x)
        {
            return Real{std::sqrt(x._value)};
        }

        /*--------------------*/

        /**
         * Returns string representation of real <C>r</C> with
         * precision and padding information.
         *
         * @return  string representation
         */
        static String toString (IN Real& r)
        {
            return r.toString();
        }

        /*--------------------*/
        /* advanced functions */
        /*--------------------*/

        /**
         * Returns the absolute value of current value as real value.
         *
         * @return  the absolute value of current value
         */
        Real abs () const
        {
            return abs(*this);
        }

        /*--------------------*/

        /**
         * Returns the ceiling of current value as real value.
         *
         * @return  the least integer value that is greater or equal to
         *          current value
         */
        Real ceiling () const
        {
            return ceiling(*this);
        }

        /*--------------------*/

        /**
          * Returns the cosine of current value.
          *
          * @return  the cosine of current value
          */
        Real cos () const
        {
            return cos(*this);
        }

        /*--------------------*/

        /**
          * Returns e^ current value
          *
          * @param[in] x  real value
          * @return  e^x
          */
        Real exp () const
        {
            return exp(*this);
        }

        /*--------------------*/

        /**
         * Returns the floor of current value as real value.
         *
         * @return  the largest integer value that is smaller or equal
         *          to current value
         */
        Real floor () const
        {
            return floor(*this);
        }

        /*--------------------*/

        /**
         * Forces current value to range between <C>lowBound</C> and
         * <C>highBound</C> and returns result.
         *
         * @param[in] lowBound   minimum acceptable percentage value
         * @param[in] highBound  maximum acceptable percentage value
         * @return value forced to range [<C>lowBound</C>,
         *                                <C>highBound</C>]
         */
        Real forceToRange (IN Real lowBound,
                           IN Real highBound) const
        {
            return forceToRange(*this, lowBound, highBound);
        }

        /*--------------------*/

        /**
         * Strips off integer part from current value and only returns
         * digits after the decimal point; for negative values also a
         * positive result is produced
         *
         * @return  the positive fractional part after the decimal point
         *          of current value
         */
        Real fractionalPart () const
        {
            return fractionalPart(*this);
        }

        /*--------------------*/

        /**
          * Returns the real modulus of current and other value;
          * brings result into the range [0,i)
          *
          * @param[in] d  the real divisor value
          * @return remainder of division in range [0,i)
          */
        Real mod (IN Real d) const
        {
            return mod(*this, d);
        }

        /*--------------------*/

        /**
          * Returns the natural logarithm of the current value.
          *
          * @return  ln(x)
          */
        Real log () const
        {
            return log(*this);
        }

        /*--------------------*/

        /**
         * Returns current value to the <C>exponent</C> power.
         *
         * @param[in] exponent  some real value
         * @return  current^exponent
         */
        Real power (IN Real exponent) const
        {
            return power(*this, exponent);
        }

        /*--------------------*/

        /**
         * Rounds current value to <C>decimalPlaceCount</C> decimal
         * places and returns result.
         *
         * @param[in] decimalPlaceCount  the number of decimal places to keep
         *                               by rounding
         * @return  the current real value rounded to
         *          <C>decimalPlaceCount</C> decimal places
         */
        Real round (IN Natural decimalPlaceCount = 0) const
        {
            return round(*this, decimalPlaceCount);
        }

        /*--------------------*/

        /**
          * Returns the sine of current value.
          *
          * @return  the sine of current value
          */
        Real sin () const
        {
            return sin(*this);
        }

        /*--------------------*/

        /**
          * Returns the square of current value.
          *
          * @return x^2
          */
        Real sqr () const
        {
            return sqr(*this);
        }

        /*--------------------*/

        /**
          * Returns the square root of the current value.
          *
          * @return x^{1/2}
          */
        Real sqrt () const
        {
            return sqrt(*this);
        }

        /*--------------------*/
        /* constants          */
        /*--------------------*/

        static const Real pi;        /**< pi */
        static const Real twoPi;     /**< 2*pi */
        static const Real infinity;  /**< infinity */

    };

    inline const Real Real::pi{3.1415926535897932385};
    inline const Real Real::twoPi{Real{2} * pi};
    inline const Real Real::infinity{1e99};

};
