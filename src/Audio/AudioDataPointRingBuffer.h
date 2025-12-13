/**
 * @file
 * The <C>AudioDataPointRingBuffer</C> specification defines a ring
 * buffer for audio data points with left and right shifts and access to
 * first and last elements in queue.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "AudioDataPointList.h"

/*--------------------*/

using Audio::AudioDataPointList;

/*====================*/

namespace Audio {

    /**
     * A <C>AudioDataPointRingBuffer</C> object is a ring buffer for
     * audio data points with left and right shifts and access to first
     * and last elements in queue.
     */
    struct AudioDataPointRingBuffer {

        /**
         * Defines new ring buffer with arbitrary length.
         */
        AudioDataPointRingBuffer ();

        /*--------------------*/

        /**
         * Defines new ring buffer with arbitrary length and given
         * initial length.
         *
         * @param[in] length  initial length of ring buffer
         */
        AudioDataPointRingBuffer (IN Natural length);

        /*--------------------*/

        /**
         * Destroys ring buffer
         */
        ~AudioDataPointRingBuffer ();

        /*--------------------*/

        /**
         * Returns string representation of data point ring buffer.
         *
         * @return string representation
         */
        String toString() const;

        /*--------------------*/

        /**
         * Sets entries in data point ring buffer to zero, but does
         * not change effective and maximum length
         */
        void setToZero ();

        /*--------------------*/

        /**
         * Sets length of ring buffer to <C>length</C> entries
         *
         * @param[in] length  new count of data points in ring buffer
         */
        void setLength (IN Natural length);

        /*--------------------*/

        /**
         * Returns length of ring buffer
         *
         * @return count of data points in ring buffer
         */
        Natural length () const;

        /*--------------------*/

        /**
         * Gets data point in ring buffer at <C>position</C> (where
         * position starts at 0)
         *
         * @param[in] position  position in data point ring buffer
         * @return data point value at position
         */
        const AudioDataPoint& at (IN Natural position) const;

        /*--------------------*/

        /**
         * Gets data point reference into ring buffer at
         * <C>position</C> (where position starts at 0)
         *
         * @param[in] position  position in data point ring buffer
         * @return data point value reference at position
         */
        AudioDataPoint& at (IN Natural position);

        /*--------------------*/

        /**
         * Gets data point in ring buffer at <C>position</C> (where
         * position starts at 0)
         *
         * @param[in] position  position in dataPoint ring buffer
         * @return data point value at position
         */
        const AudioDataPoint& operator [] (IN Natural position) const;

        /*--------------------*/

        /**
         * Gets data point reference into ring buffer at
         * <C>position</C> (where position starts at 0)
         *
         * @param[in] position  position in dataPoint ring buffer
         * @return data point value reference at position
         */
        AudioDataPoint& operator [] (IN Natural position);

        /*--------------------*/

        /**
         * Gets first data point of ring buffer
         *
         * @return first data point value
         */
        AudioDataPoint first () const;

        /*--------------------*/

        /**
         * Gets last data point of ring buffer
         *
         * @return last data point value
         */
        AudioDataPoint last () const;

        /*--------------------*/

        /**
         * Sets data point in ring buffer at <C>position</C> to
         * <C>dataPoint</C>
         *
         * @param[in] position   the position in list (starting at 0)
         *                       where data point shall be inserted
         * @param[in] dataPoint  the data point to be inserted
         */
        void set (IN Natural position,
                  IN AudioDataPoint dataPoint);

        /*--------------------*/

        /**
         * Sets first data point in ring buffer to <C>dataPoint</C>
         *
         * @param[in] dataPoint  the data point to be inserted as first
         *                       element
         */
        void setFirst (IN AudioDataPoint dataPoint);

        /*--------------------*/

        /**
         * Sets last data point in ring buffer to <C>dataPoint</C>
         *
         * @param[in] dataPoint  the data point to be inserted as last
         *                       element
         */
        void setLast (IN AudioDataPoint dataPoint);

        /*--------------------*/

        /**
         * Shifts contents of ring buffer by one position to the left
         * (making space for a new last entry); <C>dataPoint</C> is
         * inserted as the new last entry
         *
         * @param[in] dataPoint  the data point to be inserted as last
         *                       element after left-shift
         */
        void shiftLeft (IN AudioDataPoint dataPoint);

        /*--------------------*/

        /**
         * Shifts contents of ring buffer by one position to the right
         * (making space for a new first entry); <C>dataPoint</C> is
         * inserted as the new first entry
         *
         * @param[in] dataPoint  the data point to be inserted as first
         *                       element after right-shift
         */
        void shiftRight (IN AudioDataPoint dataPoint);

        /*--------------------*/

        /**
         * Gets all elements in ring buffer in ordered form into
         * <C>elementArray</C>.  Assumes that capacity of destination
         * is large enough.
         *
         * @param[out] elementArray  array of data points
         */
        void toArray (OUT AudioDataPoint* elementArray) const;

        /*--------------------*/
        /*--------------------*/

        private:

            /** the allocated length of the ring buffer */
            Natural _allocatedLength;

            /** the effective length of the ring buffer */
            Natural _length;

            /** the index of the first element in the ring
             * buffer */
            Natural _firstIndex;

            /** the elements of the ring buffer as a vector */
            GenericList<AudioDataPoint> _data;

    };
}

/*============================================================*/

#ifndef DEBUG
    /* production code is inlined */
    #include "AudioDataPointRingBuffer.cpp-inc"
#endif
