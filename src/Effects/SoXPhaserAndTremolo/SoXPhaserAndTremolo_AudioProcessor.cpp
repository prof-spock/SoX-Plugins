/**
 * @file
 * The <C>SoXPhaserAndTremolo_AudioProcessor</C> module provides
 * boilerplate code for a plugin for the SoX <B>phaser</B> and
 * <B>tremolo</B> effects that have similar parameters and hence are
 * combined into a single plugin.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-09
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioProcessor.h"
#include "SoXPhaserAndTremolo_AudioEffect.h"
#include "Logging.h"

/*--------------------*/

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

}

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
