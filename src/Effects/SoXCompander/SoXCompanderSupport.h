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
#include "AudioDataPointRingBuffer.h"

/*--------------------*/

using Audio::AudioDataPointRingBuffer;
using BaseTypes::Primitives::Object;
using BaseTypes::Primitives::Real;
using BaseTypes::GenericTypes::GenericMatrix;

/*====================*/

namespace SoXPlugins::Effects::SoXCompander {

    /**
     * A <C>SoXMultibandCompander</C> object is a complete multiband
     * compander with several compander bands separated by fifth-order
     * Linkwitz-Riley crossover filters.
     */
    struct SoXMultibandCompander {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Sets up a multiband compander.
         */
        SoXMultibandCompander ();

        /*--------------------*/

        /**
         * Constructs new compander from <C>otherCompander</C>
         * (NOT AVAILABLE!)
         *
         * @param[in] otherCompander  compander to be copied
         */
        SoXMultibandCompander
            (IN SoXMultibandCompander& otherCompander) = delete;

        /*--------------------*/

        /**
         * Destroys multiband compander.
         */
        ~SoXMultibandCompander ();

        /*--------------------*/
        /* assignment         */
        /*--------------------*/

        /**
         * Assigns current compander from <C>otherCompander</C>
         * (NOT AVAILABLE!)
         *
         * @param[in] otherCompander  compander to be assigned
         */
        SoXMultibandCompander& operator=
            (IN SoXMultibandCompander& otherCompander) = delete;

        /*--------------------*/
        /* property change    */
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
         * <C>dataPointRate</C>, <C>attack</C>, <C>release</C>,
         * <C>dBKnee</C>, <C>dBThreshold</C>, <C>ratio</C> and
         * <C>dBGain</C>; <C>topFrequency</C> gives the crossover
         * frequency for this band.
         *
         * @param[in] bandIndex      the index of the band to be
         *                           adapted
         * @param[in] dataPointRate  the new data point rate
         * @param[in] attack         the attack time for this band
         * @param[in] release        the release time for this band
         * @param[in] dBKnee         the knee in decibels for this band
         * @param[in] dBThreshold    the threshold setting in decibels
         *                           for this band
         * @param[in] ratio          the compression ration for this
         *                           band
         * @param[in] dBGain         the out gain in decibels for this
         *                           band
         * @param[in] topFrequency   the top crossover frequency for
         *                           this band
         */
        void setCompanderBandData (IN Natural bandIndex,
                                   IN Real dataPointRate,
                                   IN Real attack,
                                   IN Real release,
                                   IN Real dBKnee,
                                   IN Real dBThreshold,
                                   IN Real ratio,
                                   IN Real dBGain,
                                   IN Real topFrequency);

        /*--------------------*/
        /* complex change     */
        /*--------------------*/

        /**
         * Processes data points from <C>inputDataPointList</C> by
         * multiband compander and returns output in
         * <C>outputDataPointList</C>; updates internal buffers for
         * low and high output of each band and sums up to output.
         *
         * @param[in]  inputDataPointList   the queue of data points
         *                                  for all input channels
         * @param[out] outputDataPointList  the queue of data points
         *                                  for all output channels
         */
        void apply (IN AudioDataPointList& inputDataPointList,
                    OUT AudioDataPointList& outputDataPointList);

        /*--------------------*/
        /* type conversion    */
        /*--------------------*/

        /**
         * Return string representation of multiband compander.
         *
         * @return  string representation
         */
        String toString () const;

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
             * the vector of data point ring buffers shared between
             * the bands
             *
             * each channel contains (2 * bandCount + 1) data point
             * ring buffers where each band covers a window of three
             * entries overlapping in one entry
             */
            GenericMatrix<AudioDataPointRingBuffer>
                _dataPointRingBufferVector;

    };

}
