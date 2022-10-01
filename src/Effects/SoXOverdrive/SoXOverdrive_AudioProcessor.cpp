/**
 * @file
 * The <C>SoXOverdrive_AudioProcessor</C> module provides boilerplate
 * code for a plugin for the SoX <B>overdrive</B> effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioProcessor.h"
#include "SoXOverdrive_AudioEffect.h"
#include "Logging.h"

/*--------------------*/

using SoXPlugins::ViewAndController::SoXAudioProcessor;

/*====================*/

namespace SoXPlugins::Effects::SoXOverdrive {

    /**
     * A <C>SoXOverdrive_AudioProcessor</C> is the JUCE wrapper for
     * the SoX <B>overdrive</B> plugin distorting the input signal by
     * a gain amount with tanh distortion and adapting the colour by
     * asymmetric shaping
     */
    struct SoXOverdrive_AudioProcessor : public SoXAudioProcessor {

        SoXOverdrive_AudioProcessor ()
        {
            Logging_initializeWithDefaults("SoXOverdrive", "SoXPlugins.");
            _effect = new SoXOverdrive_AudioEffect{};
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXOverdrive");
        }

    };

}

/*--------------------*/

/**
 * Provides a callback for JUCE to create a SoXOverdrive audio processor.
 *
 * @return JUCE audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter ()
{
    return new SoXPlugins::Effects::SoXOverdrive
               ::SoXOverdrive_AudioProcessor();
}
