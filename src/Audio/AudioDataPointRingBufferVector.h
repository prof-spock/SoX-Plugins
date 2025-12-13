/**
 * @file
 * The <C>AudioDataPointRingBufferVector</C> specification defines a
 * two-dimensional list of ring buffers for audio data points.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "AudioDataPointListVector.h"
#include "AudioDataPointRingBuffer.h"

/*====================*/

using Audio::AudioDataPointListVector;
using Audio::AudioDataPointRingBuffer;

/*====================*/

namespace Audio {

    /**
     * A <C>AudioDataPointRingBufferVector</C> object is a list of
     * ring buffers for audio data points with selection by index and
     * left or right rotation on the individual buffers.
     */
    struct AudioDataPointRingBufferVector {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Makes a list of data point ring buffers for
         * <C>channelCount</C> channels with arbitrary length;
         * depending on <C>hasTwoRingBuffersPerChannel</C> either one
         * or two ring buffers are provided per channel; initial data
         * point ring buffer length is
         * <C>dataPointRingBufferLength</C>
         *
         * @param[in] channelCount                 number of channels
         *                                         in vector
         * @param[in] hasTwoRingBuffersPerChannel  tells whether each
         *                                         channel requires two
         *                                         ring buffers (instead
         *                                         of one)
         * @param[in] dataPointRingBufferLength       the length of each
         *                                         ring buffer (in data
         *                                         points)
         */
        AudioDataPointRingBufferVector
            (IN Natural channelCount = 0,
             IN Boolean hasTwoRingBuffersPerChannel = false,
             IN Natural dataPointRingBufferLength = 0);

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
         * Removes all data point ring buffers
         */
        void clear ();

        /*--------------------*/

        /**
         * Sets all entries in data point ring buffers to zero, but
         * does not change their effective and maximum length
         */
        void setToZero ();

        /*--------------------*/

        /**
         * Returns reference to data point ring buffer for channel
         * with <C>channelIndex</C> and <C>position</C> (where this
         * may be 0 or 1 when <C>_hasTwoRingBuffersPerChannel</C> has
         * been set
         *
         * @param[in] channelIndex  number of channel to be selected
         *                          (starting at zero)
         * @param[in] position      if channel has two ring buffers, an
         *                          additional selection parameter
         * @return reference to appropriate ring buffer in vector
         */
        const AudioDataPointRingBuffer& at (IN Natural channelIndex,
                                         IN Natural position = 0) const;

        /*--------------------*/

        /**
         * Returns reference to data point ring buffer for channel
         * with <C>channelIndex</C> and <C>position</C> (where this
         * may be 0 or 1 when <C>_hasTwoRingBuffersPerChannel</C> has
         * been set
         *
         * @param[in] channelIndex  number of channel to be selected
         *                          (starting at zero)
         * @param[in] position      if channel has two ring buffers, an
         *                          additional selection parameter
         * @return reference to appropriate ring buffer in vector
         */
        AudioDataPointRingBuffer& at (IN Natural channelIndex,
                                   IN Natural position = 0);

        /*--------------------*/

        /**
         * Returns reference to data point ring buffer for channel
         * with <C>channelIndex</C>; is only allowed when
         * <C>_hasTwoRingBuffersPerChannel</C> is not set
         *
         * @param[in] channelIndex  number of channel to be
         *                          selected (starting at zero)
         * @return reference to appropriate ring buffer in vector
         */
        AudioDataPointRingBuffer& operator [] (IN Natural channelIndex);

        /*--------------------*/

        /**
         * Appends <C>dataPointRingBuffer</C> as last element.
         *
         * @param[in] dataPointRingBuffer  ring buffer to be appended
         *                                 as last element of matrix
         */
        void append (IN AudioDataPointRingBuffer& dataPointRingBuffer);

        /*--------------------*/

        /**
         * Returns reference to last data point ring buffer in list.
         *
         * @return last data point ring buffer
         */
        AudioDataPointRingBuffer& last ();

        /*--------------------*/
        /* conversion         */
        /*--------------------*/

        /**
         * Returns string representation of data point ring buffer
         * list;; if <C>audioFrameCount</C> is set, only that number
         * of audio frames with data point data will be returned; if
         * <C>isGroupedByFrames</C> is set, then frames are grouped,
         * otherwise the list is by channels.
         *
         * @param[in] dataPointDataIsShown  information, whether each
         *                                  data point is returned or
         *                                  only a broad description
         *                                  is given
         * @param[in] audioFrameCount       count of data point frames
         *                                  to be returned
         * @param[in] isGroupedByFrames     information, whether grouping
         *                                  is by frames instead of
         *                                  channels
         * @return string representation of ring buffer vector
         */
        String
        toString (IN Boolean dataPointDataIsShown = false,
                  IN Natural audioFrameCount = Natural::maximumValue(),
                  IN Boolean isGroupedByFrames = false) const;

        /*--------------------*/
        /*--------------------*/

        private:

            /** flag to tell whether one or two ring buffers per
             * channel are used in matrix */
            Boolean _hasTwoRingBuffersPerChannel;

            /** the list of data point ring buffers */
            GenericList<AudioDataPointRingBuffer> _data;

    };

}

//============================================================

#ifndef DEBUG
    /* production code is inlined */
    #include "AudioDataPointRingBufferVector.cpp-inc"
#endif
