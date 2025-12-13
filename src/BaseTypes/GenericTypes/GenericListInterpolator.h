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

#include "Real.h"

/*--------------------*/

using BaseTypes::Primitives::Real;

/*====================*/

namespace BaseTypes::GenericTypes {

    /** the maximum degree for interpolation */
    const Natural maximumInterpolationDegree = 3;

    /*====================*/
    
    /**
     * A <C>GenericListInterpolator</C> specification defines a
     * wrapper object doing an interpolation on a list of values.
     */
    template <typename ListType, typename ElementType>
    class GenericListInterpolator
    {
        private:

            /** the list object to be interpolated */
            ListType* _list;

            /** the length of the list */
            Natural _length;

            /** information whether data list is cyclic */
            Boolean _isCyclic;

            /** degree of interpolation polynomial */
            Natural _degree;

            /** barymetric weights */
            Real _weightList[4];

        public:

            /*--------------------*/
            /* construction       */
            /*--------------------*/

            GenericListInterpolator ()
                : _list{NULL},
                  _length{0},
                  _isCyclic{false},
                  _degree{0},
                  _weightList{}
            {
            }

            /*--------------------*/
            /* setup              */
            /*--------------------*/

            void set (IN ListType& list,
                      IN Natural length,
                      IN Boolean isCyclic,
                      IN Natural degree)
            {
                _list     = (ListType*) &list;
                _length   = length;
                _isCyclic = isCyclic;
                _degree   = degree;

                /* update weight list */
                for (int i = 0;  i <= _degree;  i++) {
                    Real& weight = _weightList[i];
                    weight = Real::one;
                    
                    for (int j = 0;  j <= _degree;  j++) {
                        if (i != j) {
                            weight *= Real{(double) (i - j)};
                        }
                    }

                    weight = Real::one / weight;
                }
            }

            /*--------------------*/
            /* access             */
            /*--------------------*/

            /**
             * Returns interpolated value at <C>position</C> by
             * applying a polynomial on the underlying data list.
             */
            ElementType at (IN Real position) const
            {
                const Real fractionalPart = position.fractionalPart();
                Natural nPosition{Real::floor(position)};
                ElementType result;

                if (fractionalPart == 0.0 || _degree == 0) {
                    if (_isCyclic) {
                        nPosition %= _length;
                    }

                    result = _list->at(nPosition);
                } else {
                    /* do a Lagrange barycentric interpolation */
                    const Natural offset{_degree / 2};
                    ElementType valueList[4];

                    /* collect values into value list */
                    for (Natural i = 0;  i <= _degree;  i++) {
                        Integer j = Integer{nPosition + i - offset};

                        if (_isCyclic) {
                            j = (j + _length) % _length;
                        } else {
                            j = (j <= 0 ? 0
                                 : (j < _length ? j : _length - 1));
                        }

                        valueList[(int) i] = _list->at(Natural{j});
                    }

                    const Real xOffset = fractionalPart + Real{offset};
                    Real ell{Real::one};

                    for (Natural i = 0;  i <= _degree;  i++) {
                        ell *= (xOffset - Real{i});
                    }

                    for (Natural i = 0;  i <= _degree;  i++) {
                        ElementType delta{_weightList[(int) i]
                                          / (xOffset - Real{i})};
                        delta *= (ell * valueList[(int) i]);
                        result = (i == 0 ? delta : result + delta);
                    }
                }

                return result;
            }

    };

}
