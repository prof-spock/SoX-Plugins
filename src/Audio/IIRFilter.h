/**
 * @file
 * The <C>IIRFilter</C> specification defines an infinite impulse
 * response filter.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "RealList.h"
#include "AudioSampleRingBuffer.h"

/*====================*/

using BaseTypes::Containers::RealList;
using Audio::AudioSampleRingBuffer;

/*====================*/

namespace Audio {

    /**
     * A <C>IIRFilter</C> object is an infinite impulse response
     * filter.
    */
    struct IIRFilter {

        /**
         * Creates an IIR filter with <C>order</C> as a null
         * filter
         *
         * @param[in] order  the (fixed) order of the IIR filter
         */
        IIRFilter (IN Natural order);

        /*--------------------*/

        ~IIRFilter ();

        /*--------------------*/

        /**
         * Returns string representation of filter.
         *
         * @return string representation
         */
        String toString() const;

        /*--------------------*/

        /**
         * Resets filter to a null filter
         */
        void clear ();

        /*--------------------*/

        /**
         * Sets IIR filter with all zeros except for the <C>b0</C>
         * value; b0 = 1 is an identity filter, b0 = 0 a null
         * filter
         *
         * @param[in] b0  the b0 value of the IIR filter
         */
        void set (IN Real b0);

        /*--------------------*/

        /**
         * Sets the parameters of an IIR filter of order 3
         *
         * @param[in] b0  the b0 value of the third-order IIR filter
         * @param[in] b1  the b1 value of the third-order IIR filter
         * @param[in] b2  the b2 value of the third-order IIR filter
         * @param[in] a0  the a0 value of the third-order IIR filter
         * @param[in] a1  the a1 value of the third-order IIR filter
         * @param[in] a2  the a2 value of the third-order IIR filter
         */
        void set (IN Real b0, IN Real b1, IN Real b2,
                  IN Real a0, IN Real a1, IN Real a2);

        /*--------------------*/

        /**
         * Sets the parameters of an IIR filter of order 5
         *
         * @param[in] b0  the b0 value of the fifth-order IIR filter
         * @param[in] b1  the b1 value of the fifth-order IIR filter
         * @param[in] b2  the b2 value of the fifth-order IIR filter
         * @param[in] b3  the b3 value of the fifth-order IIR filter
         * @param[in] b4  the b4 value of the fifth-order IIR filter
         * @param[in] a0  the a0 value of the fifth-order IIR filter
         * @param[in] a1  the a1 value of the fifth-order IIR filter
         * @param[in] a2  the a2 value of the fifth-order IIR filter
         * @param[in] a3  the a3 value of the fifth-order IIR filter
         * @param[in] a4  the a4 value of the fifth-order IIR filter
         */
        void set (IN Real b0, IN Real b1, IN Real b2,
                  IN Real b3, IN Real b4,
                  IN Real a0, IN Real a1, IN Real a2,
                  IN Real a3, IN Real a4);

        /*--------------------*/

        /**
         * Applies IIR filter to <C>inputBuffer</C> and
         * <C>outputBuffer</C>; assumes that first entry in input
         * buffer is current sample and writes result sample into top
         * of output buffer
         *
         * @param[in] inputBuffer     the buffer (of correct length)
         *                            with the input samples
         * @param[inout] outputBuffer the buffer (of correct length)
         *                            with the output samples
         */
        void apply (IN AudioSampleRingBuffer& inputBuffer,
                    INOUT AudioSampleRingBuffer& outputBuffer) const;

        /*--------------------*/
        /*--------------------*/

        protected:

            /** the internal data of the filter */
            RealList _data;

            /** the filter order */
            Natural _order;

    };

}

//============================================================

#ifndef DEBUG
    //production code is inlined
    #include "IIRFilter.cpp-inc"
#endif
