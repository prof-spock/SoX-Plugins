/**
 * @file
 * The <C>DelayLineList</C> specification defines a list of delay
 * lines for audio data points.
 *
 * @author Dr. Thomas Tensi
 * @date   2026-05
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "DelayLine.h"
#include "GenericList.h"

/*====================*/

using Audio::DelayLine;
using BaseTypes::GenericTypes::GenericList;

/*====================*/

namespace Audio {

    /**
     * Returns name of delay line list type
     *
     * @return type name
     */
    String _delayLineListTypeName ();
    
    /*--------------------*/

    
    /**
     * A <C>DelayLineList</C> object is a list of delay lines for
     * audio data points.
     */
    struct DelayLineList
        : public GenericList<DelayLine,
                             DelayLine::toString,
                             _delayLineListTypeName > {

        /*--------------------*/
        /* constructors       */
        /*--------------------*/

        /**
         * Initializes list of delay lines containing <C>count</C>
         * lines with intial length <C>delayLineLength</C>.
         *
         * @param[in] count               initial count of delay lines
         * @param[in] delayLineLength     initial length of each delay
         *                                line
         * @return  delay line list
         */
        DelayLineList (IN Natural count = 0,
                       IN Natural delayLineLength = 0);
        
        /*--------------------*/
        /* change             */
        /*--------------------*/

        /**
         * Sets length of delay lines to <C>elementLength</C>.
         *
         * @param[in]  elementLength  new length of each delay line
         */
        void setElementLength (IN Real elementLength);

        /*--------------------*/

        /**
         * Sets interpolation parameters for all delay lines to
         * <C>interpolationOrder</C> and <C>pointLocation</C>.
         *
         * @param[in] interpolationOrder  order of interpolation polynomial
         * @param[in] pointLocation       position of reference point
         *                                in interpolation interval
         */
        void setElementInterpolation
                 (IN Natural interpolationOrder = 0,
                  IN ListInterpolatorPointLocation pointLocation
                         = ListInterpolatorPointLocation::atCenter);

        /*--------------------*/

        /**
         * Sets all entries in the delay lines to zero.
         */
        void setToZero ();

    };

}

/*============================================================*/

#ifndef DEBUG
    /* production code is inlined */
    #include "DelayLineList.cpp-inc"
#endif
