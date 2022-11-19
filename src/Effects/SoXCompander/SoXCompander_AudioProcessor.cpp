/**
 * @file
 * The <C>SoXCompander_AudioProcessor</C> module provides boilerplate
 * code for a plugin for the combined SoX <B>compand</B> and
 * <B>mcompand</B> effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioProcessor.h"
#include "SoXCompander_AudioEffect.h"
#include "Logging.h"

/*--------------------*/

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
            _setAssociatedEffect(new SoXCompander_AudioEffect{});
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXCompander");
        }

    };

}

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
