/**
 * @file
 * The <C>SoXReverbSupport</C> specification defines support classes
 * for the freeverb algorithm and provides a single <B>_SoXReverb</B>
 * class as its interface.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Natural.h"
#include "Object.h"
#include "Percentage.h"
#include "Real.h"
#include "MyString.h"
#include "SoXAudioSampleQueue.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::Object;
using SoXPlugins::BaseTypes::Primitives::Percentage;
using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::CommonAudio::SoXAudioSampleQueue;

/*====================*/

namespace SoXPlugins::Effects::SoXReverb {

    /**
     * A <C>_SoXReverb</C> object models the plain Freeverb algorithm.
     */
    struct _SoXReverb {

        /**
         * Constructs a new SoX reverb
         */
        _SoXReverb ();

        /*--------------------*/

        /**
         * Destroys SoX reverb
         */
        ~_SoXReverb ();

        /*--------------------*/

        /**
         * Returns a string representation of reverb
         */
        String toString () const;

        /*--------------------*/

        /**
         * Initializes reverb with <C>isWetOnly</C>,
         * <C>reverberance</C>, <C>hfDamping</C>, <C>roomScale</C>,
         * <C>stereoDepth</C>, <C>predelay</C> and <C>wetDbGain</C>.
         *
         * @param[in] isWetOnly     information whether direct signal
         *                          should be suppressed in output
         * @param[in] reverberance  the reverberance in percent
         * @param[in] hfDamping     the HF damping in percent
         * @param[in] roomScale     the room scale (room size)
         *                          in percent
         * @param[in] stereoDepth   the stereo depth in percent
         * @param[in] predelay      the pre-delay in milliseconds
         * @param[in] wetDbGain     the gain of the wet signal in
         *                          decibels
         */
        void setParameters (IN bool isWetOnly,
                            IN Percentage reverberance,
                            IN Percentage hfDamping,
                            IN Percentage roomScale,
                            IN Percentage stereoDepth,
                            IN Real predelay,
                            IN Real wetDbGain);

        /*--------------------*/

        /**
         * Sets number of reverb channels to <C>channelCount</C> and the
         * sample rate to <C>sampleRate</C>.
         *
         * @param[in] sampleRate    the new sample rate for this effect
         * @param[in] channelCount  the new channel count for this effect
         */
        void resize (IN Real sampleRate,
                     IN Natural channelCount);

        /*--------------------*/

        /**
         * Applies this reverb to samples for all channels in
         * <C>inputSampleList</C> and writes result in
         * <C>outputSampleList</C>.
         *
         * @param[in]  inputSampleList   the queue of samples for all
         *                               input channels
         * @param[out] outputSampleList  the queue of samples for all
         *                               output channels
         */
        void apply (IN SoXAudioSampleQueue& inputSampleList,
                    OUT SoXAudioSampleQueue& outputSampleList);

        /*--------------------*/
        /*--------------------*/

        protected:

            /** the internal parameter data object */
            Object _effectParameterData;

    };

};
