/**
 * @file
 * The <C>SoXFlangerPhaserAndTremolo_AudioProcessor</C> module
 * provides boilerplate code for a plugin for the SoX <B>flanger</B>,
 * <B>phaser</B> and <B>tremolo</B> effects that have similar
 * parameters and hence are combined into a single plugin.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-09
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioProcessor.h"
#include "SoXFlangerPhaserAndTremolo_AudioEffect.h"
#include "Logging.h"

/*--------------------*/

using SoXPlugins::ViewAndController::SoXAudioProcessor;

/*====================*/

namespace SoXPlugins::Effects::SoXFlangerPhaserAndTremolo {

    /**
     * A <C>SoXFlangerPhaserAndTremolo_AudioProcessor</C> object is the JUCE
     * wrapper for the SoX <B>phaser</B> and <B>tremolo</B> effects
     * that have similar parameters and hence are combined into a
     * single plugin.
     */
    struct SoXFlangerPhaserAndTremolo_AudioProcessor
        : public SoXAudioProcessor {

        SoXFlangerPhaserAndTremolo_AudioProcessor ()
        {
            Logging_initializeWithDefaults("SoXFlangerPhaserAndTremolo",
                                           "SoXPlugins.");
            _setAssociatedEffect(new SoXFlangerPhaserAndTremolo_AudioEffect{});
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXFlangerPhaserAndTremolo");
        }

    };

}

/*--------------------*/

/**
 * Provides a callback for JUCE to create a SoXFlangerPhaserAndTremolo audio
 * processor.
 *
 * @return JUCE audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter ()
{
    return new SoXPlugins::Effects::SoXFlangerPhaserAndTremolo
               ::SoXFlangerPhaserAndTremolo_AudioProcessor();
}
