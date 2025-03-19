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
#include "Logging.h"
#include "SoXAudioHelper.h"

/*--------------------*/

using SoXPlugins::Effects::SoXOverdrive::SoXOverdrive_AudioEffect;
using SoXPlugins::Helpers::SoXAudioHelper;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/*====================*/

/** the parameter name of the gain parameter */
static const String parameterName_gain   = "Gain [dB]";

/** the parameter name of the colour parameter */
static const String parameterName_colour = "Colour";

/** the factor from colour parameter to DC offset in the effect */
static const Real colourFactor = 0.005;

/*====================*/

namespace SoXPlugins::Effects::SoXOverdrive {

    /**
     * An <C>_EffectDescriptor_ODRV</C> object is the internal
     * implementation of an <B>overdrive</B> effect descriptor type
     * where all sample input is routed to and sample output is routed
     * from with two parameters gain factor and colour.
     */
    struct _EffectDescriptor_ODRV {

        /** the associated overdrive effect */
        SoXOverdrive_AudioEffect& effect;

        /** the number of channels in this effect */
        Natural channelCount;
        
        /** the gain of this effect (as a factor) */
        Real gain;

        /** the colour of this effect (between 0 and 100) defining
          * a DC offset applied to the signal*/
        Real colour;

        /** the list of incoming samples for all channels (one for
         * each) */
        AudioSampleList inputSampleList;

        /** the list of outgoing samples for all channels (one for
         * each) */
        AudioSampleList outputSampleList;

        /** the list of the previously incoming samples for all
         * channels (one for each) */
        AudioSampleList previousInputSampleList;

        /** the list of the previously outgoing samples for all
         * channels (one for each) */
        AudioSampleList previousOutputSampleList;

        /*--------------------*/
        /*--------------------*/

        /**
         * Sets up a new effect descriptor for <C>effect</C>
         *
         * @param[inout] effect  associated overdrive effect
         */
        _EffectDescriptor_ODRV (INOUT SoXOverdrive_AudioEffect& effect);

        /*--------------------*/

        /**
         * Applies the overdrive effect on multiple channel data from
         * <C>inputSampleList</C> and returns effect result in
         * <C>outputSampleList</C>.
         */
        void apply ();

        /*--------------------*/

        /**
         * Recalculates derived parameters in effect descriptor with a
         * given <C>newChannelCount</C>.
         *
         * @param[in] newChannelCount  the new channel count
         */
        void updateSettings (IN Natural newChannelCount);

        /*--------------------*/

        /**
         * Returns string representation of effect descriptor.
         *
         * @return  string representation
         */
        String toString () const;

    };

}

/*============================================================*/

using SoXPlugins::Effects::SoXOverdrive::_EffectDescriptor_ODRV;

/*------------------------*/
/* _EffectDescriptor_ODRV */
/*------------------------*/

_EffectDescriptor_ODRV::
_EffectDescriptor_ODRV (INOUT SoXOverdrive_AudioEffect& effect)
    : effect{effect},
      channelCount{2},
      gain{SoXAudioHelper::dBToLinear(0.0)},
      colour{Real{20.0} * colourFactor},
      inputSampleList{channelCount},
      outputSampleList{channelCount},
      previousInputSampleList{channelCount},
      previousOutputSampleList{channelCount}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

void _EffectDescriptor_ODRV::apply ()
{
    Logging_trace(">>");

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        const AudioSample inputSample = inputSampleList[channel];
        const AudioSample previousInputSample =
            previousInputSampleList[channel];
        const AudioSample previousOutputSample =
            previousOutputSampleList[channel];
        AudioSample newValue;

        newValue = (AudioSample) (inputSample * gain + colour);
        newValue = newValue.forceToInterval(-1.0, 1.0);
        newValue = (newValue - (newValue * newValue * newValue) / Real{3.0});
        const AudioSample outputSample =
            (newValue - previousInputSample
             + Real{0.995} * previousOutputSample);
        outputSampleList[channel] = (inputSample / Real::two
                                     + outputSample * Real{0.75});
        previousInputSampleList[channel]  = newValue;
        previousOutputSampleList[channel] = outputSample;
    }

    Logging_trace("<<");
}

/*--------------------*/

void _EffectDescriptor_ODRV::updateSettings (IN Natural newChannelCount)
{
    Logging_trace1(">>: channelCount = %s", TOSTRING(newChannelCount));

    effect.setParameterValidity(false);
    channelCount = newChannelCount;
    inputSampleList.setLength(channelCount);
    outputSampleList.setLength(channelCount);
    previousInputSampleList.setLength(channelCount);
    previousOutputSampleList.setLength(channelCount);
    effect.setParameterValidity(true);

    Logging_trace("<<");
}

/*---------------------*/

String _EffectDescriptor_ODRV::toString () const
{
    String st =
        STR::expand("gain = %1dB, colour = %2,"
                    " previousInputSampleList = %3,"
                    " previousOutputSampleList = %4",
                    TOSTRING(gain), TOSTRING(colour),
                    previousInputSampleList.toString(),
                    previousOutputSampleList.toString());

    st = STR::expand("_EffectDescriptor_ODRV(%1)", st);
    return st;
}

/*============================================================*/

/*---------------------*/
/* setup & destruction */
/*---------------------*/

SoXOverdrive_AudioEffect::SoXOverdrive_AudioEffect ()
{
    Logging_trace(">>");

    /* initialize descriptor */
    _effectDescriptor = new _EffectDescriptor_ODRV(*this);

    /* initialize parameters */
    _effectParameterMap.setKindAndValueInt(parameterName_gain,
                                           0, 100, 1, 20);
    _effectParameterMap.setKindAndValueInt(parameterName_colour,
                                           0, 100, 1, 20);
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXOverdrive_AudioEffect::~SoXOverdrive_AudioEffect ()
{
    Logging_trace(">>");
    delete (_EffectDescriptor_ODRV*) _effectDescriptor;
    Logging_trace("<<");
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
    const _EffectDescriptor_ODRV& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ODRV>(_effectDescriptor);
    return effectDescriptor.toString();
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXOverdrive_AudioEffect::name() const
{
    return "SoX Overdrive";
}

/*--------------------*/

Real SoXOverdrive_AudioEffect::tailLength () const
{
    Logging_trace(">>");

    /* use a length that ensures a decay to -100dB for a unity signal
       (with an attenuation factor of 0.995 from above) */
    Real result = Real{10000.0} / _sampleRate;

    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXParameterValueChangeKind
SoXOverdrive_AudioEffect::_setValueInternal
                              (IN String& parameterName,
                               IN String& value,
                               IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    _EffectDescriptor_ODRV& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ODRV>(_effectDescriptor);
    SoXParameterValueChangeKind result =
        SoXParameterValueChangeKind::parameterChange;

    if (parameterName == parameterName_gain) {
        const Real dBGain = STR::toReal(value);
        effectDescriptor.gain = SoXAudioHelper::dBToLinear(dBGain);
    } else if (parameterName == parameterName_colour) {
        effectDescriptor.colour = STR::toReal(value) * colourFactor;
    }

    Logging_trace1("<<: %1", SoXParameterValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXOverdrive_AudioEffect::setDefaultValues () {
    Logging_trace(">>");
    _effectParameterMap.setValue(parameterName_gain, "20");
    _effectParameterMap.setValue(parameterName_colour, "20");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void
SoXOverdrive_AudioEffect::processBlock (IN Real timePosition,
                                        INOUT AudioSampleListVector& buffer)
{
    Logging_trace1(">>: %1", TOSTRING(timePosition));

    if (_parametersAreValid) {
        SoXAudioEffect::processBlock(timePosition, buffer);

        _EffectDescriptor_ODRV& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_ODRV>(_effectDescriptor);

        if (_channelCount != effectDescriptor.channelCount) {
            /* channel count has changed => update descriptor */
            effectDescriptor.updateSettings(_channelCount);
        }

        const Natural sampleCount = buffer[0].size();
        AudioSampleList& inputSampleList  = effectDescriptor.inputSampleList;
        AudioSampleList& outputSampleList = effectDescriptor.outputSampleList;

        for (Natural i = 0;  i < sampleCount;  i++) {
            /* read input sample list at position i for all channels */
            for (Natural channel = 0;  channel < _channelCount;  channel++) {
                const AudioSampleList& inputList = buffer[channel];
                inputSampleList[channel] = inputList[i];
            }
            
            effectDescriptor.apply();

            /* write output sample list at position i onto all channels */
            for (Natural channel = 0;  channel < _channelCount;  channel++) {
                AudioSampleList& outputList = buffer[channel];
                outputList[i] = outputSampleList[channel];
            }
        }
    }

    Logging_trace("<<");
}
