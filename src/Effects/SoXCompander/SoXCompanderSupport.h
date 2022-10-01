/**
 * @file
 * The <C>SoXCompanderSupport</C> specification defines support
 * classes for the compander algorithm and provides a single
 * <B>SoXMultibandCompander</B> class as its interface.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "GenericMatrix.h"
#include "Object.h"
#include "Real.h"
#include "AudioSampleRingBuffer.h"

/*--------------------*/

using Audio::AudioSampleRingBuffer;
using BaseTypes::Primitives::Object;
using BaseTypes::Primitives::Real;
using BaseTypes::Containers::GenericMatrix;

/*====================*/

namespace SoXPlugins::Effects::SoXCompander {

    /**
     * A <C>SoXMultibandCompander</C> object is a complete multiband
     * compander with several compander bands separated by fifth-order
     * Linkwitz-Riley crossover filters.
     */
    struct SoXMultibandCompander {

        /**
         * Sets up a multiband compander.
         */
        SoXMultibandCompander ();

        /*--------------------*/

        ~SoXMultibandCompander ();

        /*--------------------*/

        /**
         * Return string representation of multiband compander.
         *
         * @return  string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Resizes multiband compander to <C>bandCount</C> bands and
         * <C>channelCount</C> channels.
         *
         * @param[in] bandCount     the new band count for this effect
         * @param[in] channelCount  the new channel count for this effect
         */
        void resize (IN Natural bandCount,
                     IN Natural channelCount);

        /*--------------------*/

        /**
         * Sets effective number of bands in multiband compander to
         * <C>bandCount</C>.
         *
         * @param[in] bandCount  the new band count for this effect
         */
        void setEffectiveSize (IN Natural bandCount);

        /*--------------------*/

        /**
         * Sets band compander characteristics in multiband compander
         * for band with <C>bandIndex</C> to data given by
         * <C>sampleRate</C>, <C>attack</C>, <C>release</C>,
         * <C>dBKnee</C>, <C>dBThreshold</C>, <C>ratio</C> and
         * <C>dBGain</C>; <C>topFrequency</C> gives the crossover
         * frequency for this band.
         *
         * @param[in] bandIndex     the index of the band to be adapted
         * @param[in] sampleRate    the new sample rate
         * @param[in] attack        the attack time for this band
         * @param[in] release       the release time for this band
         * @param[in] dBKnee        the knee in decibels for this band
         * @param[in] dBThreshold   the threshold setting in decibels
         *                          for this band
         * @param[in] ratio         the compression ration for this band
         * @param[in] dBGain        the out gain in decibels for this band
         * @param[in] topFrequency  the top crossover frequency for this
         *                          band
         */
        void setCompanderBandData (IN Natural bandIndex,
                                   IN Real sampleRate,
                                   IN Real attack,
                                   IN Real release,
                                   IN Real dBKnee,
                                   IN Real dBThreshold,
                                   IN Real ratio,
                                   IN Real dBGain,
                                   IN Real topFrequency);

        /*--------------------*/

        /**
         * Processes samples from <C>inputSampleList</C> by multiband
         * compander and returns output in <C>outputSampleList</C>;
         * updates internal buffers for low and high output of each
         * band and sums up to output.
         *
         * @param[in]  inputSampleList   the queue of samples for all
         *                               input channels
         * @param[out] outputSampleList  the queue of samples for all
         *                               output channels
         */
        void apply (IN AudioSampleList& inputSampleList,
                    OUT AudioSampleList& outputSampleList);

        /*--------------------*/
        /*--------------------*/

        private:

            /** the allocated number of bands in this multiband
             * compander */
            Natural _allocatedBandCount;

            /** the effective number of bands in this multiband
             * compander */
            Natural _bandCount;

            /** the number of channels in this multiband compander */
            Natural _channelCount;

            /** the list of compander bands in this multiband
             * compander */
            Object _companderBandList;

            /**
             * the vector of sample ring buffers shared between the
             * bands
             *
             * each channel contains (2 * bandCount + 1) sample ring
             * buffers where each band covers a window of three
             * entries overlapping in one entry
             */
            GenericMatrix<AudioSampleRingBuffer> _sampleRingBufferVector;

    };

}
