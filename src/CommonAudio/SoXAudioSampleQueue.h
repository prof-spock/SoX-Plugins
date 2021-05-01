/**
 * @file
 * The <C>SoXAudioSampleQueue</C> specification defines a ring buffer
 * for audio samples with left and right shifts and access to first
 * and last elements in queue.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Natural.h"
#include "SoXAudioSample.h"
#include "MyString.h"

#include "GenericSequence.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::BaseTypes::Containers::GenericSequence;
using SoXPlugins::CommonAudio::SoXAudioSample;

/*====================*/

namespace SoXPlugins::CommonAudio {

    /**
     * A <C>SoXAudioSampleQueue</C> object is a ring buffer for audio
     * samples with left and right shifts and access to first and last
     * elements in queue.
     */
    struct SoXAudioSampleQueue {

        /**
         * Defines new queue with arbitrary length.
         */
        SoXAudioSampleQueue ();

        /*--------------------*/

        /**
         * Defines new queue with arbitrary length and given
         * initial length.
         *
         * @param[in] length  initial length of queue
         */
        SoXAudioSampleQueue (IN Natural length);

        /*--------------------*/

        /**
         * Destroys queue
         */
        ~SoXAudioSampleQueue ();

        /*--------------------*/

        /**
         * Returns string representation of sample queue.
         *
         * @return string representation
         */
        String toString() const;

        /*--------------------*/

        /**
         * Sets entries in sample queue to zero, but does not
         * change effective and maximum length
         */
        void setToZero ();

        /*--------------------*/

        /**
         * Sets length of queue to <C>length</C> entries
         *
         * @param[in] length  new count of samples in queue
         */
        void setLength (IN Natural length);

        /*--------------------*/

        /**
         * Returns length of queue
         *
         * @return count of samples in queue
         */
        Natural length () const;

        /*--------------------*/

        /**
         * Gets sample in queue at <C>position</C> (where position
         * starts at 0)
         *
         * @param[in] position  position in sample queue
         * @return sample value at position
         */
        const SoXAudioSample& at (IN Natural position) const;

        /*--------------------*/

        /**
         * Gets sample reference into queue at <C>position</C> (where
         * position starts at 0)
         *
         * @param[in] position  position in sample queue
         * @return sample value reference at position
         */
        SoXAudioSample& at (IN Natural position);

        /*--------------------*/

        /**
         * Gets sample in queue at <C>position</C> (where position
         * starts at 0)
         *
         * @param[in] position  position in sample queue
         * @return sample value at position
         */
        const SoXAudioSample& operator [] (IN Natural position) const;

        /*--------------------*/

        /**
         * Gets sample reference into queue at <C>position</C> (where
         * position starts at 0)
         *
         * @param[in] position  position in sample queue
         * @return sample value reference at position
         */
        SoXAudioSample& operator [] (IN Natural position);

        /*--------------------*/

        /**
         * Gets first sample of queue
         *
         * @return first sample value
         */
        SoXAudioSample first () const;

        /*--------------------*/

        /**
         * Gets last sample of queue
         *
         * @return last sample value
         */
        SoXAudioSample last () const;

        /*--------------------*/

        /**
         * Sets sample in queue at <C>position</C> to
         * <C>sample</C>
         *
         * @param[in] position  the position in list (starting at 0)
         *                      where sample shall be inserted
         * @param[in] sample    the sample to be inserted
         */
        void setAt (IN Natural position,
                    IN SoXAudioSample sample);

        /*--------------------*/

        /**
         * Sets first sample in queue to <C>sample</C>
         *
         * @param[in] sample  the sample to be inserted as first
         *                    element
         */
        void setFirst (IN SoXAudioSample sample);

        /*--------------------*/

        /**
         * Sets last sample in queue to <C>sample</C>
         *
         * @param[in] sample  the sample to be inserted as last
         *                    element
         */
        void setLast (IN SoXAudioSample sample);

        /*--------------------*/

        /**
         * Shifts contents of queue by one position to the left
         * (making space for a new last entry); <C>sample</C> is
         * inserted as the new last entry
         *
         * @param[in] sample  the sample to be inserted as last
         *                    element after left-shift
         */
        void shiftLeft (IN SoXAudioSample sample);

        /*--------------------*/

        /**
         * Shifts contents of queue by one position to the right
         * (making space for a new first entry); <C>sample</C> is
         * inserted as the new first entry
         *
         * @param[in] sample  the sample to be inserted as first
         *                    element after right-shift
         */
        void shiftRight (IN SoXAudioSample sample);

        /*--------------------*/
        /*--------------------*/

        private:

            /** the allocated length of the queue */
            Natural _allocatedLength;

            /** the effective length of the queue */
            Natural _length;

            /** the index of the first element in the ring
             * buffer */
            Natural _firstIndex;

            /** the elements of the queue as a vector */
            GenericSequence<SoXAudioSample> _data;

    };
};

//============================================================

#ifndef DEBUG
    //production code is inlined
    #include "SoXAudioSampleQueue.cpp-inc"
#endif
