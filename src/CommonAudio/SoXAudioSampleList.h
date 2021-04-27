/**
 * @file
 * The <C>SoXAudioSampleList</C> specification defines sequences of
 * audio samples with zero-based arbitrary indexed access to positions
 * in the sequence.
 */

#pragma once

/*====================*/

#include "GenericSequence.h"
#include "SoXAudioSample.h"
#include "StringUtil.h"

/*====================*/

using SoXPlugins::BaseTypes::Containers::GenericSequence;
using SoXPlugins::CommonAudio::SoXAudioSample;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;

/*====================*/

namespace SoXPlugins::CommonAudio {

    /**
     * An <C>SoXAudioSampleList</C> object is a sequence of audio
     * samples with arbitrary indexed access to positions in the
     * sequence.  Indexing starts at zero and is consecutive.  Lists
     * also allow duplicate elements.
     */
    struct SoXAudioSampleList
        : public GenericSequence<SoXAudioSample, StringUtil::toString>
    {

        /**
         * Returns printable representation of list.
         *
         * @return string representation of list
         */
        String toString () const;

        /*--------------------*/

        /**
         * Returns printable representation of <C>list</C>.
         *
         * @param[in] list  list to be converted to string
         * @return string representation of list
         */
        static String toString (IN SoXAudioSampleList& list);

        /*--------------------*/

        /**
         * Sets all entries in sample list to 0
         */
        void setToZero ();

    };

};
