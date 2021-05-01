/**
 * @file
 * The <C>SoXAudioEffect</C> specification defines the root class for
 * all SoX audio effects.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include <vector>

#include "GlobalMacros.h"
#include "Natural.h"
#include "Object.h"
#include "Real.h"
#include "SoXAudioParameterMap.h"
#include "SoXAudioSample.h"
#include "SoXAudioSampleBuffer.h"
#include "SoXAudioSampleList.h"
#include "SoXAudioValueChangeKind.h"
#include "String.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::Object;
using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::CommonAudio::SoXAudioParameterMap;
using SoXPlugins::CommonAudio::SoXAudioSampleBuffer;
using SoXPlugins::CommonAudio::SoXAudioSampleList;
using SoXPlugins::CommonAudio::SoXAudioValueChangeKind;

/*====================*/

namespace SoXPlugins::CommonAudio {

    /**
     * A <C>SoXAudioEffect</C> object is a generic stereo audio effect
     * from SoX encapsulating the common services of such an effect.
     */
    struct SoXAudioEffect {

        /**
         * Makes empty audio effect.
         */
        SoXAudioEffect ();

        /*--------------------*/

        /**
         * The destructor of an audio effect.
         */
        virtual ~SoXAudioEffect ();

        /*-----------------------*/
        /* string representation */
        /*-----------------------*/

        /**
         * Returns string representation of audio effect
         *
         * @return string representation
         */
        virtual String toString () const;

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        /**
         * Returns the name of this effect.
         *
         * @return  name of effect
         */
        virtual String name () const;

        /*--------------------*/
        /* parameter map      */
        /*--------------------*/

        /**
         * Returns the mapping from audio parameter name onto
         * parameter data (e.g. the current value).
         *
         * @return  parameter map of audio effect
         */
        SoXAudioParameterMap& audioParameterMap () const;

        /*--------------------*/

        /**
         * Sets parameter named <C>parameterName</C> to <C>value</C>.
         * If value has wrong kind, it is ignored; if
         * <C>recalculationIsSuppressed</C> is set, the recalculation
         * of dependent internal settings is suppressed; returns
         * change kind of value to be reported to observers
         *
         * @param[in] parameterName              name of parameter to
         *                                       be set
         * @param[in] value                      associated value of
         *                                       parameter
         * @param[in] recalculationIsSuppressed  flag to tell whether
         *                                       complex recalculations
         *                                       should be done
         * @return  change kind of value (e.g. parameter change)
         */
        SoXAudioValueChangeKind
        setValue (IN String& parameterName,
                  IN String& value,
                  IN bool recalculationIsSuppressed = false);

        /*--------------------*/

        /**
         * Sets parameters to effect default values.
         */
        virtual void setDefaultValues () = 0;

        /*--------------------*/

        /**
         * Tells whether effect parameters are set.
         *
         * @return validity of parameters
         */
        bool hasValidParameters () const;

        /*--------------------*/

        /**
         * Sets parameters collectively to valid or invalid depending
         * on <C>isValid</C>.
         *
         * @param[in] isValid  information whether parameter values
         *                     shall be collectively set to valid
         */
        void setParameterValidity (IN bool isValid);

        /*--------------------*/
        /* event handling     */
        /*--------------------*/

        /**
         * Informs effect about upcoming processing with
         * <C>sampleRate</C>.
         *
         * @param[in] sampleRate  sample rate to be used for
         *                        processing
         */
        virtual void prepareToPlay (IN Real sampleRate);

        /*--------------------*/

        /**
         * Tells effect to release resources after playback.
         */
        virtual void releaseResources ();

        /*--------------------*/

        /**
         * Processes a block of audio samples <C>buffer</C> for
         * position <C>timePosition</C>.
         *
         * @param[in]    timePosition  position where processing starts
         * @param[inout] buffer        buffer of input and output audio
         *                             samples to be processed
         */
        virtual void processBlock (IN Real timePosition,
                                   INOUT SoXAudioSampleBuffer& buffer);

        /*--------------------*/
        /*--------------------*/

        protected:

            /**
             * Returns data of audio effect as string without surrounding
             * type information
             *
             * @return  string representation of internal data without
             *          surrounding type information
             */
            virtual String _asRawString () const;

            /*--------------------*/

            /**
             * Returns effect descriptor data as string
             *
             * @return  string representation of internal effect 
             *          descriptor
             */
            virtual String _effectDescriptorToString () const;

            /*--------------------*/

            /**
             * Sets parameter named <C>parameterName</C> to
             * <C>value</C>.  If value has wrong kind, it is ignored;
             * if <C>recalculationIsSuppressed</C> is set, the
             * recalculation of dependent internal settings is
             * suppressed; returns kind of value change to be reported
             * to observers
             *
             * @param[in] parameterName              name of parameter to
             *                                       be set
             * @param[in] value                      associated value of
             *                                       parameter
             * @param[in] recalculationIsSuppressed  flag to tell whether
             *                                       complex recalculations
             *                                       should be done
             */
            virtual SoXAudioValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value,
                               IN bool recalculationIsSuppressed) = 0;

            /*--------------------*/

            /** the audio sample rate to be used in this effect */
            Real _sampleRate;

            /** the count of channels in this effect */
            Natural _channelCount;

            /** the map of audio parameters */
            SoXAudioParameterMap _audioParameterMap;

            /** the specific parameters for this effect (private
             * type) */
            Object _effectDescriptor;

            /** during processing: the current time position in
             * seconds */
            Real _currentTimePosition;

            /** during processing: the previous time position in
             * seconds */
            Real _previousTimePosition;

            /** tells whether the time position has moved between
             *  current and previous processing */
            bool _timePositionHasMoved;

            /** tells whether effect parameters have a significant
             * value */
            bool _parametersAreValid;

    };

};
