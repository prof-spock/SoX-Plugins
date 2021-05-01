/**
 * @file
 * The <C>SoXReverb_main-vst3</C> module provides boilerplate code for
 * a VST3 plugin for the SoX <B>reverb</B> effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

/*====================*/

#include "SoXReverb_juceConfig.h"
#include "SoXAudioProcessor.h"
#include "SoXReverb_AudioEffect.h"
#include "Logging.h"
#include "MyString.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::ViewAndController::SoXAudioProcessor;

/*====================*/

namespace SoXPlugins::Effects::SoXReverb {

    /**
     * A <C>SoXReverb_AudioProcessor</C> object is a JUCE wrapper for
     * the SoX <B>reverb</B> effect implementing the Freeverb
     * algorithm.
     */
    struct SoXReverb_AudioProcessor  : public SoXAudioProcessor {

        SoXReverb_AudioProcessor ()
        {
            Logging_initializeWithDefaults("SoXReverb", "SoXPlugins.");
            _effect = new SoXReverb_AudioEffect{};
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXReverb");
        }

    };

};

/*====================*/
/* CONNECTION TO JUCE */
/*====================*/

#undef IN
#undef OUT
#undef INOUT

/*--------------------*/

/**
 * Provides a callback for JUCE to create a SoXReverb audio processor.
 *
 * @return JUCE audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter ()
{
    return new SoXPlugins::Effects::SoXReverb::SoXReverb_AudioProcessor();
}

/*--------------------*/

#include <juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp>
