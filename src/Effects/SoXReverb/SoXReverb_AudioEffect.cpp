/**
 * @file
 * The <C>SoXReverb_AudioEffect</C> body implements the SoX
 * <B>reverb</B> plugin using the freeverb algorithm for room
 * simulation.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXReverb_AudioEffect.h"
#include "Logging.h"
#include "SoXReverbSupport.h"

/*--------------------*/

using SoXPlugins::Effects::SoXReverb::_SoXReverb;
using SoXPlugins::Effects::SoXReverb::SoXReverb_AudioEffect;

/** abbreviation for StringUtil::expand */
#define expand StringUtil::expand

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
        Boolean isWetOnly;

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

        /*--------------------*/
        /*--------------------*/

        /**
         * Returns descriptor string representation.
         *
         * @return  string representation of effect
         */
        String toString () const
        {
            String st =
                expand("_EffectDescriptor_RVRB("
                       "isWetOnly = %1, reverberance = %2%,"
                       " hfDamping = %3%, roomScale = %4%,"
                       " stereoDepth = %5%, preDelayInMs = %6ms,"
                       " wetDbGain = %7dB, channelCount = %8,"
                       " reverb = %9)",
                       TOSTRING(isWetOnly), TOSTRING(reverberance),
                       TOSTRING(hfDamping), TOSTRING(roomScale),
                       TOSTRING(stereoDepth), TOSTRING(preDelayInMs),
                       TOSTRING(wetDbGain), TOSTRING(channelCount),
                       reverb.toString());

            return st;
        }

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
        Logging_trace(">>");

        _EffectDescriptor_RVRB* result =
            new _EffectDescriptor_RVRB{
                false, // isWetOnly
                50.0,  // reverberance
                50.0,  // hfDamping
                100.0, // roomScale
                100.0, // stereoDepth
                0.0,   // preDelayInMs
                0.0,   // wetDbGain
                0,     // channelCount
                {}     // reverb
            };

        Logging_trace1("<<: %1", result->toString());
        return result;
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
    _updateSettings (INOUT _EffectDescriptor_RVRB& effectDescriptor,
                     IN Real sampleRate,
                     IN Natural channelCount)
    {
        Logging_trace3(">>: descriptor = %1, sampleRate = %2,"
                       " channelCount = %3",
                       effectDescriptor.toString(),
                       TOSTRING(sampleRate), TOSTRING(channelCount));

        _SoXReverb& reverb = effectDescriptor.reverb;
        reverb.setParameters(effectDescriptor.isWetOnly,
                             effectDescriptor.reverberance,
                             effectDescriptor.hfDamping,
                             effectDescriptor.roomScale,
                             effectDescriptor.stereoDepth,
                             effectDescriptor.preDelayInMs / 1000.0,
                             effectDescriptor.wetDbGain);
        reverb.resize(sampleRate, channelCount);

        Logging_trace1("<<: %1", effectDescriptor.toString());
    }

}

/*============================================================*/

/*---------------------*/
/* setup & destruction */
/*---------------------*/

SoXReverb_AudioEffect::SoXReverb_AudioEffect ()
{
    Logging_trace(">>");

    /* initialize descriptor */
    _effectDescriptor = _createEffectDescriptor();
    _EffectDescriptor_RVRB& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);

    /* initialize parameters */
    _effectParameterMap.clear();
    _effectParameterMap.setKindEnum(parameterName_isWetOnly, _yesNoList);
    _effectParameterMap.setKindReal(parameterName_reverberance,
                                    0.0, 100.0, 0.001);
    _effectParameterMap.setKindReal(parameterName_hfDamping,
                                    0.0, 100.0, 0.001);
    _effectParameterMap.setKindReal(parameterName_roomScale,
                                    0.0, 100.0, 0.001);
    _effectParameterMap.setKindReal(parameterName_stereoDepth,
                                    0.0, 100.0, 0.001);
    _effectParameterMap.setKindReal(parameterName_preDelay,
                                    0.0, 500.0, 0.001);
    _effectParameterMap.setKindReal(parameterName_wetGain,
                                    -100.0, 100.0, 0.001);

    _updateSettings(effectDescriptor, _sampleRate, _channelCount);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXReverb_AudioEffect::~SoXReverb_AudioEffect ()
{
    Logging_trace(">>");
    delete (_EffectDescriptor_RVRB*) _effectDescriptor;
    Logging_trace("<<");
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
    _EffectDescriptor_RVRB& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);
    return effectDescriptor.toString();
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

SoXParameterValueChangeKind
SoXReverb_AudioEffect::_setValueInternal
                           (IN String& parameterName,
                            IN String& value,
                            IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalculationIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    _EffectDescriptor_RVRB& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);
    SoXParameterValueChangeKind result =
        SoXParameterValueChangeKind::parameterChange;

    if (parameterName == parameterName_isWetOnly) {
        effectDescriptor.isWetOnly = (value == "Yes");
    } else if (parameterName == parameterName_reverberance) {
        effectDescriptor.reverberance = StringUtil::toPercentage(value);
    } else if (parameterName == parameterName_hfDamping) {
        effectDescriptor.hfDamping = StringUtil::toPercentage(value);
    } else if (parameterName == parameterName_roomScale) {
        effectDescriptor.roomScale = StringUtil::toPercentage(value);
    } else if (parameterName == parameterName_stereoDepth) {
        effectDescriptor.stereoDepth = StringUtil::toPercentage(value);
    } else if (parameterName == parameterName_preDelay) {
        effectDescriptor.preDelayInMs = StringUtil::toReal(value);
    } else if (parameterName == parameterName_wetGain) {
        effectDescriptor.wetDbGain = StringUtil::toReal(value);
    }

    if (recalculationIsForced) {
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
    }

    Logging_trace1("<<: %1", SoXParameterValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXReverb_AudioEffect::setDefaultValues ()
{
    Logging_trace(">>");

    _effectParameterMap.setValue(parameterName_isWetOnly,    "No");
    _effectParameterMap.setValue(parameterName_reverberance, "50");
    _effectParameterMap.setValue(parameterName_hfDamping,    "50");
    _effectParameterMap.setValue(parameterName_roomScale,    "100");
    _effectParameterMap.setValue(parameterName_stereoDepth,  "100");
    _effectParameterMap.setValue(parameterName_preDelay,     "0");
    _effectParameterMap.setValue(parameterName_wetGain,      "0");

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXReverb_AudioEffect::processBlock
                                (IN Real timePosition,
                                 INOUT AudioSampleListVector& buffer)
{
    Logging_trace1(">>: time = %1", TOSTRING(timePosition));

    SoXAudioEffect::processBlock(timePosition, buffer);

    // apply the effect
    _EffectDescriptor_RVRB& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);

    if (_channelCount != effectDescriptor.channelCount) {
        effectDescriptor.channelCount = _channelCount;
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
    }

    const Natural sampleCount = buffer[0].size();
    _SoXReverb& reverb = effectDescriptor.reverb;
    AudioSampleRingBuffer inputSampleList{_channelCount};
    AudioSampleRingBuffer outputSampleList{_channelCount};

    for (Natural i = 0;  i < sampleCount;  i++) {
        // read all channel samples into one list
        for (Natural channel = 0;  channel < _channelCount;
             channel++) {
            const AudioSampleList& inputList = buffer[channel];
            inputSampleList[channel] = inputList[i];
        }

        reverb.apply(inputSampleList, outputSampleList);

        // write output sample list onto all channels
        for (Natural channel = 0;  channel < _channelCount;
             channel++) {
            AudioSampleList& outputList = buffer[channel];
            outputList[i] = outputSampleList[channel];
        }
    }

    Logging_trace("<<");
}
