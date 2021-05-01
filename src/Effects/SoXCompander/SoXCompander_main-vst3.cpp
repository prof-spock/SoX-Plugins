/**
 * @file
 * The <C>SoXCompander_main-vst3</C> module provides boilerplate code
 * for a VST3 plugin for the combined SoX <B>compand</B> and
 * <B>mcompand</B> effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

/*====================*/

#include "SoXCompander_juceConfig.h"
#include "SoXAudioProcessor.h"
#include "SoXCompander_AudioEffect.h"
#include "Logging.h"
#include "MyString.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::ViewAndController::SoXAudioProcessor;

/*====================*/

namespace SoXPlugins::Effects::SoXCompander {

    /**
     * A <C>SoXCompander_AudioProcessor</C> is the JUCE wrapper for
     * the SoX <B>compand</B> and <B>mcompand</B> audio effects; a
     * simple compander is just seen as a multiband compander with a
     * single band.
     */
    struct SoXCompander_AudioProcessor : public SoXAudioProcessor {

        SoXCompander_AudioProcessor () {
            Logging_initializeWithDefaults("SoXCompander", "SoXPlugins.");
            _effect = new SoXCompander_AudioEffect{};
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXCompander");
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
 * Provides a callback for JUCE to create a SoXCompander audio
 * processor.
 *
 * @return JUCE audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter ()
{
    return new SoXPlugins::Effects::SoXCompander
                   ::SoXCompander_AudioProcessor();
}

/*--------------------*/

#include <juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp>
