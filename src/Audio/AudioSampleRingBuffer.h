/**
 * @file
 * The <C>AudioSampleRingBuffer</C> specification defines a ring
 * buffer for audio samples with left and right shifts and access to
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

#include "AudioSampleList.h"

/*--------------------*/

using Audio::AudioSampleList;

/*====================*/

namespace Audio {

    /**
     * A <C>AudioSampleRingBuffer</C> object is a ring buffer for
     * audio samples with left and right shifts and access to first
     * and last elements in queue.
     */
    struct AudioSampleRingBuffer {

        /**
         * Defines new ring buffer with arbitrary length.
         */
        AudioSampleRingBuffer ();

        /*--------------------*/

        /**
         * Defines new ring buffer with arbitrary length and given
         * initial length.
         *
         * @param[in] length  initial length of ring buffer
         */
        AudioSampleRingBuffer (IN Natural length);

        /*--------------------*/

        /**
         * Destroys ring buffer
         */
        ~AudioSampleRingBuffer ();

        /*--------------------*/

        /**
         * Returns string representation of sample ring buffer.
         *
         * @return string representation
         */
        String toString() const;

        /*--------------------*/

        /**
         * Sets entries in sample ring buffer to zero, but does not
         * change effective and maximum length
         */
        void setToZero ();

        /*--------------------*/

        /**
         * Sets length of ring buffer to <C>length</C> entries
         *
         * @param[in] length  new count of samples in ring buffer
         */
        void setLength (IN Natural length);

        /*--------------------*/

        /**
         * Returns length of ring buffer
         *
         * @return count of samples in ring buffer
         */
        Natural length () const;

        /*--------------------*/

        /**
         * Gets sample in ring buffer at <C>position</C> (where position
         * starts at 0)
         *
         * @param[in] position  position in sample ring buffer
         * @return sample value at position
         */
        const AudioSample& at (IN Natural position) const;

        /*--------------------*/

        /**
         * Gets sample reference into ring buffer at <C>position</C> (where
         * position starts at 0)
         *
         * @param[in] position  position in sample ring buffer
         * @return sample value reference at position
         */
        AudioSample& at (IN Natural position);

        /*--------------------*/

        /**
         * Gets sample in ring buffer at <C>position</C> (where position
         * starts at 0)
         *
         * @param[in] position  position in sample ring buffer
         * @return sample value at position
         */
        const AudioSample& operator [] (IN Natural position) const;

        /*--------------------*/

        /**
         * Gets sample reference into ring buffer at <C>position</C> (where
         * position starts at 0)
         *
         * @param[in] position  position in sample ring buffer
         * @return sample value reference at position
         */
        AudioSample& operator [] (IN Natural position);

        /*--------------------*/

        /**
         * Gets first sample of ring buffer
         *
         * @return first sample value
         */
        AudioSample first () const;

        /*--------------------*/

        /**
         * Gets last sample of ring buffer
         *
         * @return last sample value
         */
        AudioSample last () const;

        /*--------------------*/

        /**
         * Sets sample in ring buffer at <C>position</C> to
         * <C>sample</C>
         *
         * @param[in] position  the position in list (starting at 0)
         *                      where sample shall be inserted
         * @param[in] sample    the sample to be inserted
         */
        void set (IN Natural position,
                  IN AudioSample sample);

        /*--------------------*/

        /**
         * Sets first sample in ring buffer to <C>sample</C>
         *
         * @param[in] sample  the sample to be inserted as first
         *                    element
         */
        void setFirst (IN AudioSample sample);

        /*--------------------*/

        /**
         * Sets last sample in ring buffer to <C>sample</C>
         *
         * @param[in] sample  the sample to be inserted as last
         *                    element
         */
        void setLast (IN AudioSample sample);

        /*--------------------*/

        /**
         * Shifts contents of ring buffer by one position to the left
         * (making space for a new last entry); <C>sample</C> is
         * inserted as the new last entry
         *
         * @param[in] sample  the sample to be inserted as last
         *                    element after left-shift
         */
        void shiftLeft (IN AudioSample sample);

        /*--------------------*/

        /**
         * Shifts contents of ring buffer by one position to the right
         * (making space for a new first entry); <C>sample</C> is
         * inserted as the new first entry
         *
         * @param[in] sample  the sample to be inserted as first
         *                    element after right-shift
         */
        void shiftRight (IN AudioSample sample);

        /*--------------------*/

        /**
         * Gets all elements in ring buffer in ordered form into
         * <C>elementArray</C>.  Assumes that capacity of target is
         * large enough.
         *
         * @param[out] elementArray  array of samples
         */
        void toArray (OUT AudioSample* elementArray) const;

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
            GenericList<AudioSample> _data;

    };
}

/*============================================================*/

#ifndef DEBUG
    /* production code is inlined */
    #include "AudioSampleRingBuffer.cpp-inc"
#endif
