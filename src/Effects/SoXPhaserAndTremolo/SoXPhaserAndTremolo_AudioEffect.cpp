/**
 * @file
 * The <C>SoXPhaserAndTremolo_AudioEffect</C> body implements the SoX
 * <B>phaser</B> and <B>tremolo</B> modulation effects that have
 * similar parameters and hence are combined into a single plugin.
 *
 * The original algorithms can be found in the <A
 * HREF="https://sourceforge.net/p/sox/code">SoX repository</A>
 * in the files
 *   - "phaser.c" by Juergen Mueller and Sundry Contributors with the
 *     code for the phaser effect
 *   - "synth.c" by Carsten Borchardt and Rob Sowell with the
 *     code for the synth base effect, and
 *   - "tremolo.c" by Rob Sewell with the code for the (wrapper) tremolo
 *     effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-09
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include <cmath>
#include <cstdio>

#include "Logging.h"
#include "Percentage.h"
#include "AudioSampleRingBufferVector.h"
#include "WaveForm.h"
#include "SoXPhaserAndTremolo_AudioEffect.h"

/*--------------------*/

using Audio::AudioSampleRingBufferVector;
using Audio::WaveForm;
using Audio::WaveFormKind;
using Audio::WaveFormIteratorState;
using BaseTypes::Primitives::Percentage;
using SoXPlugins::Effects::SoXPhaserAndTremolo
      ::SoXPhaserAndTremolo_AudioEffect;

/*============================================================*/

namespace SoXPlugins::Effects::SoXPhaserAndTremolo {

    /** the maximum allowable delay in seconds */
    static const Real _maximumDelay = 0.005;

    /** the internal separator for lists */
    static const String separator = "/";

    /** the standard starting phase of a phaser (90°) */
    static const Radians _defaultPhase = Real::pi / 2.0;

    /** the name of the tremolo effect */
    static const String _tremoloEffectKind = "Tremolo";

    /** the list of effect kinds in this plugin */
    static const StringList _kindList =
        StringList::makeBySplit("Phaser/Tremolo", separator);

    /*....................*/
    /* parameter names    */
    /*....................*/

    /** the name for the decay parameter */
    static const String parameterName_decay        = "Decay";

    /** the name for the delay parameter */
    static const String parameterName_delayInMs    = "Delay [ms]";

    /** the name for the depth parameter */
    static const String parameterName_depth        = "Depth [%]";

    /** the name for the effect kind parameter */
    static const String parameterName_effectKind   = "Effect Kind";

    /** the name for the frequency parameter */
    static const String parameterName_frequency    = "Modulation [Hz]";

    /** the name for the input gain parameter */
    static const String parameterName_inGain       = "In Gain [dB]";

    /** the name for the output gain parameter */
    static const String parameterName_outGain      = "Out Gain [dB]";

    /** the name for the time offset parameter */
    static const String parameterName_timeOffset   = "Time Offset [s]";

    /** the name for the waveform kind parameter */
    static const String parameterName_waveFormKind = "Waveform";

    /*--------------------*/
    /* internal routines  */
    /*--------------------*/

    /**
     * An <C>_EffectDescriptor_PHTR</C> object is the internal
     * implementation of a effect descriptor type for phaser and
     * tremolo; both effects share parameters and have similar
     * implementations: hence they can be combined.
     *
     * For the tremolo the phaser parameters are assumed as follows: delay
     * is 0s, inGain and outGain are 1.0, the decay is not relevant.
     *
     * For the phaser the tremolo parameter depth is not relevant and
     * ignored.
     */
    struct _EffectDescriptor_PHTR {

        /** information whether effect is a phaser or tremolo */
        Boolean isPhaser;

        /** the modulation frequency in Hz */
        Real frequency;

        /** the modulation waveform kind */
        WaveFormKind waveFormKind;

        /** the waveform data */
        WaveForm waveForm;

        /** the time offset of this effect in seconds */
        Real timeOffset;

        /** the input gain as a factor */
        Real inGain;

        /** the output gain as a factor */
        Real outGain;

        /** the delay in seconds */
        Real delay;

        /** the decay in seconds */
        Real decay;

        /** the modulation depth in percent */
        Percentage depth;

        /** the sample ring buffer for the delay line */
        AudioSampleRingBufferVector delayRingBufferList;

        /** the delay buffer length in samples */
        Natural delayRingBufferLength;

        /** the pointer to the delay buffer (as an index) */
        Natural delayRingBufferIndex;
    };

    /*====================*/

    /*--------------------*/
    /* internal routines  */
    /*--------------------*/

    /**
     * Sets up a new phaser/tremolo effect descriptor based on
     * <C>sampleRate</C>
     *
     * @param[in] sampleRate  the sample rate for this effect
     * @return  new phaser/tremolo effect descriptor
     */
    static
    _EffectDescriptor_PHTR* _createEffectDescriptor (IN Real sampleRate)
    {
        const Natural maximumDelayBufferLength =
            Natural::ceiling((double)(_maximumDelay * sampleRate));

        _EffectDescriptor_PHTR* result =
            new _EffectDescriptor_PHTR{
                true,                                  // phaser

                0.5,                                   // frequency
                WaveFormKind::triangle,             // waveFormKind
                {},                                    // waveForm
                0.0,                                   // timeOffset

                0.4,                                   // inGain
                0.74,                                  // outGain
                0.003,                                 // delay
                0.4,                                   // decay
                40.0,                                  // depth

                {2, false, maximumDelayBufferLength},  // delayRingBufferList
                maximumDelayBufferLength,              // delayRingBufferLength
                0                                      // delayRingBufferIndex
            };

        return result;
    }

    /*--------------------*/

    /**
     * Returns <C>descriptor</C> string representation.
     *
     * @param[in] effectDescriptor  descriptor of effect
     * @return  string representation of effect
     */
    static String
    _effectDescriptorPHTRToString (INOUT _EffectDescriptor_PHTR*
                                             effectDescriptor)
    {
        String st = "_EffectDescriptor_PHTR(";
        st += "isPhaser = " + TOSTRING(effectDescriptor->isPhaser);
        st += (", frequency = " + TOSTRING(effectDescriptor->frequency)
               + "Hz");
        st += (", timeOffset = " + TOSTRING(effectDescriptor->timeOffset)
               + "s");
        st += ", inGain = " + TOSTRING(effectDescriptor->inGain);
        st += ", outGain = " + TOSTRING(effectDescriptor->outGain);
        st += ", delay = " + TOSTRING(effectDescriptor->delay) + "s";
        st += ", decay = " + TOSTRING(effectDescriptor->decay) + "s";
        st += ", depth = " + TOSTRING(effectDescriptor->depth) + "%";
        st += ", waveForm = " + effectDescriptor->waveForm.toString();
        st += (", delayRingBufferLength = "
               + TOSTRING(effectDescriptor->delayRingBufferLength));
        st += (", delayRingBufferIndex = "
               + TOSTRING(effectDescriptor->delayRingBufferIndex));
        st += (", delayRingBufferList = "
               + effectDescriptor->delayRingBufferList.toString());
        st += ")";

        return st;
    }

    /*--------------------*/

    /**
     * Sets up all audio editor parameters in <C>parameterMap</C> for
     * effect kind given as <C>effectKind</C>.
     *
     * @param[inout] parameterMap  audio parameter map of gain effect
     *                             to be initialized
     * @param[inout] effectKind    kind of effect (phaser/tremolo)
     */
    static void
    _initializeAllParameters (INOUT SoXAudioParameterMap& parameterMap,
                              INOUT String effectKind)
    {
        if (!_kindList.contains(effectKind)) {
            effectKind = _kindList[0];
        }

        const Boolean isTremolo = (effectKind == _tremoloEffectKind);

        parameterMap.clear();
        parameterMap.setKindAndValueEnum(parameterName_effectKind, _kindList,
                                         effectKind);

        if (isTremolo) {
            parameterMap.setKindReal(parameterName_frequency,
                                     0.1, 2.0, 0.001);
            parameterMap.setKindReal(parameterName_depth, 0.0, 100.0, 0.001);
        } else {
            const StringList waveFormKindValueList =
                StringList::makeBySplit("Sine/Triangle", separator);
            parameterMap.setKindReal(parameterName_inGain, 0.0, 1.0, 0.001);
            parameterMap.setKindReal(parameterName_outGain, 0.0, 1000.0, 0.001);
            parameterMap.setKindReal(parameterName_delayInMs, 0.0, 5.0, 0.001);
            parameterMap.setKindReal(parameterName_decay, 0.0, 0.99, 0.001);
            parameterMap.setKindReal(parameterName_frequency, 0.1, 2.0, 0.001);
            parameterMap.setKindEnum(parameterName_waveFormKind,
                                     waveFormKindValueList);
        }

        parameterMap.setKindReal(parameterName_timeOffset, -1E5, 1E5, 0.0001);
    }

    /*--------------------*/

    /**
     * Recalculates derived parameters in <C>effectDescriptor</C> from
     * parameter map and <C>sampleRate</C> and start time in
     * <C>currentTime</C>.
     *
     * @param[inout] effectDescriptor  descriptor of effect
     * @param[in] sampleRate           new sample rate for effect
     * @param[in] currentTime          current time for effect
     */
    static void
    _updateSettings (INOUT _EffectDescriptor_PHTR* effectDescriptor,
                     IN Real sampleRate, IN Real currentTime)
    {
        Logging_trace2(">>: sampleRate = %1, currentTime = %2",
                       TOSTRING(sampleRate), TOSTRING(currentTime));

        const Real frequency = effectDescriptor->frequency;
        const Real waveFormLength = sampleRate / frequency;
        Natural delayRingBufferLength;
        Real lowModulationValue;
        Real highModulationValue;
        Boolean hasIntegerValues;

        if (effectDescriptor->isPhaser) {
            // phaser
            delayRingBufferLength =
                Natural{Real::round(effectDescriptor->delay * sampleRate)};
            lowModulationValue  = 1.0;
            highModulationValue = Real{delayRingBufferLength};
            hasIntegerValues = true;
        } else {
            // tremolo: set some effect parameters to constants
            effectDescriptor->delay   = 0.0;
            effectDescriptor->inGain  = 1.0;
            effectDescriptor->outGain = 1.0;
            effectDescriptor->waveFormKind = WaveFormKind::sine;

            delayRingBufferLength = 0;
            lowModulationValue =
                Real{1.0} - effectDescriptor->depth / Real{100.0};
            highModulationValue = 1.0;
            hasIntegerValues = false;
        }

        // delay settings
        effectDescriptor->delayRingBufferIndex  = 0;
        effectDescriptor->delayRingBufferLength = delayRingBufferLength;
        effectDescriptor->delayRingBufferList.setRingBufferLength(delayRingBufferLength);
        effectDescriptor->delayRingBufferList.setToZero();

        // waveform
        const Radians effectivePhase =
            (_defaultPhase
             + WaveForm::phaseByTime(frequency,
                                        effectDescriptor->timeOffset,
                                        currentTime));

        WaveForm& waveForm = effectDescriptor->waveForm;
        waveForm.set(waveFormLength, effectDescriptor->waveFormKind,
                     lowModulationValue, highModulationValue,
                     effectivePhase, hasIntegerValues);

        Logging_trace1("<<: %1",
                       _effectDescriptorPHTRToString(effectDescriptor));
    }

}

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXPhaserAndTremolo_AudioEffect::SoXPhaserAndTremolo_AudioEffect ()
    : SoXAudioEffect()
{
    _effectDescriptor = _createEffectDescriptor(_sampleRate);
    _initializeAllParameters(_audioParameterMap, _tremoloEffectKind);
    setDefaultValues();
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXPhaserAndTremolo_AudioEffect::toString () const
{
    String st = "SoXPhaserAndTremolo_AudioEffect(";
    st += _asRawString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXPhaserAndTremolo_AudioEffect::_effectDescriptorToString () const
{
    _EffectDescriptor_PHTR* effectDescriptor =
        static_cast<_EffectDescriptor_PHTR*>(_effectDescriptor);
    return _effectDescriptorPHTRToString(effectDescriptor);
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXPhaserAndTremolo_AudioEffect::name() const
{
    return "SoX Phaser & Tremolo";
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXAudioValueChangeKind
SoXPhaserAndTremolo_AudioEffect::_setValueInternal
                                     (IN String& parameterName,
                                      IN String& value,
                                      IN Boolean recalculationIsSuppressed)
{
    SoXAudioValueChangeKind result =
        SoXAudioValueChangeKind::parameterChange;

    _EffectDescriptor_PHTR* effectDescriptor =
        static_cast<_EffectDescriptor_PHTR*>(_effectDescriptor);

    if (parameterName == parameterName_effectKind) {
        _initializeAllParameters(_audioParameterMap, value);
        effectDescriptor->isPhaser = (value != _tremoloEffectKind);
        result = SoXAudioValueChangeKind::globalChange;
    } else {
        if (parameterName == parameterName_decay) {
            effectDescriptor->decay = StringUtil::toReal(value);
        } else if (parameterName == parameterName_delayInMs) {
            effectDescriptor->delay = StringUtil::toReal(value) / 1000.0;
        } else if (parameterName == parameterName_depth) {
            effectDescriptor->depth = StringUtil::toPercentage(value);
        } else if (parameterName == parameterName_frequency) {
            effectDescriptor->frequency = StringUtil::toReal(value);
        } else if (parameterName == parameterName_inGain) {
            effectDescriptor->inGain = StringUtil::toReal(value);
        } else if (parameterName == parameterName_outGain) {
            effectDescriptor->outGain = StringUtil::toReal(value);
        } else if (parameterName == parameterName_waveFormKind) {
            effectDescriptor->waveFormKind = (value == "Sine"
                                              ? WaveFormKind::sine
                                              : WaveFormKind::triangle);
        } else if (parameterName == parameterName_timeOffset) {
            effectDescriptor->timeOffset = StringUtil::toReal(value);
        }

        if (!recalculationIsSuppressed) {
            _updateSettings(effectDescriptor, _sampleRate,
                            _currentTimePosition);
        }
    }

    return result;
}

/*--------------------*/

void SoXPhaserAndTremolo_AudioEffect::setDefaultValues ()
{
    const String effectKind =
        _audioParameterMap.value(parameterName_effectKind);
    const Boolean isTremolo = (effectKind == _tremoloEffectKind);

    if (isTremolo) {
        _audioParameterMap.setValue(parameterName_frequency, "0.5");
        _audioParameterMap.setValue(parameterName_depth,     "40");
    } else {
        _audioParameterMap.setValue(parameterName_inGain,       "0.4");
        _audioParameterMap.setValue(parameterName_outGain,      "0.74");
        _audioParameterMap.setValue(parameterName_delayInMs,    "3.0");
        _audioParameterMap.setValue(parameterName_decay,        "0.4");
        _audioParameterMap.setValue(parameterName_frequency,    "0.5");
        _audioParameterMap.setValue(parameterName_waveFormKind, "Triangle");
    }

    _audioParameterMap.setValue(parameterName_timeOffset, "0");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void
SoXPhaserAndTremolo_AudioEffect::processBlock
                                    (IN Real timePosition,
                                     INOUT AudioSampleListVector& buffer)
{
    SoXAudioEffect::processBlock(timePosition, buffer);

    _EffectDescriptor_PHTR* effectDescriptor =
        static_cast<_EffectDescriptor_PHTR*>(_effectDescriptor);

    if (_timePositionHasMoved) {
        // playhead was moved ==> keep time synchronisation of
        // waveform
        _updateSettings(effectDescriptor, _sampleRate,
                        _currentTimePosition);
    }

    const Natural sampleCount = buffer[0].size();
    const Boolean isPhaser = effectDescriptor->isPhaser;
    const Real inGain   = effectDescriptor->inGain;
    const Real outGain  = effectDescriptor->outGain;
    const Real decay    = effectDescriptor->decay;
    const Natural delayRingBufferLength =
        effectDescriptor->delayRingBufferLength;
    Natural delayRingBufferIndex  =
        effectDescriptor->delayRingBufferIndex;
    AudioSampleRingBufferVector& delayRingBufferList =
        effectDescriptor->delayRingBufferList;
    WaveForm& waveForm = effectDescriptor->waveForm;
    WaveFormIteratorState state = waveForm.state();

    for (Natural channel = 0;  channel < _channelCount;
         channel++) {
        const AudioSampleList& inputList = buffer[channel];
        AudioSampleList& outputList = buffer[channel];
        AudioSampleRingBuffer& delayRingBuffer = delayRingBufferList[channel];

        waveForm.setState(state);
        delayRingBufferIndex = effectDescriptor->delayRingBufferIndex;

        for (Natural i = 0;  i < sampleCount;  i++) {
            const AudioSample inputSample = inputList[i];
            AudioSample outputSample = 0.0;

            if (!isPhaser) {
                Real factor = waveForm.current();
                //outputSample = inputSample * waveForm.current();
                outputSample = inputSample * factor;
                //Logging_trace4("--: currentTime = %1, in = %2"
                //               ", fact = %3, out = %4",
                //               TOSTRING(timePosition), TOSTRING(inputSample),
                //               TOSTRING(factor), TOSTRING(outputSample));
            } else if (delayRingBufferLength > 0) {
                const Natural modulatedIndex =
                    ((delayRingBufferIndex + Natural{waveForm.current()})
                     % delayRingBufferLength);
                outputSample = (inputSample * inGain
                                + delayRingBuffer[modulatedIndex] * decay);
                delayRingBufferIndex =
                    (delayRingBufferIndex + 1) % delayRingBufferLength;
                delayRingBuffer[delayRingBufferIndex] = outputSample;
                outputSample *= outGain;
            }

            outputList[i] = outputSample;
            waveForm.advance();
        }
    }

    effectDescriptor->delayRingBufferIndex = delayRingBufferIndex;
}
