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

#include "GenericMap.h"
#include "IntegerList.h"
#include "RealList.h"

/*--------------------*/

using BaseTypes::GenericTypes::GenericMap;
using BaseTypes::Containers::IntegerList;
using BaseTypes::Containers::RealList;

/*====================*/

namespace BaseTypes::GenericTypes {

    /*--------------------*/
    /* class attributes   */
    /*--------------------*/

    /** the maximum polynomial order for interpolation */
    const Natural GLI_maximumInterpolationOrder = 3;

    /** mapping from interpolation order to offsets in base
        point list */
    static GenericMap<Natural, IntegerList> _GLI_orderToOffsetMap;

    /** mapping from interpolation order to barymetric weights */
    static GenericMap<Natural, RealList> _GLI_orderToWeightMap;

    /*====================*/
    
    /**
     * A <C>GenericListInterpolator</C> specification defines a
     * wrapper object doing an interpolation on a list of values.
     * This value list has some element count and may be either
     * stretched to a given length (e.g. when implementing waveforms
     * using prototype data) or left as is and interpolated (e.g. when
     * implementing delay lines with fractional delay line length). It
     * is always assumed that the underlying value list is cyclic.
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

            /** multiplication factor for the range or the underlying
             * data list to be used for positioning */
            Real _stretchFactor;

            /** order of interpolation polynomial */
            Natural _interpolationOrder;

            /*--------------------*/
            /* setup              */
            /*--------------------*/

            /**
             * Initializes maps with offsets and weights
             */
            static void _initializeMaps ()
            {
                _GLI_orderToOffsetMap.clear();
                _GLI_orderToWeightMap.clear();

                for (Natural i = 0; i <= GLI_maximumInterpolationOrder; i++) {
                    IntegerList offsetList;
                    RealList weightList;
                    const Integer offset = i / 2;

                    for (Natural j = 0;  j <= i; j++) {
                        Real weight = Real::one;
                        offsetList.append(Integer{j} - offset);
                        
                        for (Natural k = 0;  k <= i; k++) {
                            if (j != k) {
                                weight *= Real{j} - Real{k};
                            }
                        }

                        weightList.append(Real::one / weight);
                    }

                    _GLI_orderToOffsetMap.set(i, offsetList);
                    _GLI_orderToWeightMap.set(i, weightList);
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
            GenericListInterpolator ()
                : _list{NULL},
                  _elementCount{0},
                  _listLength{0.0},
                  _stretchFactor{1.0},
                  _interpolationOrder{0}
            {
                // initialize the static variables if empty
                if (_GLI_orderToOffsetMap.isEmpty()) {
                    _initializeMaps();
                }
            }

            /*--------------------*/
            /* setup              */
            /*--------------------*/

            /**
             * Sets interpolator to point to <C>list</C>, have
             * <C>elementCount</C> base points, an real-valued length
             * of <C>listLength</C>, using range stretching signified
             * by <C>isStretched</C> and an order of interpolation of
             * <C>interpolationOrder</C>.
             *
             * @param[in] list                list to be interpolated
             * @param[in] elementCount        count of base points in
             *                                list
             * @param[in] listLength          assumed length of list
             * @param[in] isStretched         tells whether base points
             *                                are stretched to length
             *                                or left as is
             * @param[in] interpolationOrder  order of interpolation
             *                                polynomial
             */
            void set (IN ListType& list,
                      IN Natural elementCount,
                      IN Real listLength,
                      IN Boolean isStretched,
                      IN Natural interpolationOrder)
            {
                _list               = (ListType*) &list;
                _elementCount       = elementCount;
                _listLength         = listLength;
                _stretchFactor      = (!isStretched || listLength == 0.0
                                       ? 1.0
                                       : Real{elementCount} / listLength);
                _interpolationOrder = interpolationOrder;
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
                const Real adaptedPosition =
                    (position * _stretchFactor).mod(_listLength);
                const Real fractionalPart = adaptedPosition.fractionalPart();
                Natural nPosition{Real::floor(adaptedPosition)};

                ElementType result;

                if (fractionalPart == 0.0 || _interpolationOrder == 0) {
                    nPosition %= _elementCount;
                    result = _list->at(nPosition);
                } else {
                    /* do a Lagrange barycentric interpolation */
                    const IntegerList& offsetList =
                        _GLI_orderToOffsetMap.at(_interpolationOrder);
                    const RealList& weightList =
                        _GLI_orderToWeightMap.at(_interpolationOrder);
                    RealList xDifferenceList;
                    GenericList<ElementType> yList;

                    /* collect x-differences and values into lists */
                    for (Natural i = 0;  i <= _interpolationOrder;  i++) {
                        Integer j = offsetList.at(i);
                        xDifferenceList.append(fractionalPart - Real{j});
                        j = (j + nPosition + _elementCount) % _elementCount;
                        yList.append(_list->at(Natural{j}));
                    }

                    /* calculate the ell-function: i.e. the product of
                       (x - x_i) for all i */
                    Real ell{Real::one};

                    for (Natural i = 0;  i <= _interpolationOrder;  i++) {
                        ell *= xDifferenceList.at(i);
                    }

                    /* add all Lagrange terms w_i*f(i)/(x-x_i) for all i */
                    for (Natural i = 0;  i <= _interpolationOrder;  i++) {
                        const ElementType delta{
                            weightList.at(i) * yList.at(i)
                            / xDifferenceList.at(i)};
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
