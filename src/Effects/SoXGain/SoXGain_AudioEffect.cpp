/**
 * @file
 * The <C>SoXGain_AudioEffect</C> body implements the SoX <B>gain</B>
 * plugin amplifying or attenuating the input signal by a specific dB
 * amount.
 *
 * @author Dr. Thomas Tensi
 */

/*====================*/

#include "SoXAudioHelper.h"
#include "SoXGain_AudioEffect.h"
#include "StringUtil.h"

/*====================*/

using SoXPlugins::Effects::SoXGain::SoXGain_AudioEffect;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
namespace SoXAudioHelper = SoXPlugins::CommonAudio::SoXAudioHelper;

/*============================================================*/

namespace SoXPlugins::Effects::SoXGain {

    /**
     * An <C>_EffectDescriptor_GAIN</C> object is the internal
     * implementation of a gain effect descriptor type where all
     * sample input is routed to and sample output is routed from with
     * a single gain factor parameter.
     */
    struct _EffectDescriptor_GAIN {

        /** the gain (as a real factor) */
        Real gain;
    };

    /*====================*/

    /** the parameter name of the gain parameter */
    static const String parameterName_gain   = "Gain [dB]";

    /*--------------------*/
    /* internal routines  */
    /*--------------------*/

    /**
     * Sets up a new gain effect descriptor and returns it.
     *
     * @return new gain effect descriptor
     */
    static _EffectDescriptor_GAIN* _createEffectDescriptor ()
    {
        _EffectDescriptor_GAIN* result =
            new _EffectDescriptor_GAIN{
                0.0 // gain
            };

        return result;
    }

    /*--------------------*/

    /**
     * Sets up all audio editor parameters in <C>parameterMap</C> for
     * effect
     *
     * @param[in] parameterMap  audio parameter map of gain effect to be
     *                          initialized
     */
    static
    void _initializeAllParameters (INOUT SoXAudioParameterMap& parameterMap)
    {
        parameterMap.setKindReal(parameterName_gain, -10, 10, 0.001);
    }

};

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXGain_AudioEffect::SoXGain_AudioEffect ()
    : SoXAudioEffect{}
{
    _effectDescriptor = _createEffectDescriptor();
    _audioParameterMap.clear();
    _initializeAllParameters(_audioParameterMap);
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXGain_AudioEffect::toString () const
{
    String st = "SoXGain_AudioEffect(";
    st += _asRawString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXGain_AudioEffect::_effectDescriptorToString () const
{
    _EffectDescriptor_GAIN* effectDescriptor =
        static_cast<_EffectDescriptor_GAIN*>(_effectDescriptor);

    String st = "_EffectDescriptor_GAIN(";
    st += "gain = " + TOSTRING(effectDescriptor->gain);
    st += ")";

    return st;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXGain_AudioEffect::name() const
{
    return "SoX Gain";
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXAudioValueChangeKind
SoXGain_AudioEffect::_setValueInternal (IN String& parameterName,
                                        IN String& value,
                                        IN bool recalculationIsSuppressed)
{
    _EffectDescriptor_GAIN* effectDescriptor =
        static_cast<_EffectDescriptor_GAIN*>(_effectDescriptor);
    SoXAudioEffect::setValue(parameterName, value);

    if (parameterName == parameterName_gain) {
        const Real dBGain = StringUtil::toReal(value);
        effectDescriptor->gain = SoXAudioHelper::dBToLinear(dBGain);
    }

    return SoXAudioValueChangeKind::parameterChange;
}

/*--------------------*/

void SoXGain_AudioEffect::setDefaultValues () {
    _audioParameterMap.setValue(parameterName_gain, "0");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXGain_AudioEffect::processBlock (IN Real timePosition,
                                        INOUT SoXAudioSampleBuffer& buffer)
{
    SoXAudioEffect::processBlock(timePosition, buffer);
    _EffectDescriptor_GAIN* effectDescriptor =
        static_cast<_EffectDescriptor_GAIN*>(_effectDescriptor);

    const Natural sampleCount = buffer[0].size();
    const Real gain = effectDescriptor->gain;

    for (Natural channel = 0;  channel < _channelCount;
         channel++) {
        const SoXAudioSampleList& inputList  = buffer[channel];
        SoXAudioSampleList& outputList = buffer[channel];

        for (Natural i = 0;  i < sampleCount;  i++) {
            const SoXAudioSample inputSample = inputList[i];
            SoXAudioSample& outputSample     = outputList[i];
            outputSample = (SoXAudioSample) (inputSample * gain);
        }
    }
}
