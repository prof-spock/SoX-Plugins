/**
 * @file
 * The <C>GenericListInterpolator</C> specification defines a wrapper
 * object doing an interpolation on a list of values.
 *
 * @author Dr. Thomas Tensi
 * @date   2025-04
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "GenericList.h"
#include "IntegerList.h"
#include "MyArray.h"
#include "RealList.h"

/*--------------------*/

using BaseTypes::Containers::IntegerList;
using BaseTypes::Containers::RealList;
using BaseTypes::Containers::Array_at;
using BaseTypes::Containers::Array_set;

/*====================*/

namespace BaseTypes::GenericTypes {

    /** the position of the reference point when interpolating; it may
     * be at the begin of the interval (using other points with higher
     * indices), at the end (using points with lower indices) or at
     * the center (using points with lower and higher indices);
     * typically for the interpolation a center position of the
     * reference point is preferred */
    enum class ListInterpolatorPointLocation {
        atBegin, atCenter, atEnd
    };

    /*--------------------*/

    /**
     * Returns string representation of point location <C>p</C>.
     *
     * @param[in] p   point location to be converted to string
     * @return string representation
     */
    String pointLocationToString (IN ListInterpolatorPointLocation p);
    
    /*====================*/
    
    /**
     * A <C>GenericListInterpolator</C> specification defines a
     * wrapper object doing an interpolation on a list of values.
     * This value list has some element count and is interpolated
     * (e.g. when implementing delay lines with fractional delay line
     * length). It is always assumed that the underlying value list is
     * cyclic.
     */
    template <typename ListType, typename ElementType>
    class GenericListInterpolator
    {
        private:

            /** the list object to be interpolated */
            ListType* _list;

            /** the element count of the list */
            Natural _elementCount;

            /** the real-valued list length */
            Real _listLength;

            /** order of interpolation polynomial */
            Natural _interpolationOrder;

            /** offsets in base point list */
            IntegerList _offsetList;

            /** barymetric weights list */
            RealList _weightList;

            /*--------------------*/
            /* setup              */
            /*--------------------*/

            /**
             * Sets coefficient lists containing offsets and weights
             * using <C>interpolationOrder</C> and
             * <C>pointLocation</C>.
             *
             * @param[in] interpolationOrder  order of interpolation
             *                                polynomial
             * @param[in] pointLocation       position of reference
             *                                point in interpolation
             *                                interval
             */
            void _setInterpolationCoefficients
                     (IN Natural interpolationOrder,
                      IN ListInterpolatorPointLocation pointLocation)
            {
                _interpolationOrder = interpolationOrder;
                _offsetList.clear();
                _weightList.clear();
                
                const Integer offset =
                    (pointLocation == ListInterpolatorPointLocation::atCenter
                     ? interpolationOrder / 2
                     : (pointLocation == ListInterpolatorPointLocation::atBegin
                        ? 0 : interpolationOrder));

                for (Natural j = 0;  j <= interpolationOrder; j++) {
                    _offsetList.append(Integer{j} - offset);
                    Real weight = Real::one;
                        
                    for (Natural k = 0;  k <= interpolationOrder; k++) {
                        if (j != k) {
                            weight *= Real{j} - Real{k};
                        }
                    }

                    _weightList.append(Real::one / weight);
                }
            }

        public:

            /*--------------------*/
            /* construction       */
            /*--------------------*/

            /**
             * Initializes interpolator to point to no list and have a
             * length of zero.
             */
            GenericListInterpolator
                (IN Natural interpolationOrder = 0)
                : _list{nullptr},
                  _elementCount{0},
                  _listLength{0.0},
                  _interpolationOrder{0},
                  _offsetList{},
                  _weightList{}
            {
            }

            /*--------------------*/
            /* setup              */
            /*--------------------*/

            /**
             * Sets interpolator to point to <C>list</C>, have
             * <C>elementCount</C> base points, an real-valued length
             * of <C>listLength</C> and an order of interpolation of
             * <C>interpolationOrder</C> and <C>pointLocation</C>.
             *
             * @param[in] list                list to be interpolated
             * @param[in] elementCount        count of base points in
             *                                list
             * @param[in] listLength          assumed length of list
             * @param[in] interpolationOrder  order of interpolation
             *                                polynomial
             * @param[in] pointLocation       position of reference
             *                                point in interpolation
             *                                interval
             */
            void set (IN ListType& list,
                      IN Natural elementCount,
                      IN Real listLength,
                      IN Natural interpolationOrder,
                      IN ListInterpolatorPointLocation pointLocation
                             = ListInterpolatorPointLocation::atCenter)
            {
                _list               = (ListType*) &list;
                _elementCount       = elementCount;
                _listLength         = listLength;

                _setInterpolationCoefficients(interpolationOrder,
                                              pointLocation);
            }

            /*--------------------*/
            /* access             */
            /*--------------------*/

            /**
             * Returns interpolated value at <C>position</C> by
             * applying a polynomial on the underlying data list.
             *
             * @param [in] position  position where underlying list
             *                       shall be evaluated
             */
            ElementType at (IN Real position) const
            {
                const Real adaptedPosition = position.mod(_listLength);
                const Real fractionalPart = adaptedPosition.fractionalPart();
                Natural nPosition{Real::floor(adaptedPosition)};

                ElementType result;

                if (fractionalPart == 0.0 || _interpolationOrder == 0) {
                    nPosition %= _elementCount;
                    result = _list->at(nPosition);
                } else {
                    /* do a Lagrange barycentric interpolation */
                    Real* xDifferenceList =
                        Array_createLocal(Real, _interpolationOrder + 1);
                    ElementType* yList =
                        Array_createLocal(ElementType,
                                          _interpolationOrder + 1);

                    /* collect x-differences and values into lists */
                    for (Natural i = 0;  i <= _interpolationOrder;  i++) {
                        Integer j = _offsetList.at(i);
                        Array_set(xDifferenceList, i,
                                  fractionalPart - Real{j});
                        j = (j + nPosition + _elementCount) % _elementCount;
                        Array_set(yList, i, _list->at(Natural{j}));
                    }

                    /* calculate the ell-function: i.e. the product of
                       (x - x_i) for all i */
                    Real ell{Real::one};

                    for (Natural i = 0;  i <= _interpolationOrder;  i++) {
                        ell *= Array_at(xDifferenceList, i);
                    }

                    /* add all Lagrange terms w_i*f(i)/(x-x_i) for all i */
                    for (Natural i = 0;  i <= _interpolationOrder;  i++) {
                        const ElementType delta{
                            _weightList.at(i) * Array_at(yList, i)
                            / Array_at(xDifferenceList, i)};
                        result = (i == 0 ? delta : result + delta);
                    }

                    /* multiply the Lagrange term sum by the
                       ell-function */
                    result *= ell;
                }

                return result;
            }

    };

}
