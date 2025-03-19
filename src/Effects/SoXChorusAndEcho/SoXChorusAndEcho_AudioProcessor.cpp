/**
 * @file
 * The <C>SoXChorusAndEcho_AudioProcessor</C> module provides
 * boilerplate code for a plugin for the combined SoX <B>chorus</B>,
 * <B>echo</B> and <B>echos</B> effect; the difference between both
 * echos is that <B>echo</B> has parallel taps all fed by the input
 * signal while <B>echos</B> has sequential taps fed by previous taps.
 *
 * @author Dr. Thomas Tensi
 * @date   2024-12
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "Logging.h"
#include "SoXAudioProcessor.h"
#include "SoXChorusAndEcho_AudioEffect.h"

/*--------------------*/

using SoXPlugins::ViewAndController::SoXAudioProcessor;

/*====================*/

namespace SoXPlugins::Effects::SoXChorusAndEcho {

    /**
     * A <C>SoXChorusAndEcho_AudioProcessor</C> is the JUCE wrapper
     * for the SoX <B>chorus</B>, <B>echo</B> and <B>echos</B> audio
     * effects; the difference between both echos is that <B>echo</B>
     * has parallel taps all fed by the input signal while
     * <B>echos</B> has sequential taps fed by previous taps.
     */
    struct SoXChorusAndEcho_AudioProcessor : public SoXAudioProcessor {

        SoXChorusAndEcho_AudioProcessor ()
        {
            Logging_initializeWithDefaults("SoXChorusAndEcho", "SoXPlugins.");
            _setAssociatedEffect(new SoXChorusAndEcho_AudioEffect{});
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXChorusAndEcho");
        }

    };

}

/*--------------------*/

/**
 * Provides a callback for JUCE to create a SoXChorusAndEcho audio
 * processor.
 *
 * @return JUCE audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter ()
{
    return new SoXPlugins::Effects::SoXChorusAndEcho
                   ::SoXChorusAndEcho_AudioProcessor();
}
