/**
 * @file
 * The <C>SoXFilter_AudioProcessor</C> module provides boilerplate
 * code for a plugin for several SoX filter effects combined into one:
 * those filters are <B>allpass</B>, <B>band</B>, <B>bandpass</B>,
 * <B>bandreject</B>, <B>bass</B>, <B>biquad</B>, <B>equalizer</B>,
 * <B>highpass</B>, <B>lowpass</B> and <B>treble</B>.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-11
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioProcessor.h"
#include "SoXFilter_AudioEffect.h"
#include "Logging.h"

/*--------------------*/

using SoXPlugins::ViewAndController::SoXAudioProcessor;

/*====================*/

namespace SoXPlugins::Effects::SoXFilter {

    /**
     * A <C>SoXFilter_AudioProcessor</C> is the JUCE wrapper for
     * several SoX IIR filters: those filters are <B>allpass</B>,
     * <B>band</B>, <B>bandpass</B>, <B>bandreject</B>, <B>bass</B>,
     * <B>biquad</B>, <B>equalizer</B>, <B>highpass</B>,
     * <B>lowpass</B> and <B>treble</B>.
     */
    struct SoXFilter_AudioProcessor : public SoXAudioProcessor {

        SoXFilter_AudioProcessor ()
        {
            Logging_initializeWithDefaults("SoXFilter", "SoXPlugins.");
            _setAssociatedEffect(new SoXFilter_AudioEffect{});
        }

        /*--------------------*/

        String name () const override
        {
            return String("SoXFilter");
        }

    };

}

/*--------------------*/

/**
 * Provides a callback for JUCE to create a SoXFilter audio processor.
 *
 * @return JUCE audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter ()
{
    return new SoXPlugins::Effects::SoXFilter::SoXFilter_AudioProcessor();
}
