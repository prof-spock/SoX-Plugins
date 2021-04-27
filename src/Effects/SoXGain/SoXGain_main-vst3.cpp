/**
 * @file
 * The <C>SoXGain_main-vst3</C> module provides boilerplate code for
 * a VST3 plugin for the SoX <B>gain</B> effect.
 *
 * @author Dr. Thomas Tensi
 */

/*====================*/

#include "SoXGain_juceConfig.h"
#include "SoXAudioProcessor.h"
#include "SoXGain_AudioEffect.h"
#include "Logging.h"
#include "MyString.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::ViewAndController::SoXAudioProcessor;

/*====================*/

namespace SoXPlugins::Effects::SoXGain {

    /**
     * A <C>SoXGain_AudioProcessor</C> is the JUCE wrapper for the SoX
     * <B>gain</B> plugin amplifying or attenuating the input signal
     * by a specific dB amount
     */
    struct SoXGain_AudioProcessor  : public SoXAudioProcessor {

        SoXGain_AudioProcessor ()
        {
            Logging_initializeWithDefaults("SoXGain", "SoXPlugins.");
            _effect = new SoXGain_AudioEffect{};
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXGain");
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
 * Provides a callback for JUCE to create a SoXGain audio processor.
 *
 * @return JUCE audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter ()
{
    return new SoXPlugins::Effects::SoXGain::SoXGain_AudioProcessor();
}

/*--------------------*/

#include <juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp>
