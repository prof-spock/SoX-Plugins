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

#include "Object.h"
#include "AudioSampleListVector.h"
#include "SoXEffectParameterMap.h"
#include "SoXParameterValueChangeKind.h"

/*--------------------*/

using Audio::AudioSampleList;
using Audio::AudioSampleListVector;
using BaseTypes::Primitives::Object;
using BaseTypes::Primitives::String;
using SoXPlugins::Helpers::SoXEffectParameterMap;
using SoXPlugins::Helpers::SoXParameterValueChangeKind;

/*====================*/

namespace SoXPlugins::Effects {

    /** a callback function type for receiving a value change
     * notification */
    typedef void (*ValueChangeNotification)(INOUT Object,
                                            IN String&,
                                            IN String&,
                                            IN Boolean);

    /*====================*/

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
         * Sets parameter named <C>parameterName</C> to <C>value</C>
         * without informing parent processor.  If value has wrong
         * kind, it is ignored; if <C>recalculationIsForced</C> is
         * set, the recalculation of dependent internal settings is
         * forced (otherwise it is suppressed); returns change kind of
         * value to be reported to observers
         *
         * @param[in] parameterName          name of parameter to
         *                                   be set
         * @param[in] value                  associated value of
         *                                   parameter
         * @param[in] recalculationIsForced  flag to tell whether
         *                                   complex recalculations
         *                                   should be done
         * @return  change kind of value (e.g. parameter change)
         */
        SoXParameterValueChangeKind
        setValue (IN String& parameterName,
                  IN String& value,
                  IN Boolean recalculationIsForced = true);

        /*--------------------*/

        /**
         * Sets parameter named <C>parameterName</C> to <C>value</C>
         * by informing parent processor.  If value has wrong kind, it
         * is ignored; if <C>recalculationIsForced</C> is set, the
         * recalculation of dependent internal settings is forced
         * (otherwise it is suppressed)
         *
         * @param[in] parameterName          name of parameter to
         *                                   be set
         * @param[in] value                  associated value of
         *                                   parameter
         * @param[in] recalculationIsForced  flag to tell whether
         *                                   complex recalculations
         *                                   should be done
         */
        void setValueViaParent (IN String& parameterName,
                                IN String& value,
                                IN Boolean recalculationIsForced = true);
        
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
        Boolean hasValidParameters () const;

        /*--------------------*/

        /**
         * Sets parameters collectively to valid or invalid depending
         * on <C>isValid</C>.
         *
         * @param[in] isValid  information whether parameter values
         *                     shall be collectively set to valid
         */
        void setParameterValidity (IN Boolean isValid);

        /*----------------------*/
        /* data change listener */
        /*----------------------*/

        /**
         * Sets handler object and procedure for routing value changes
         * to <C>parent</C> and <C>notificationProc</C>; if handler
         * object is NULL, any value change is just processed locally,
         * otherwise it is assumed that handler calls the local
         * setValue function indirectly
         *
         * @param [inout] valueChangeHandler  object processing the
         *                                    value change
         * @param [inout] notificationProc    callback routine called
         *                                    for some value change
         */
        void setValueChangeHandler (Object valueChangeHandler,
                                    ValueChangeNotification notificationProc);

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
                                   INOUT AudioSampleListVector& buffer);

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
            virtual String _effectDescriptorToString () const = 0;

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
             * @param[in] recalculationIsForced  flag to tell whether
             *                                   complex recalculations
             *                                   should be done
             * @return  value change kind to be reported to observers
             */
            virtual SoXParameterValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value,
                               IN Boolean recalculationIsForced) = 0;

            /*--------------------*/

            /** target object for value change information */
            Object _valueChangeHandler;
        
            /** callback function connecting this effect to value
             * change handler */
            ValueChangeNotification _notificationProc;

            /** the audio sample rate to be used in this effect */
            Real _sampleRate;

            /** the count of channels in this effect */
            Natural _channelCount;

            /** the map of effect parameters */
            SoXEffectParameterMap _effectParameterMap;

            /** during processing: the current time position in
             * seconds */
            Real _currentTimePosition;

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
