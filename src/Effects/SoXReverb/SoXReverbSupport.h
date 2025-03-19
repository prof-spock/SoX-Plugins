/**
 * @file
 * The <C>SoXReverbSupport</C> specification defines support classes
 * for the freeverb algorithm and provides a single <B>_SoXReverb</B>
 * class as its interface.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "AudioSampleRingBuffer.h"
#include "Object.h"
#include "Percentage.h"

/*--------------------*/

using Audio::AudioSampleRingBuffer;
using BaseTypes::Primitives::Object;
using BaseTypes::Primitives::Percentage;
using BaseTypes::Primitives::Real;

/*====================*/

namespace SoXPlugins::Effects::SoXReverb {

    /**
     * A <C>_SoXReverb</C> object models the plain Freeverb algorithm.
     */
    struct _SoXReverb {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Constructs a new SoX reverb
         */
        _SoXReverb ();

        /*--------------------*/

        /**
         * Constructs new reverb from <C>otherReverb</C>
         * (NOT AVAILABLE!)
         *
         * @param[in] otherReverb  reverb to be copied
         */
        _SoXReverb (IN _SoXReverb& otherReverb) = delete;

        /*--------------------*/

        /**
         * Destroys SoX reverb
         */
        ~_SoXReverb ();

        /*--------------------*/
        /* assignment         */
        /*--------------------*/

        /**
         * Assigns current reverb from <C>otherReverb</C>
         * (NOT AVAILABLE!)
         *
         * @param[in] otherReverb  reverb to be assigned
         */
        _SoXReverb& operator= (IN _SoXReverb& otherReverb) = delete;

        /*--------------------*/
        /* measurement        */
        /*--------------------*/

        /**
         * Returns the length of the reverb tail in seconds.
         *
         * @return length of reverb tail in seconds
         */
        Real tailLength () const;

        /*--------------------*/
        /* property change    */
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
        void setParameters (IN Boolean isWetOnly,
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
        /* complex change     */
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
        void apply (IN AudioSampleRingBuffer& inputSampleList,
                    OUT AudioSampleRingBuffer& outputSampleList);

        /*--------------------*/
        /* type conversion    */
        /*--------------------*/

        /**
         * Returns a string representation of reverb
         * 
         * @return string representation
         */
        String toString () const;

        /*--------------------*/
        /*--------------------*/

        protected:

            /** the internal parameter data object */
            Object _effectParameterData;

    };

}
