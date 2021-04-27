/**
 * @file
 * The <C>SoXPhaserAndTremolo_main-vst3</C> module provides
 * boilerplate code for a VST3 plugin for the SoX <B>phaser</B> and
 * <B>tremolo</B> effects that have similar parameters and hence are
 * combined into a single plugin.
 *
 * @author Dr. Thomas Tensi
 */

/*====================*/

#include "SoXPhaserAndTremolo_juceConfig.h"
#include "SoXAudioProcessor.h"
#include "SoXPhaserAndTremolo_AudioEffect.h"
#include "Logging.h"
#include "MyString.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::ViewAndController::SoXAudioProcessor;

/*====================*/

namespace SoXPlugins::Effects::SoXPhaserAndTremolo {

    /**
     * A <C>SoXPhaserAndTremolo_AudioProcessor</C> object is the JUCE
     * wrapper for the SoX <B>phaser</B> and <B>tremolo</B> effects
     * that have similar parameters and hence are combined into a
     * single plugin.
     */
    struct SoXPhaserAndTremolo_AudioProcessor : public SoXAudioProcessor {

        SoXPhaserAndTremolo_AudioProcessor ()
        {
            Logging_initializeWithDefaults("SoXPhaserAndTremolo",
                                           "SoXPlugins.");
            _effect = new SoXPhaserAndTremolo_AudioEffect{};
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXPhaserAndTremolo");
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
 * Provides a callback for JUCE to create a SoXPhaserAndTremolo audio
 * processor.
 *
 * @return JUCE audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter ()
{
    return new SoXPlugins::Effects::SoXPhaserAndTremolo
               ::SoXPhaserAndTremolo_AudioProcessor();
}

/*--------------------*/

#include <juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp>
