/**
 * @file
 * The <C>SoXReverb_AudioEffect</C> body implements the SoX
 * <B>reverb</B> plugin using the freeverb algorithm for room
 * simulation.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

/*====================*/

#include "SoXReverb_AudioEffect.h"

#include <vector>
#include "Natural.h"
#include "Percentage.h"
#include "SoXAudioHelper.h"
#include "SoXReverbSupport.h"
#include "StringUtil.h"

/*====================*/

using std::vector;
using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::Percentage;
using SoXPlugins::Effects::SoXReverb::_SoXReverb;
using SoXPlugins::Effects::SoXReverb::SoXReverb_AudioEffect;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
namespace SoXAudioHelper = SoXPlugins::CommonAudio::SoXAudioHelper;

/*============================================================*/

namespace SoXPlugins::Effects::SoXReverb {

    /** the possible values of a yes-no-combobox (in English language) */
    static const StringList _yesNoList =
        StringList::makeBySplit("Yes/No", "/");

    /*--------------------*/

    /**
     * An <C>_EffectDescriptor_RVRB</C> object is the internal
     * implementation of a reverb effect descriptor type where all
     * sample input is routed to and sample output is routed from with
     * several parameters for the freeverb algorithm (like e.g.\ the
     * room scale).
     */
    struct _EffectDescriptor_RVRB {

        /** information whether direct signal should be suppressed in
         * output */
        bool isWetOnly;

        /** the reverberance in percent */
        Percentage reverberance;

        /** the HF damping in percent */
        Percentage hfDamping;

        /** the room scale (room size) in percent */
        Percentage roomScale;

        /** the stereo depth in percent */
        Percentage stereoDepth;

        /** the pre-delay in milliseconds */
        Real preDelayInMs;

        /** the gain of the wet signal in decibels */
        Real wetDbGain;

        /** the number of channels of this effect */
        Natural channelCount;

        /** the internal reverb effect */
        _SoXReverb reverb;
    };

    /*====================*/

    /*....................*/
    /* parameter names    */
    /*....................*/

    /** the name for the isWetOnly parameter */
    static const String parameterName_isWetOnly    = "Is Wet Only?";

    /** the name for the reverberance parameter */
    static const String parameterName_reverberance = "Reverberance [%]";

    /** the name for the hfDamping parameter */
    static const String parameterName_hfDamping    = "HF Damping [%]";

    /** the name for the roomScale parameter */
    static const String parameterName_roomScale    = "Room Scale [%]";

    /** the name for the stereoDepth parameter */
    static const String parameterName_stereoDepth  = "Stereo Depth [%]";

    /** the name for the preDelay parameter */
    static const String parameterName_preDelay     = "Predelay [ms]";

    /** the name for the wetGain parameter */
    static const String parameterName_wetGain      = "Wet Gain [dB]";

    /*--------------------*/
    /* internal routines  */
    /*--------------------*/

    /**
     * Sets up a new effect descriptor.
     *
     * @return new reverb effect descriptor
     */
    static _EffectDescriptor_RVRB* _createEffectDescriptor ()
    {
        _EffectDescriptor_RVRB* result =
            new _EffectDescriptor_RVRB{
                false, // isWetOnly
                50,    // reverberance
                50,    // hfDamping
                100,   // roomScale
                100,   // stereoDepth
                0,     // preDelayInMs
                0,     // wetDbGain
                0,     // channelCount
                {}     // reverb
            };

        return result;
    }

    /*--------------------*/

    /**
     * Sets up all audio editor parameters in <C>parameterMap</C> for
     * effect
     *
     * @param[in] parameterMap  audio parameter map of reverb effect to be
     *                          initialized
     */
    static void
    _initializeAllParameters (INOUT SoXAudioParameterMap& parameterMap)
    {
        parameterMap.setKindEnum(parameterName_isWetOnly, _yesNoList);
        parameterMap.setKindReal(parameterName_reverberance, 0, 100, 0.001);
        parameterMap.setKindReal(parameterName_hfDamping, 0, 100, 0.001);
        parameterMap.setKindReal(parameterName_roomScale, 0, 100, 0.001);
        parameterMap.setKindReal(parameterName_stereoDepth, 0, 100, 0.001);
        parameterMap.setKindReal(parameterName_preDelay, 0, 500, 0.001);
        parameterMap.setKindReal(parameterName_wetGain, -100, 100, 0.001);
    }

    /*--------------------*/

    /**
     * Recalculates derived parameters in reverb <C>effectDescriptor</C>
     * from other parameters, <C>sampleRate</C> and <C>channelCount</C>.
     *
     * @param[inout] effectDescriptor  effect descriptor of reverb
     * @param[in]    sampleRate        new sample rate
     * @param[in]    channelCount      new channel count
     */
    static void
    _updateSettings (INOUT _EffectDescriptor_RVRB* effectDescriptor,
                     IN Real sampleRate,
                     IN Natural channelCount)
    {
        _SoXReverb& reverb = effectDescriptor->reverb;
        reverb.setParameters(effectDescriptor->isWetOnly,
                             effectDescriptor->reverberance,
                             effectDescriptor->hfDamping,
                             effectDescriptor->roomScale,
                             effectDescriptor->stereoDepth,
                             effectDescriptor->preDelayInMs / 1000,
                             effectDescriptor->wetDbGain);
        reverb.resize(sampleRate, channelCount);
    }

}

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXReverb_AudioEffect::SoXReverb_AudioEffect ()
    : SoXAudioEffect{}
{
    _EffectDescriptor_RVRB* effectDescriptor = _createEffectDescriptor();
    _effectDescriptor = effectDescriptor;
    _initializeAllParameters(_audioParameterMap);
    _updateSettings(effectDescriptor, _sampleRate, _channelCount);
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXReverb_AudioEffect::toString () const
{
    String st = "SoXReverb_AudioEffect(";
    st += _asRawString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXReverb_AudioEffect::_effectDescriptorToString () const
{
    _EffectDescriptor_RVRB* effectDescriptor =
        static_cast<_EffectDescriptor_RVRB*>(_effectDescriptor);

    String st = "_EffectDescriptor_RVRB(";
    st += "isWetOnly = " + TOSTRING(effectDescriptor->isWetOnly);
    st += (", reverberance = "
           + TOSTRING(effectDescriptor->reverberance) + "%");
    st += ", hfDamping = " + TOSTRING(effectDescriptor->hfDamping) + "%";
    st += ", roomScale = " + TOSTRING(effectDescriptor->roomScale) + "%";
    st += (", stereoDepth = "
           + TOSTRING(effectDescriptor->stereoDepth) + "%");
    st += (", preDelayInMs = "
           + TOSTRING(effectDescriptor->preDelayInMs) + "ms");
    st += ", wetDbGain = " + TOSTRING(effectDescriptor->wetDbGain) + "dB";
    st += ", channelCount = " + TOSTRING(effectDescriptor->channelCount);
    st += ", reverb = " + effectDescriptor->reverb.toString();
    st += ")";

    return st;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXReverb_AudioEffect::name() const
{
    return "SoX Reverb";
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXAudioValueChangeKind
SoXReverb_AudioEffect::_setValueInternal (IN String& parameterName,
                                          IN String& value,
                                          IN bool recalculationIsSuppressed)
{
    _EffectDescriptor_RVRB* effectDescriptor =
        static_cast<_EffectDescriptor_RVRB*>(_effectDescriptor);

    if (parameterName == parameterName_isWetOnly) {
        effectDescriptor->isWetOnly = (value == "Yes");
    } else if (parameterName == parameterName_reverberance) {
        effectDescriptor->reverberance =
            Percentage{(double) StringUtil::toReal(value)};
    } else if (parameterName == parameterName_hfDamping) {
        effectDescriptor->hfDamping =
            Percentage{(double) StringUtil::toReal(value)};
    } else if (parameterName == parameterName_roomScale) {
        effectDescriptor->roomScale =
            Percentage{(double) StringUtil::toReal(value)};
    } else if (parameterName == parameterName_stereoDepth) {
        effectDescriptor->stereoDepth =
            Percentage{(double) StringUtil::toReal(value)};
    } else if (parameterName == parameterName_preDelay) {
        effectDescriptor->preDelayInMs = StringUtil::toReal(value);
    } else if (parameterName == parameterName_wetGain) {
        effectDescriptor->wetDbGain = StringUtil::toReal(value);
    }

    if (!recalculationIsSuppressed) {
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
    }

    return SoXAudioValueChangeKind::parameterChange;
}

/*--------------------*/

void SoXReverb_AudioEffect::setDefaultValues ()
{
    _audioParameterMap.setValue(parameterName_isWetOnly,    "No");
    _audioParameterMap.setValue(parameterName_reverberance, "50");
    _audioParameterMap.setValue(parameterName_hfDamping,    "50");
    _audioParameterMap.setValue(parameterName_roomScale,    "100");
    _audioParameterMap.setValue(parameterName_stereoDepth,  "100");
    _audioParameterMap.setValue(parameterName_preDelay,     "0");
    _audioParameterMap.setValue(parameterName_wetGain,      "0");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXReverb_AudioEffect::processBlock
                                (IN Real timePosition,
                                 INOUT SoXAudioSampleListVector& buffer)
{
    SoXAudioEffect::processBlock(timePosition, buffer);

    // apply the effect
    _EffectDescriptor_RVRB* effectDescriptor =
        static_cast<_EffectDescriptor_RVRB*>(_effectDescriptor);

    if (_channelCount != effectDescriptor->channelCount) {
        effectDescriptor->channelCount = _channelCount;
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
    }

    const Natural sampleCount = buffer[0].size();
    _SoXReverb& reverb = effectDescriptor->reverb;
    SoXAudioSampleQueue inputSampleList{_channelCount};
    SoXAudioSampleQueue outputSampleList{_channelCount};

    for (Natural i = 0;  i < sampleCount;  i++) {
        // read all channel samples into one list
        for (Natural channel = 0;  channel < _channelCount;
             channel++) {
            const SoXAudioSampleList& inputList = buffer[channel];
            inputSampleList[channel] = inputList[i];
        }

        reverb.apply(inputSampleList, outputSampleList);

        // write output sample list onto all channels
        for (Natural channel = 0;  channel < _channelCount;
             channel++) {
            SoXAudioSampleList& outputList = buffer[channel];
            outputList[i] = outputSampleList[channel];
        }
    }
}
