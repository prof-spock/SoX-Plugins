/**
 * @file
 * The <C>SoXGain_AudioProcessor</C> module provides boilerplate
 * code for a plugins for the SoX <B>gain</B> effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-07
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioProcessor.h"
#include "SoXGain_AudioEffect.h"
#include "Logging.h"

/*--------------------*/

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
            _setAssociatedEffect(new SoXGain_AudioEffect{});
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXGain");
        }

    };

}

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
