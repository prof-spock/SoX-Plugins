/**
 * @file
 * The <C>SoXIIRFilter</C> specification defines an infinite impulse
 * response filter.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */


#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Natural.h"
#include "Real.h"
#include "RealList.h"
#include "SoXAudioSampleQueue.h"
#include "MyString.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::BaseTypes::Containers::RealList;
using SoXPlugins::CommonAudio::SoXAudioSampleQueue;

/*====================*/

namespace SoXPlugins::CommonAudio {

    /**
     * A <C>SoXIIRFilter</C> object is an infinite impulse response
     * filter.
    */
    struct SoXIIRFilter {

        /**
         * Creates an IIR filter with <C>order</C> as a null
         * filter
         *
         * @param[in] order  the (fixed) order of the IIR filter
         */
        SoXIIRFilter (IN Natural order);

        /*--------------------*/

        ~SoXIIRFilter ();

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
         * Applies IIR filter to <C>inputQueue</C> and
         * <C>outputQueue</C>; assumes that first entry in input
         * queue is current sample and writes result sample into
         * top of output queue
         *
         * @param[in] inputQueue      the queue (of correct length)
         *                            with the input samples
         * @param[inout] outputQueue  the queue (of correct length)
         *                            with the output samples
         */
        void apply (IN SoXAudioSampleQueue& inputQueue,
                    INOUT SoXAudioSampleQueue& outputQueue) const;

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
    #include "SoXIIRFilter.cpp-inc"
#endif
