/**
 * @file
 * The <C>SoXPhaserAndTremolo_AudioEffect</C> body implements the SoX
 * <B>phaser</B> and <B>tremolo</B> modulation effects that have
 * similar parameters and hence are combined into a single plugin.
 *
 * @author Dr. Thomas Tensi
 */

/*====================*/

#include <cmath>
#include <cstdio>

#include "Natural.h"
#include "Percentage.h"
#include "Radians.h"
#include "Real.h"
#include "MyString.h"
#include "StringList.h"
#include "StringUtil.h"

#include "SoXAudioHelper.h"
#include "SoXAudioSampleMatrix.h"
#include "SoXAudioSampleQueue.h"
#include "SoXPhaserAndTremolo_AudioEffect.h"
#include "SoXWaveForm.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::Percentage;
using SoXPlugins::BaseTypes::Primitives::Radians;
using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::BaseTypes::Containers::StringList;
using SoXPlugins::CommonAudio::SoXAudioSampleQueue;
using SoXPlugins::CommonAudio::SoXAudioSampleMatrix;
using SoXPlugins::CommonAudio::SoXWaveForm;
using SoXPlugins::CommonAudio::SoXWaveFormKind;
using SoXPlugins::CommonAudio::SoXWaveFormIteratorState;
using SoXPlugins::Effects::SoXPhaserAndTremolo
      ::SoXPhaserAndTremolo_AudioEffect;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;

/*============================================================*/

namespace SoXPlugins::Effects::SoXPhaserAndTremolo {

    /** the maximum allowable delay in seconds */
    static const Real _maximumDelay = 0.005;

    /** the internal separator for lists */
    static const String separator = "/";

    /** the standard starting phase of a phaser (90°) */
    static const Radians _defaultPhase = Real::pi / Real{2};

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
        bool isPhaser;

        /** the modulation frequency in Hz */
        Real frequency;

        /** the modulation waveform kind */
        SoXWaveFormKind waveFormKind;

        /** the waveform data */
        SoXWaveForm waveForm;

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

        /** the buffer for the delay line */
        SoXAudioSampleMatrix delayBufferList;

        /** the delay buffer length in samples */
        Natural delayBufferLength;

        /** the pointer to the delay buffer (as an index) */
        Natural delayBufferIndex;
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
                SoXWaveFormKind::triangle,             // waveFormKind
                {},                                    // waveForm
                0.0,                                   // timeOffset

                0.4,                                   // inGain
                0.74,                                  // outGain
                0.003,                                 // delay
                0.4,                                   // decay
                40,                                    // depth

                {2, false, maximumDelayBufferLength},  // delayBufferList
                maximumDelayBufferLength,              // delayBufferLength
                0                                      // delayBufferIndex
            };

        return result;
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

        const bool isTremolo = (effectKind == _tremoloEffectKind);

        parameterMap.clear();
        parameterMap.setKindAndValueEnum(parameterName_effectKind, _kindList,
                                         effectKind);

        if (isTremolo) {
            parameterMap.setKindReal(parameterName_frequency, 0.1, 2, 0.001);
            parameterMap.setKindReal(parameterName_depth, 0, 100, 0.001);
        } else {
            const StringList waveFormKindValueList =
                StringList::makeBySplit("Sine/Triangle", separator);
            parameterMap.setKindReal(parameterName_inGain, 0, 1, 0.001);
            parameterMap.setKindReal(parameterName_outGain, 0, 1000, 0.001);
            parameterMap.setKindReal(parameterName_delayInMs, 0, 5, 0.001);
            parameterMap.setKindReal(parameterName_decay, 0, 0.99, 0.001);
            parameterMap.setKindReal(parameterName_frequency, 0.1, 2, 0.001);
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
        const Real frequency = effectDescriptor->frequency;
        const Real waveFormLength = sampleRate / frequency;
        Natural delayBufferLength;
        Real lowModulationValue;
        Real highModulationValue;
        bool hasIntegerValues;

        if (effectDescriptor->isPhaser) {
            // phaser
            delayBufferLength =
                Natural::round((double)(effectDescriptor->delay * sampleRate));
            lowModulationValue  = 1;
            highModulationValue = Real{delayBufferLength};
            hasIntegerValues = true;
        } else {
            // tremolo: set some effect parameters to constants
            effectDescriptor->delay   = 0;
            effectDescriptor->inGain  = 1;
            effectDescriptor->outGain = 1;
            effectDescriptor->waveFormKind = SoXWaveFormKind::sine;

            delayBufferLength = 0;
            lowModulationValue =
                Real{1} - effectDescriptor->depth / Real{100.0};
            highModulationValue = 1;
            hasIntegerValues = false;
        }

        // delay settings
        effectDescriptor->delayBufferIndex  = 0;
        effectDescriptor->delayBufferLength = delayBufferLength;
        effectDescriptor->delayBufferList.setQueueLength(delayBufferLength);
        effectDescriptor->delayBufferList.setToZero();

        // waveform
        const Radians effectivePhase =
            (_defaultPhase
             + SoXWaveForm::phaseByTime(frequency,
                                        effectDescriptor->timeOffset,
                                        currentTime));

        SoXWaveForm& waveForm = effectDescriptor->waveForm;
        waveForm.set(waveFormLength, effectDescriptor->waveFormKind,
                     lowModulationValue, highModulationValue,
                     effectivePhase, hasIntegerValues);
    }

};

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

    String st = "_EffectDescriptor_PHTR(";
    st += "isPhaser = " + TOSTRING(effectDescriptor->isPhaser);
    st += ", frequency = " + TOSTRING(effectDescriptor->frequency) + "Hz";
    st += ", timeOffset = " + TOSTRING(effectDescriptor->timeOffset) + "s";
    st += ", inGain = " + TOSTRING(effectDescriptor->inGain);
    st += ", outGain = " + TOSTRING(effectDescriptor->outGain);
    st += ", delay = " + TOSTRING(effectDescriptor->delay) + "s";
    st += ", decay = " + TOSTRING(effectDescriptor->decay) + "s";
    st += ", depth = " + TOSTRING(effectDescriptor->depth) + "%";
    st += (effectDescriptor->waveFormKind == SoXWaveFormKind::sine
           ? "sine" : "triangle");
    st += ", waveForm = " + effectDescriptor->waveForm.toString();
    st += (", delayBufferLength = "
           + TOSTRING(effectDescriptor->delayBufferLength));
    st += (", delayBufferIndex = "
           + TOSTRING(effectDescriptor->delayBufferIndex));
    st += (", delayBufferList = "
           + effectDescriptor->delayBufferList.toString());
    st += ")";

    return st;
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
                                      IN bool recalculationIsSuppressed)
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
            effectDescriptor->depth = (double) StringUtil::toReal(value);
        } else if (parameterName == parameterName_frequency) {
            effectDescriptor->frequency = StringUtil::toReal(value);
        } else if (parameterName == parameterName_inGain) {
            effectDescriptor->inGain = StringUtil::toReal(value);
        } else if (parameterName == parameterName_outGain) {
            effectDescriptor->outGain = StringUtil::toReal(value);
        } else if (parameterName == parameterName_waveFormKind) {
            effectDescriptor->waveFormKind = (value == "Sine"
                                              ? SoXWaveFormKind::sine
                                              : SoXWaveFormKind::triangle);
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
    const bool isTremolo = (effectKind == _tremoloEffectKind);

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

void SoXPhaserAndTremolo_AudioEffect::processBlock
                                          (IN Real timePosition,
                                           INOUT SoXAudioSampleBuffer& buffer)
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
    const bool isPhaser = effectDescriptor->isPhaser;
    const Real inGain   = effectDescriptor->inGain;
    const Real outGain  = effectDescriptor->outGain;
    const Real decay    = effectDescriptor->decay;
    const Natural delayBufferLength =
        effectDescriptor->delayBufferLength;
    Natural delayBufferIndex  =
        effectDescriptor->delayBufferIndex;
    SoXAudioSampleMatrix& delayBufferList =
        effectDescriptor->delayBufferList;
    SoXWaveForm& waveForm = effectDescriptor->waveForm;
    SoXWaveFormIteratorState state = waveForm.state();

    for (Natural channel = 0;  channel < _channelCount;
         channel++) {
        const SoXAudioSampleList& inputList = buffer[channel];
        SoXAudioSampleList& outputList = buffer[channel];
        SoXAudioSampleQueue& delayBuffer = delayBufferList[channel];

        waveForm.setState(state);
        delayBufferIndex = effectDescriptor->delayBufferIndex;

        for (Natural i = 0;  i < sampleCount;  i++) {
            const SoXAudioSample inputSample = inputList[i];
            SoXAudioSample outputSample = 0;

            if (!isPhaser) {
                outputSample = inputSample * waveForm.current();
            } else if (delayBufferLength > 0) {
                const Natural modulatedIndex =
                    ((delayBufferIndex
                      + Natural::floor((double) waveForm.current()))
                     % delayBufferLength);
                outputSample = (inputSample * inGain
                                + delayBuffer[modulatedIndex] * decay);
                delayBufferIndex =
                    (delayBufferIndex + 1) % delayBufferLength;
                delayBuffer[delayBufferIndex] = outputSample;
                outputSample *= outGain;
            }

            outputList[i] = outputSample;
            waveForm.advance();
        }
    }

    effectDescriptor->delayBufferIndex     = delayBufferIndex;
    _previousTimePosition = _currentTimePosition;
}
