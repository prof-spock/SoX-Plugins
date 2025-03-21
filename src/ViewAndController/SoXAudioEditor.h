/**
 * @file
 * The <C>SoXAudioEditor</C> specification defines the audio editor
 * for a plugin, represented by a display window containing the
 * parameters.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "GenericList.h"
#include "SoXAudioEditorWidget.h"
#include "SoXAudioProcessor.h"

/*--------------------*/

using BaseTypes::GenericTypes::GenericList;
using SoXPlugins::ViewAndController::SoXAudioProcessor;
using SoXPlugins::ViewAndController::SoXAudioEditorWidget;

/*====================*/

namespace SoXPlugins::ViewAndController {

    /** a list of audio editor widgets */
    using SoXAudioEditorWidgetPtrList = GenericList<SoXAudioEditorWidget*>;

    /*--------------------*/

    /**
     * A <C>SoXAudioEditor</C> object models the (generic)audio editor
     * for a plugin, represented by a display window containing the
     * parameters in editor widgets.
     */
    struct SoXAudioEditor : public juce::AudioProcessorEditor {

        /**
         * Creates an empty audio editor (without fields)
         * registered as observer for <C>processor</C>.
         *
         * @param[inout] processor  audio processor to be observed and
         *                          changed by this editor
         */
        SoXAudioEditor (INOUT SoXAudioProcessor& processor);

        /*--------------------*/

        ~SoXAudioEditor();

        /*--------------------*/
        /* graphics events    */
        /*--------------------*/

        /**
         * Tells editor to paint contents in <C>graphics</C>.
         *
         * @param graphics  JUCE graphics context to be used by
         *                  editor
         */
        void paint (juce::Graphics& graphics) override;

        /*--------------------*/

        /**
         * Tells editor to resize contents.
         */
        void resized() override;

        /*--------------------*/
        /* parameter mgmt     */
        /*--------------------*/

        /**
         * Returns the current effect parameter map from associated
         * audio processor.
         *
         * @return  map (from parameter name to associated data)
         */
        const SoXEffectParameterMap& effectParameterMap () const;

        /*--------------------*/

        /**
         * Sets parameter named <C>parameterName</C> to
         * <C>value</C> (caused by a change in one of the embedded
         * widgets).  If value has wrong kind, it is ignored.
         *
         * @param[in] parameterName  name of parameter to be set
         * @param[in] value          new value of parameter
         */
        void setValue (IN String& parameterName,
                       IN String& value);

        /*--------------------*/

        /**
         * Callback method to get informed about a change.
         *
         * @param[in] kind  change kind
         * @param[in] data  data depending on change kind
         */
        virtual void notifyAboutChange (IN SoXParameterValueChangeKind kind,
                                        IN String& data="");

        /*--------------------*/
        /*--------------------*/

        protected:

            /** the audio processor displayed by this editor */
            SoXAudioProcessor& _processor;

            /*--------------------*/

            /**
             * Resets the appearance of the editor (upon
             * construction or when some global state has
             * changed).
             */
            virtual void _resetAppearance ();

        /*--------------------*/
        /*--------------------*/

        private:

            /** list of all widgets shown in this sox audio
             * editor */
            SoXAudioEditorWidgetPtrList _widgetList;

            /** the current display page of this editor (starting
             * with 1) */
            Natural _currentEditorPageIndex;

            /** the last display page of this editor */
            Natural _lastEditorPageIndex;

            /** the count of widgets fixed on a tabbed page */
            Natural _commonWidgetCount;

            /** the maximum count of widgets on a tabbed page */
            Natural _maximumWidgetCount;

            /*--------------------*/

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoXAudioEditor)

    };

}
