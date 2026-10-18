/**
 * @file
 * The <C>DelayLine</C> specification defines a ring buffer for audio
 * data points with real-valued index access and real-valued length.
 *
 * @author Dr. Thomas Tensi
 * @date   2025-12
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "AudioDataPointRingBuffer.h"
#include "GenericListInterpolator.h"

/*--------------------*/

using Audio::AudioDataPointRingBuffer;
using BaseTypes::GenericTypes::GenericListInterpolator;
using BaseTypes::GenericTypes::ListInterpolatorPointLocation;

/*====================*/

namespace Audio {

    /** the internal ring buffer interpolator */
    typedef GenericListInterpolator<AudioDataPointRingBuffer,
                                    AudioDataPoint>
        AudioDataPointRingBufferInterpolator;
    
    /**
     * A <C>DelayLine</C> object is a ring buffer for audio data
     * points with real-valued index access and real-valued length; it
     * differs from an <C>AudioDataPointRingBuffer</C> by
     * interpolating function values between the base points instead
     * of assuming a staircase function.
     */
    struct DelayLine {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Defines new delay line with arbitrary length and given
         * initial length, interpolation order and reference point
         * location.
         *
         * @param[in] length              initial real-valued length
         *                                of delay line
         * @param[in] interpolationOrder  initial order of
         *                                interpolation polynomial
         * @param[in] pointLocation       initial position of reference
         *                                point in interpolation interval
         */
        DelayLine (IN Real length = 0.0,
                   IN Natural interpolationOrder = 0,
                   IN ListInterpolatorPointLocation pointLocation
                          = ListInterpolatorPointLocation::atCenter);

        /*--------------------*/

        /**
         * Destroys delay line
         */
        ~DelayLine ();

        /*--------------------*/
        /* type conversion    */
        /*--------------------*/

        /**
         * Gets all elements in delay line in ordered form into
         * <C>elementArray</C>.  Assumes that capacity of destination
         * is large enough.
         *
         * @param[out] elementArray  array of data points
         */
        void toArray (OUT AudioDataPoint* elementArray) const;

        /*--------------------*/

        /**
         * (with at most <C>dataPointCount</C> data points of the list
         * included in the representation.
         *
         * @param[in] dataPointCount  count of data points to be
         *                            included in string
         * @return string representation
         */
        String toString (IN Natural dataPointCount
                             = Natural::maximumValue()) const;

        /*--------------------*/

        /**
         * Returns string representation of <C>delayLine</C>.
         *
         * @param[in] delayLine  delay line to be converted to a string
         * @return  string representation
         */
        static String toString (IN DelayLine& delayLine);

        /*--------------------*/
        /* property access    */
        /*--------------------*/

        /**
         * Returns interpolation order of delay line
         *
         * @return order of interpolation polynomial for the delay
         *         line
         */
        Natural interpolationOrder () const;

        /*--------------------*/

        /**
         * Tells whether delay line is empty.
         *
         * @return  information whether no <C>element</C> occurs in
         *          delay line
         */
        Boolean isEmpty () const;

        /*--------------------*/

        /**
         * Returns standard length of delay line
         *
         * @return length of delay line (as a real)
         */
        Real length () const;

        /*--------------------*/

        /**
         * Returns the reference point location for the delay line
         *
         * @return reference point location within interpolation
         *         interval
         */
        ListInterpolatorPointLocation referencePointLocation () const;

        /*--------------------*/
        /* element access     */
        /*--------------------*/

        /**
         * Gets data point in delay line at <C>position</C> (where
         * position starts at 0).
         *
         * @param[in] position  position in delay line
         * @return data point value at position
         */
        AudioDataPoint at (IN Real position) const;

        /*--------------------*/

        /**
         * Gets data point in delay line at first position (0.0)
         *
         * @return data point value at first position
         */
        AudioDataPoint first () const;

        /*--------------------*/
        /* change             */
        /*--------------------*/

        /**
         * Shifts contents of delay line by one position to the left
         * (making space for a new last entry); <C>dataPoint</C> is
         * inserted as the new last entry
         *
         * @param[in] dataPoint  the data point to be inserted as last
         *                       element after left-shift
         */
        void advance (IN AudioDataPoint dataPoint);

        /*--------------------*/

        /**
         * Sets data point in delay line at <C>position</C> (where
         * position starts at 0) to <C>dataPoint</C>.
         *
         * @param[in] position   position in delay line
         * @param[in] dataPoint  data point value at position
         */
        void set (IN Natural position,
                  IN AudioDataPoint dataPoint);

        /*--------------------*/

        /**
         * Sets interpolation order of delay line to
         * <C>interpolationOrder</C>
         *
         * @param[in] interpolationOrder  new order of interpolation
         *                                polynomial
         */
        void setInterpolationOrder (IN Natural interpolationOrder);

        /*--------------------*/

        /**
         * Sets length of delay line to <C>length</C> entries
         *
         * @param[in] length  new count of data points in delay line
         */
        void setLength (IN Real length);

        /*--------------------*/

        /**
         * Sets the reference point location for the delay line
         *
         * @param[in] pointLocation  reference point location within
         *                           interpolation interval
         */
        void setReferencePointLocation
                 (IN ListInterpolatorPointLocation pointLocation);

        /*--------------------*/

        /**
         * Sets entries in delay line to zero, but does not change
         * effective and maximum length
         */
        void setToZero ();

        /*--------------------*/
        /*--------------------*/

        private:

            /** the effective (real-valued) length of the delay
             * line */
            Real _length;

            /** the order of the interpolation polynomial */
            Natural _interpolationOrder;

            /** the reference point location in the interpolation
             * interval */
            ListInterpolatorPointLocation _referencePointLocation;

            /** the interpolator */
            AudioDataPointRingBufferInterpolator _interpolator;
        
            /** the elements of the delay line as a ring buffer */
            AudioDataPointRingBuffer _data;

            /*--------------------*/

            /**
             * Update underlying ring buffer and interpolator
             */
            void _updateData ();

    };
}

/*============================================================*/

#ifndef DEBUG
    /* production code is inlined */
    #include "DelayLine.cpp-inc"
#endif
