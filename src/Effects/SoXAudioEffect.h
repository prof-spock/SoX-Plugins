/**
 * @file
 * The <C>SoXAudioEffect</C> specification defines the root class for
 * all SoX audio effects.
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
#include "SoXEffectParameterMap.h"
#include "SoXValueChangeObserver.h"

/*--------------------*/

using Audio::AudioDataPointList;
using Audio::AudioDataPointListVector;
using SoXPlugins::Helpers::SoXEffectParameterMap;
using SoXPlugins::Helpers::SoXValueChangeKind;
using SoXPlugins::Helpers::SoXValueChangeNotification;
using SoXPlugins::Helpers::SoXValueChangeObserver;

/*====================*/

namespace SoXPlugins::Effects {

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
        /* type conversion       */
        /*-----------------------*/

        /**
         * Returns string representation of audio effect
         *
         * @return string representation
         */
        String toString () const;

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        /**
         * Returns the number of channels for this effect
         *
         * @return count of channels of effect
         */
        Natural channelCount () const;

        /*--------------------*/

        /**
         * Returns the data point rate for this effect
         *
         * @return data point rate of effect
         */
        Real dataPointRate () const;

        /*--------------------*/

        /**
         * Tells whether effect parameters are set.
         *
         * @return validity of parameters
         */
        Boolean hasValidParameters () const;

        /*--------------------*/

        /**
         * Tells whether effect settings are dependent on the time
         * position (e.g. when modulators are used).
         *
         * @return  information whether effect settings depend on the
         *          time
         */
        virtual Boolean isTimeVariant () const;

        /*--------------------*/

        /**
         * Returns the name of this effect.
         *
         * @return  name of effect
         */
        virtual String name () const;

        /*--------------------*/

        /**
         * Returns the length of the effect's tail in seconds.
         *
         * @return length of effect tail in seconds
         */
        virtual Real tailLength () const;

        /*--------------------*/
        /* parameter map      */
        /*--------------------*/

        /**
         * Returns the mapping from effect parameter name onto
         * parameter data (e.g. the current value).
         *
         * @return  parameter map of audio effect
         */
        SoXEffectParameterMap& effectParameterMap () const;

        /*--------------------*/

        /**
         * Sets parameters to effect default values.
         */
        virtual void setDefaultValues () = 0;

        /*--------------------*/

        /**
         * Sets parameters collectively to valid or invalid depending
         * on <C>isValid</C>.
         *
         * @param[in] isValid  information whether parameter values
         *                     shall be collectively set to valid
         */
        void setParameterValidity (IN Boolean isValid);

        /*--------------------*/

        /**
         * Sets parameter named <C>parameterName</C> to <C>value</C>.
         * If value has wrong kind, it is ignored; otherwise ensures,
         * that this value is available as value for given parameter
         * name in parameter map
         *
         * @param[in] parameterName  name of parameter to be set
         * @param[in] value          associated value of parameter
         */
        void setValue (IN String& parameterName, IN String& value);

        /*--------------------*/

        /**
         * Sets parameters to values defined by <C>dictionary</C>.
         *
         * @param[in] dictionary  dictionary from parameter names to
         *                        associated string values to be set
         *                        for the parameters
         */
        void setValues (IN Dictionary& dictionary);
        
        /*----------------------*/
        /* data change listener */
        /*----------------------*/

        /**
         * Sets handler object and procedure for routing value changes
         * to <C>parent</C> and <C>notificationProc</C>; if handler
         * object is <C>nullptr</C>, any value change is just
         * processed locally
         *
         * @param [inout] valueChangeHandler  object processing the
         *                                    value change
         * @param [inout] notificationProc    callback routine called
         *                                    for some value change
         */
        void setValueChangeHandler
                 (Object valueChangeHandler,
                  SoXValueChangeNotification notificationProc);

        /*--------------------*/
        /* event handling     */
        /*--------------------*/

        /**
         * Informs effect about upcoming processing with
         * <C>dataPointRate</C>.
         *
         * @param[in] dataPointRate  data point rate to be used for
         *                           processing
         */
        void prepareToPlay (IN Real dataPointRate);

        /*--------------------*/

        /**
         * Processes a block of audio data points <C>buffer</C> for
         * position <C>timePosition</C>; this is a template function
         * relying on <C>_applyForFrame</C> and <C>_internalState</C>.
         *
         * @param[in]    timePosition  position where processing starts
         * @param[inout] buffer        buffer of input and output audio
         *                             data points to be processed
         */
        void processBlock (IN Real timePosition,
                           INOUT AudioDataPointListVector& buffer);

        /*--------------------*/

        /**
         * Recalculates dependent data in effect
         */
        virtual void recalculate ();

        /*--------------------*/

        /**
         * Tells effect to release resources after playback.
         */
        virtual void releaseResources ();

        /*--------------------*/
        /*--------------------*/

        protected:

            /*-----------------------*/
            /* string representation */
            /*-----------------------*/

            /**
             * Returns effect descriptor data as string
             *
             * @return  string representation of internal effect 
             *          descriptor
             */
            virtual String _effectDescriptorToString () const;

            /*-----------------------*/

            /**
             * Returns characteristic state data of effect as a string
             *
             * @return  string representation of internal effect state
             */
            virtual String _internalState () const;

            /*--------------------*/
            /* parameter map      */
            /*--------------------*/

            /**
             * Sets parameter named <C>parameterName</C> to
             * <C>value</C>.  If value has wrong kind, it is ignored;
             * if <C>recalculationIsForced</C> is set, the
             * recalculation of dependent internal settings is forced
             * (otherwise it it suppressed); returns kind of value
             * change to be reported to observers
             *
             * @param[in] parameterName          name of parameter to
             *                                   be set
             * @param[in] value                  associated value of
             *                                   parameter
             * @return  value change kind to be reported to observers
             */
            virtual SoXValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value) = 0;

            /*--------------------*/
            /* event handling     */
            /*--------------------*/

            /**
             * Applies effect for a single frame given by
             * <C>inputDataPointList</C> with results stored in
             * <C>outputDataPointList</C>
             *
             * @param[in]  inputDataPointList   frame input data points
             * @param[out] outputDataPointList  frame output data points
             */
            virtual void
            _applyForFrame (IN AudioDataPointList& inputDataPointList,
                            OUT AudioDataPointList& outputDataPointList) = 0;

            /*--------------------*/

            /**
             * Informs effect about a change in <C>dataPointRate</C>.
             *
             * @param[in] dataPointRate  data point rate to be used for
             *                           processing
             */
            virtual void _handleDataPointRateChange (IN Real dataPointRate);

            /*--------------------*/
            /*--------------------*/

            /** observer object for value change information */
            SoXValueChangeObserver _valueChangeObserver;
        
            /** the audio data point rate to be used in this effect */
            Real _dataPointRate;

            /** the count of channels in this effect */
            Natural _channelCount;

            /** the map of effect parameters */
            SoXEffectParameterMap _effectParameterMap;

            /** during processing: the current time position in
             * seconds */
            Real _currentTimePosition;

            /** during processing: the time difference between two
             * adjacent audio data points */
            Real _deltaTime;

            /** during processing: the expected next time position in
             * seconds */
            Real _expectedNextTimePosition;

            /** tells whether the time position has moved between
             *  current and previous processing */
            Boolean _timePositionHasMoved;

            /** tells whether effect parameters have a significant
             * value */
            Boolean _parametersAreValid;

            /** the specific parameters for this effect (private
             * type) */
            Object _effectDescriptor;

    };

}
