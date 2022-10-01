/**
 * @file
 * The <C>SoXOverdrive_AudioEffect</C> body implements the SoX
 * <B>overdrive</B> plugin distorting an input signal with a tanh
 * curve.
 *
 * The original algorithms can be found in the <A
 * HREF="https://sourceforge.net/p/sox/code">SoX repository</A>
 * and the file "overdrive.c" by Rob Sewell.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXOverdrive_AudioEffect.h"

#include <cmath>
#include "StringUtil.h"
#include "AudioSampleRingBufferVector.h"
#include "SoXAudioHelper.h"

/*--------------------*/

using Audio::AudioSampleRingBufferVector;
using BaseModules::StringUtil;
using SoXPlugins::Effects::SoXOverdrive::SoXOverdrive_AudioEffect;
using SoXPlugins::Helpers::SoXAudioHelper;

/*====================*/

namespace SoXPlugins::Effects::SoXOverdrive {

    /**
     * An <C>_EffectDescriptor_OVRD</C> object is the internal
     * implementation of an <B>overdrive</B> effect descriptor type
     * where all sample input is routed to and sample output is routed
     * from with two parameters gain factor and colour.
     */
    struct _EffectDescriptor_OVRD {

        /** the gain of this effect (as a factor) */
        Real gain;

        /** the colour of this effect (between 0 and 100) defining
          * a DC offset applied to the signal*/
        Real colour;

        /** the list of samples ring buffers for input and output */
        AudioSampleRingBufferVector sampleRingBufferVector;
    };

    /*====================*/

    /** the parameter name of the gain parameter */
    static const String parameterName_gain   = "Gain [dB]";

    /** the parameter name of the colour parameter */
    static const String parameterName_colour = "Colour";

    /** the factor from colour parameter to DC offset in the effect */
    static const Real colourFactor = 0.005;

    /*--------------------*/
    /* internal routines  */
    /*--------------------*/

    /**
     * Sets up a new effect descriptor and returns it.
     *
     * @return new overdrive effect descriptor
     */
    static _EffectDescriptor_OVRD* _createEffectDescriptor ()
    {
        _EffectDescriptor_OVRD* result =
            new _EffectDescriptor_OVRD{
                SoXAudioHelper::dBToLinear(0.0),  // gain
                Real{20.0} * colourFactor,               // colour
                {2, true, 1}                             // sampleRingBufferVector
            };

        return result;
    }

    /*--------------------*/

    /**
     * Sets up all audio editor parameters in <parameterMap> for effect
     *
     * @param[in] parameterMap  audio parameter map of overdrive effect
     *                          to be initialized
     */
    static
    void _initializeAllParameters (INOUT SoXAudioParameterMap& parameterMap)
    {
        parameterMap.setKindAndValueInt(parameterName_gain, 0, 100, 1, 20);
        parameterMap.setKindAndValueInt(parameterName_colour, 0, 100, 1, 20);
    }

}

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXOverdrive_AudioEffect::SoXOverdrive_AudioEffect ()
    : SoXAudioEffect()
{
    _effectDescriptor = _createEffectDescriptor();
    _initializeAllParameters(_audioParameterMap);
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXOverdrive_AudioEffect::toString () const
{
    String st = "SoXOverdrive_AudioEffect(";
    st += _asRawString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXOverdrive_AudioEffect::_effectDescriptorToString () const
{
    _EffectDescriptor_OVRD* effectDescriptor =
        static_cast<_EffectDescriptor_OVRD*>(_effectDescriptor);

    String st = "_EffectDescriptor_OVRD(";
    st += "gain = " + StringUtil::toString(effectDescriptor->gain);
    st += ", colour = " + StringUtil::toString(effectDescriptor->colour);
    st += (", sampleRingBufferVector = "
           + effectDescriptor->sampleRingBufferVector.toString());
    st += ")";

    return st;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXOverdrive_AudioEffect::name() const
{
    return "SoX Overdrive";
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXAudioValueChangeKind
SoXOverdrive_AudioEffect::_setValueInternal
                              (IN String& parameterName,
                               IN String& value,
                               IN Boolean recalculationIsSuppressed)
{
    _EffectDescriptor_OVRD* effectDescriptor =
        static_cast<_EffectDescriptor_OVRD*>(_effectDescriptor);

    if (parameterName == parameterName_gain) {
        const Real dBGain = StringUtil::toReal(value);
        effectDescriptor->gain = SoXAudioHelper::dBToLinear(dBGain);
    } else if (parameterName == parameterName_colour) {
        effectDescriptor->colour = (StringUtil::toReal(value)
                                    * colourFactor);
    }

    return SoXAudioValueChangeKind::parameterChange;
}

/*--------------------*/

void SoXOverdrive_AudioEffect::setDefaultValues () {
    _audioParameterMap.setValue(parameterName_gain, "20");
    _audioParameterMap.setValue(parameterName_colour, "20");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void
SoXOverdrive_AudioEffect::processBlock (IN Real timePosition,
                                        INOUT AudioSampleListVector& buffer)
{
    SoXAudioEffect::processBlock(timePosition, buffer);
    _EffectDescriptor_OVRD* effectDescriptor =
        static_cast<_EffectDescriptor_OVRD*>(_effectDescriptor);

    const Natural sampleCount = buffer[0].size();
    const Real gain   = effectDescriptor->gain;
    const Real colour = effectDescriptor->colour;
    AudioSampleRingBufferVector& sampleRingBufferVector =
        effectDescriptor->sampleRingBufferVector;

    for (Natural channel = 0;  channel < _channelCount;
         channel++) {
        AudioSampleRingBuffer& inputSampleRingBuffer =
            sampleRingBufferVector.at(channel, 0);
        AudioSampleRingBuffer& outputSampleRingBuffer =
            sampleRingBufferVector.at(channel, 1);
        const AudioSampleList& inputList  = buffer[channel];
        AudioSampleList& outputList = buffer[channel];

        for (Natural i = 0;  i < sampleCount;  i++) {
            const AudioSample inputSample = inputList[i];
            const AudioSample previousInputSample =
                inputSampleRingBuffer.first();
            const AudioSample previousOutputSample =
                outputSampleRingBuffer.first();
            AudioSample newValue;

            newValue = (AudioSample) (inputSample * gain + colour);
            newValue = newValue.forceToRange(-1.0, 1.0);
            newValue = (newValue - (newValue * newValue * newValue)
                        / Real{3.0});
            const AudioSample outputSample =
                (newValue - previousInputSample
                 + Real{0.995}  * previousOutputSample);
            outputList[i] = (inputSample / Real{2.0}
                             + outputSample * Real{0.75});
            inputSampleRingBuffer.setFirst(newValue);
            outputSampleRingBuffer.setFirst(outputSample);
        }
    }
}
