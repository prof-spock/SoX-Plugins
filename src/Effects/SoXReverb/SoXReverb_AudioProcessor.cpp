/**
 * @file
 * The <C>SoXReverb_AudioProcessor</C> module provides boilerplate
 * code for a plugin for the SoX <B>reverb</B> effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioProcessor.h"
#include "SoXReverb_AudioEffect.h"
#include "Logging.h"

/*--------------------*/

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

}

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
