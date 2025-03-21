/**
 * @file
 * The <C>SoXAudioProcessor</C> specification defines an audio effect
 * wrapper for a plugin responsible for the communication to the
 * enclosing plugin host.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include <set>
#include "JuceHeaders.h"

#include "SoXAudioEffect.h"

/*--------------------*/

using std::set;
using SoXPlugins::Effects::SoXAudioEffect;

/*====================*/

namespace SoXPlugins::ViewAndController {

    /**
     * A <C>SoXAudioProcessor</C> object provides an audio effect
     * wrapper for a plugin responsible for the communication to the
     * enclosing plugin host.
     */
    struct SoXAudioProcessor : public juce::AudioProcessor {

        /**
         * Creates an audio processor to be associated with an audio
         * effect
         */
        SoXAudioProcessor ();

        /*--------------------*/

        ~SoXAudioProcessor ();

        /*--------------------*/

        /**
         * Creates an editor for this processor and returns
         * resulting editor.
         *
         * @return new JUCE editor
         */
        juce::AudioProcessorEditor* createEditor () override;

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        /**
         * Tells whether this processor has an editor.
         *
         * @return  information whether this processor has an
         *          editor (true)
         */
        bool hasEditor () const override;

        /*--------------------*/

        /**
         * Tells whether this processor can handle double sample
         * values.
         *
         * @return  information whether this processor can handle
         *          double sample values (false)
         */
        bool supportsDoublePrecisionProcessing () const override;

        /*--------------------*/

        /**
         * Tells whether this processor accepts MIDI data.
         *
         * @return  information whether this processor accepts MIDI
         *          data (false)
         */
        bool acceptsMidi () const override;

        /*--------------------*/

        /**
         * Tells whether this processor produces MIDI data.
         *
         * @return  information whether this processor produces MIDI
         *          data (false)
         */
        bool producesMidi () const override;

        /*--------------------*/

        /**
         * Tells whether this processor is a MIDI effect.
         *
         * @return  information whether this processor is a MIDI
         *          effect (false)
         */
        bool isMidiEffect () const override;

        /*--------------------*/

        /**
         * Returns the tail length of this effect.
         *
         * @return  the length in seconds of a tail of this effect
         */
        double getTailLengthSeconds () const override;

        /*--------------------*/

        /**
         * Returns the number of programs this effect supports
         *
         * @return  the number of programs of this effect
         */
        int getNumPrograms () override;

        /*--------------------*/

        /**
         * Returns the current program of this effect
         *
         * @return  the current program of this effect
         */
        int getCurrentProgram () override;

        /*--------------------*/

        /**
         * Sets the current program of this effect by an index.
         *
         * @param[in] index  the index of the current program to
         *                   be set
         */
        void setCurrentProgram (int index) override;

        /*--------------------*/

        /**
         * Returns the program name for this processor given by
         * <C>index</C>.
         *
         * @param[in] index  the index of the program of this
         *                   processor
         * @return name of program
         */
        const juce::String getProgramName (int index) override;

        /*--------------------*/

        /**
         * Changes the program name for this processor given by
         * <C>index</C>.
         *
         * @param[in] index    the index of the program of this
         *                     processor
         * @param[in] newName  new name of program
         */
        void changeProgramName (int index,
                                const juce::String& newName)
            override;

        /*--------------------*/

        /**
         * Gets name of this plugin for JUCE.
         *
         * @return  name of processor as a JUCE string
         */
        const juce::String getName () const override;

        /*--------------------*/

        /**
         * Gets name of this plugin.
         *
         * @return string name of plugin
         */
        virtual String name () const;

        /*--------------------*/
        /* parameter map      */
        /*--------------------*/

        /**
         * Returns the mapping from effect parameter name onto
         * currently associated value.
         *
         * @return  map (from parameter name to associated data)
         */
        SoXEffectParameterMap& effectParameterMap () const;

        /*--------------------*/

        /**
         * Sets parameter named <C>parameterName</C> to <C>value</C>
         * and notifies host.  If value has wrong kind, it is ignored;
         * if <C>recalculationIsForced</C> is set, the recalculation
         * of dependent internal settings is forced (otherwise it is
         * suppressed).
         *
         * @param[in] parameterName              name of parameter to
         *                                       be set
         * @param[in] value                      new value of parameter
         * @param[in] recalculationIsForced  tells whether some
         *                                       internal recalculation
         *                                       of associated effect
         *                                       must be done
         */
        virtual void setValue (IN String& parameterName,
                               IN String& value,
                               IN Boolean recalculationIsForced = true);

        /*--------------------*/

        /**
         * Sets all parameters for audio processor to values
         * defined by <C>dictionary</C>.
         *
         * @param[in] dictionary  dictionary from parameter names to
         *                        associated string values to be set
         *                        for the parameters
         */
        void setValues (IN Dictionary& dictionary);

        /*--------------------*/
        /* observer mgmt      */
        /*--------------------*/

        /**
         * Registers <C>observer</C> as object to be notified
         * about parameter change; nothing will happen when
         * observer is already registered
         *
         * @param[in] observer  editor to be added as observer for
         *                      this processor
         */
        void registerObserver (INOUT Object observer);

        /*--------------------*/

        /**
         * Deletes registration of <C>observer</C> as object to be
         * notified about parameter change; nothing will happen
         * when observer is not registered
         *
         * @param[in] observer  editor to be removed as observer for
         *                      this processor
         */
        void unregisterObserver (INOUT Object observer);

        /*--------------------*/
        /* event handling     */
        /*--------------------*/

        /**
         * Informs the processor to be prepared for playback.
         *
         * @param[in] sampleRate                      the sample rate
         *                                            to be used for
         *                                            playback
         * @param[in] maximumExpectedSamplesPerBlock  the number of
         *                                            samples per
         *                                            processing call
         */
        void prepareToPlay (double sampleRate,
                            int maximumExpectedSamplesPerBlock)
            override;

        /*--------------------*/

        /**
         * Tells processor to release resources after playback.
         */
        void releaseResources () override;

        /*--------------------*/

        /**
         * Processes a block of float samples with this audio
         * processor.
         *
         * @param[inout] buffer     combination of input and output
         *                          sample lists in float format
         * @param[in] midiMessages  list of midi messages to be
         *                          processed (here ignored)
         */
        void processBlock (juce::AudioBuffer<float>& buffer,
                           juce::MidiBuffer& midiMessages)
            override;

        /*--------------------*/
        /* persistence        */
        /*--------------------*/

        /**
         * Gets data from processor and stores it in serialized
         * form in <C>destData</C>.
         *
         * @param[out] destData  JUCE memory block to be adapted
         *                       with serialized form
         */
        void getStateInformation (juce::MemoryBlock& destData)
            override;

        /*--------------------*/

        /**
         * Sets data for processor from serialized form in
         * <C>data</C> with length <C>sizeInBytes</C>.
         *
         * @param[in] data         byte list with serialized form
         *                         for processor
         * @param[in] sizeInBytes  length of serialized data
         */
        void setStateInformation (const void* data, int sizeInBytes)
            override;

        /*--------------------*/
        /*--------------------*/

        protected:

            /** the object containing the internal data of
                the audio processor */
            Object _descriptor;

            /*--------------------*/

            /**
             * Tells all observers about change of kind <C>kind</C>
             * using (optional) <C>data</C>.
             *
             * @param[in] kind  kind of change to be reported
             * @param[in] data  associated data for change
             */
            void _notifyObserversAboutChange
                     (IN SoXParameterValueChangeKind kind,
                      IN String& data = "");

            /*--------------------*/

            /**
             * Sets associated audio effect to <C>effect</C>, collects
             * all effect parameters from it and creates corresponding
             * audio parameter objects for communication with host.
             *
             * @param[in] effect  associated audio effect for this
             *                    processor
             */
            void _setAssociatedEffect (IN SoXAudioEffect* effect);

        private:

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoXAudioProcessor)

    };

}
