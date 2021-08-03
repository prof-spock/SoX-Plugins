/**
 * @file
 * The <C>SoXAudioSampleListVector</C> specification defines the
 * communication buffer structure for the block processing of SoX
 * audio effects; it is a list of audio sample lists containing the
 * input samples and the (processed) output samples
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include "GenericSequence.h"
#include "SoXAudioSampleList.h"

/*====================*/

using SoXPlugins::BaseTypes::Containers::GenericSequence;
using SoXPlugins::CommonAudio::SoXAudioSample;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;

/*====================*/

namespace SoXPlugins::CommonAudio {

    /**
     * A <C>SoXAudioSampleListVector</C> object is a buffer of audio
     * sample lists containing the input samples and the (processed)
     * output samples serving as the communication buffer structure
     * for the block processing of SoX audio effects.
     */
    struct SoXAudioSampleListVector
        : public GenericSequence<SoXAudioSampleList,
                                 SoXAudioSampleList::toString> {

        /**
         * Returns printable representation of buffer.
         *
         * @return string representation of buffer
         */
        String toString () const;

        /*--------------------*/

        /**
         * Sets all entries in sample buffer to 0
         */
        void setToZero ();
    };

}

//============================================================

#ifndef DEBUG
    //production code is inlined
    #include "SoXAudioSampleListVector.cpp-inc"
#endif
