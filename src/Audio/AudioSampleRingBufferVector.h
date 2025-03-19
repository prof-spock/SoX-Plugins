/**
 * @file
 * The <C>AudioSampleRingBufferVector</C> specification defines a
 * two-dimensional list of ring buffers for audio samples.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "AudioSampleListVector.h"
#include "AudioSampleRingBuffer.h"

/*====================*/

using Audio::AudioSampleListVector;
using Audio::AudioSampleRingBuffer;

/*====================*/

namespace Audio {

    /**
     * A <C>AudioSampleRingBufferVector</C> object is a list of ring
     * buffers for audio samples with selection by index and left or
     * right rotation on the individual buffers.
     */
    struct AudioSampleRingBufferVector {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Makes a list of sample ring buffers for <C>channelCount</C>
         * channels with arbitrary length; depending on
         * <C>hasTwoRingBuffersPerChannel</C> either one or two ring
         * buffers are provided per channel; initial sample ring
         * buffer length is <C>sampleRingBufferLength</C>
         *
         * @param[in] channelCount                 number of channels
         *                                         in vector
         * @param[in] hasTwoRingBuffersPerChannel  tells whether each
         *                                         channel requires two
         *                                         ring buffers (instead
         *                                         of one)
         * @param[in] sampleRingBufferLength       the length of each
         *                                         ring buffer (in samples)
         */
        AudioSampleRingBufferVector
            (IN Natural channelCount = 0,
             IN Boolean hasTwoRingBuffersPerChannel = false,
             IN Natural sampleRingBufferLength = 0);

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        /**
         * Returns count of all ring buffers
         *
         * @return count of ring buffers
         */
        Natural ringBufferCount () const;

        /*--------------------*/

        /**
         * Returns length of each ring buffer.
         *
         * @return  length of ring buffer
         */
        Natural ringBufferLength () const;

        /*--------------------*/
        /* property change    */
        /*--------------------*/

        /**
         * Sets count of all ring buffers to <C>count</C>
         *
         * @param[in] count  the new count of ring buffers
         */
        void setRingBufferCount (IN Natural count);

        /*--------------------*/

        /**
         * Sets length of all ring buffers to <C>length</C>
         *
         * @param[in] length  the new length of each ring buffer in
         *                    vector
         */
        void setRingBufferLength (IN Natural length);

        /*--------------------*/

        /**
         * Removes all sample ring buffers
         */
        void clear ();

        /*--------------------*/

        /**
         * Sets all entries in sample ring buffers to zero, but does
         * not change their effective and maximum length
         */
        void setToZero ();

        /*--------------------*/

        /**
         * Returns reference to sample ring buffer for channel with
         * <C>channelIndex</C> and <C>position</C> (where this may
         * be 0 or 1 when <C>_hasTwoRingBuffersPerChannel</C> has been
         * set
         *
         * @param[in] channelIndex  number of channel to be selected
         *                          (starting at zero)
         * @param[in] position      if channel has two ring buffers, an
         *                          additional selection parameter
         * @return reference to appropriate ring buffer in vector
         */
        const AudioSampleRingBuffer& at (IN Natural channelIndex,
                                         IN Natural position = 0) const;

        /*--------------------*/

        /**
         * Returns reference to sample ring buffer for channel with
         * <C>channelIndex</C> and <C>position</C> (where this may
         * be 0 or 1 when <C>_hasTwoRingBuffersPerChannel</C> has been
         * set
         *
         * @param[in] channelIndex  number of channel to be selected
         *                          (starting at zero)
         * @param[in] position      if channel has two ring buffers, an
         *                          additional selection parameter
         * @return reference to appropriate ring buffer in vector
         */
        AudioSampleRingBuffer& at (IN Natural channelIndex,
                                   IN Natural position = 0);

        /*--------------------*/

        /**
         * Returns reference to sample ring buffer for channel with
         * <C>channelIndex</C>; is only allowed when
         * <C>_hasTwoRingBuffersPerChannel</C> is not set
         *
         * @param[in] channelIndex  number of channel to be
         *                          selected (starting at zero)
         * @return reference to appropriate ring buffer in vector
         */
        AudioSampleRingBuffer& operator [] (IN Natural channelIndex);

        /*--------------------*/

        /**
         * Appends <C>sampleRingBuffer</C> as last element.
         *
         * @param[in] sampleRingBuffer  ring buffer to be appended as
         *                              last element of matrix
         */
        void append (IN AudioSampleRingBuffer& sampleRingBuffer);

        /*--------------------*/

        /**
         * Returns reference to last sample ring buffer in list.
         *
         * @return last sample ring buffer
         */
        AudioSampleRingBuffer& last ();

        /*--------------------*/
        /* conversion         */
        /*--------------------*/

        /**
         * Returns string representation of sample ring buffer list;;
         * if <C>audioFrameCount</C> is set, only that number of audio
         * frames with sample data will be returned; if
         * <C>isGroupedByFrames</C> is set, then frames are grouped,
         * otherwise the list is by channels.
         *
         * @param[in] sampleDataIsShown  information, whether each
         *                               sample is returned or only a
         *                               broad description is given
         * @param[in] audioFrameCount    count of sample frames to be
         *                               returned
         * @param[in] isGroupedByFrames  information, whether grouping
         *                               is by frames instead of channels
         * @return string representation of ring buffer vector
         */
        String
        toString (IN Boolean sampleDataIsShown = false,
                  IN Natural audioFrameCount = Natural::maximumValue(),
                  IN Boolean isGroupedByFrames = false) const;

        /*--------------------*/
        /*--------------------*/

        private:

            /** flag to tell whether one or two ring buffers per
             * channel are used in matrix */
            Boolean _hasTwoRingBuffersPerChannel;

            /** the list of sample ring buffers */
            GenericList<AudioSampleRingBuffer> _data;

    };

}

//============================================================

#ifndef DEBUG
    /* production code is inlined */
    #include "AudioSampleRingBufferVector.cpp-inc"
#endif
