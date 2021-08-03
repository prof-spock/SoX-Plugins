/**
 * @file
 * The <C>SoXAudioSampleQueueMatrix</C> specification defines a
 * two-dimensional list of ring buffers for audio samples.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Natural.h"
#include "MyString.h"
#include "GenericSequence.h"
#include "SoXAudioSampleQueue.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::BaseTypes::Containers::GenericSequence;
using SoXPlugins::CommonAudio::SoXAudioSampleQueue;

/*====================*/

namespace SoXPlugins::CommonAudio {

    /**
     * A <C>SoXAudioSampleQueueMatrix</C> object is a list of ring
     * buffers for audio samples with selection by index and left or
     * right rotation on the individual buffers.
     */
    struct SoXAudioSampleQueueMatrix {

        /**
         * Makes a list of sample queues for <C>channelCount</C>
         * channels with arbitrary length; depending on
         * <C>hasTwoQueuesPerChannel</C> either one or two queues are
         * provided per channel; initial sample queue length is
         * <C>sampleQueueLength</C>
         *
         * @param[in] channelCount            number of channels in
         *                                    matrix
         * @param[in] hasTwoQueuesPerChannel  tells whether each
         *                                    channel requires two
         *                                    queues (instead of one)
         * @param[in] sampleQueueLength       the length of each queue
         *                                    (in samples)
         */
        SoXAudioSampleQueueMatrix (IN Natural channelCount = 0,
                                   IN bool hasTwoQueuesPerChannel = false,
                                   IN Natural sampleQueueLength = 0);

        /*--------------------*/

        /**
         * Returns string representation of sample queue list.
         *
         * @return string representation
         */
        String toString() const;

        /*--------------------*/

        /**
         * Returns count of all queues
         *
         * @return count of queues
         */
        Natural queueCount () const;

        /*--------------------*/

        /**
         * Sets count of all queues to <C>count</C>
         *
         * @param[in] count  the new count of queues
         */
        void setQueueCount (IN Natural count);

        /*--------------------*/

        /**
         * Sets length of all queues to <C>length</C>
         *
         * @param[in] length  the new length of each queue in
         *                    matrix
         */
        void setQueueLength (IN Natural length);

        /*--------------------*/

        /**
         * Removes all sample queues
         */
        void clear ();

        /*--------------------*/

        /**
         * Sets all entries in sample queues to zero, but does not
         * change their effective and maximum length
         */
        void setToZero ();

        /*--------------------*/

        /**
         * Returns reference to sample queue for channel with
         * <C>channelIndex</C> and <C>position</C> (where this may
         * be 0 or 1 when <C>_hasTwoQueuesPerChannel</C> has been
         * set
         *
         * @param[in] channelIndex  number of channel to be selected
         *                          (starting at zero)
         * @param[in] position      if channel has two queues, an
         *                          additional selection parameter
         * @return reference to appropriate queue in matrix
         */
        SoXAudioSampleQueue& at (IN Natural channelIndex,
                                 IN Natural position);

        /*--------------------*/

        /**
         * Returns reference to sample queue for channel with
         * <C>channelIndex</C>; is only allowed when
         * <C>_hasTwoQueuesPerChannel</C> is not set
         *
         * @param[in] channelIndex  number of channel to be
         *                          selected (starting at zero)
         * @return reference to appropriate queue in matrix
         */
        SoXAudioSampleQueue& at (IN Natural channelIndex);

        /*--------------------*/

        /**
         * Returns reference to sample queue for channel with
         * <C>channelIndex</C>; is only allowed when
         * <C>_hasTwoQueuesPerChannel</C> is not set
         *
         * @param[in] channelIndex  number of channel to be
         *                          selected (starting at zero)
         * @return reference to appropriate queue in matrix
         */
        SoXAudioSampleQueue& operator [] (IN Natural channelIndex);

        /*--------------------*/

        /**
         * Appends <C>sampleQueue</C> as last element.
         *
         * @param[in] sampleQueue  queue to be appended as last
         *                         element of matrix
         */
        void append (IN SoXAudioSampleQueue& sampleQueue);

        /*--------------------*/

        /**
         * Returns reference to last sample queue in list.
         *
         * @return last sample queue
         */
        SoXAudioSampleQueue& last ();

        /*--------------------*/
        /*--------------------*/

        private:

            /** flag to tell whether one or two queues per channel
             * are used in matrix */
            bool _hasTwoQueuesPerChannel;

            /** the sequence of sample queues */
            GenericSequence<SoXAudioSampleQueue> _data;

    };

}

//============================================================

#ifndef DEBUG
    //production code is inlined
    #include "SoXAudioSampleQueueMatrix.cpp-inc"
#endif
