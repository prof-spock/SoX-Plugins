/**
 * @file
 * The <C>SoXGain_AudioEffect</C> body implements the SoX <B>gain</B>
 * plugin amplifying or attenuating the input signal by a specific dB
 * amount.
 *
 * The original algorithms can be found in the <A
 * HREF="https://sourceforge.net/p/sox/code">SoX repository</A>
 * and the file "gain.c" by Rob Sewell.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-07
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "Logging.h"
#include "SoXAudioHelper.h"
#include "SoXGain_AudioEffect.h"

/*--------------------*/

using SoXPlugins::Effects::SoXGain::SoXGain_AudioEffect;
using SoXPlugins::Helpers::SoXAudioHelper;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/*============================================================*/

/** the parameter name of the gain parameter */
static const String parameterName_gain = "Gain [dB]";

/*============================================================*/

namespace SoXPlugins::Effects::SoXGain {

    /**
     * An <C>_EffectDescriptor_GAIN</C> object is the internal
     * implementation of a gain effect descriptor type where all data
     * point input is routed to and data point output is routed from
     * with a single gain factor parameter.
     */
    struct _EffectDescriptor_GAIN {

        /** the associated gain effect */
        SoXGain_AudioEffect& effect;
        
        /** the gain (as a real factor) */
        Real gain;

        /*--------------------*/
        /*--------------------*/

        /**
         * Sets up a new effect descriptor for <C>effect</C>
         *
         * @param[inout] effect  associated gain effect
         */
        _EffectDescriptor_GAIN (INOUT SoXGain_AudioEffect& effect);

        /*--------------------*/

        /**
         * Returns string representation of effect descriptor.
         *
         * @return  string representation
         */
        String toString() const;

    };
}

/*============================================================*/

using SoXPlugins::Effects::SoXGain::_EffectDescriptor_GAIN;

/*------------------------*/
/* _EffectDescriptor_GAIN */
/*------------------------*/

_EffectDescriptor_GAIN::
_EffectDescriptor_GAIN (INOUT SoXGain_AudioEffect& effect)
    : effect(effect),
      gain{0.0}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

String _EffectDescriptor_GAIN::toString() const
{
    String st =
        STR::expand("_EffectDescriptor_GAIN(gain = %1dB)",
                    TOSTRING(gain));
    return st;
}

/*============================================================*/

/*---------------------*/
/* setup & destruction */
/*---------------------*/

SoXGain_AudioEffect::SoXGain_AudioEffect ()
{
    Logging_trace(">>");

    /* initialize descriptor */
    _effectDescriptor = new _EffectDescriptor_GAIN(*this);

    /* initialize parameters */
    _effectParameterMap.clear();
    _effectParameterMap.setKindReal(parameterName_gain,
                                    -100.0, 100.0, 0.001);

    Logging_trace1("<<: %1", toString());
}

/*---------------------*/

SoXGain_AudioEffect::~SoXGain_AudioEffect ()
{
    Logging_trace(">>");
    delete (_EffectDescriptor_GAIN*) _effectDescriptor;
    Logging_trace("<<");
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXGain_AudioEffect::_effectDescriptorToString () const
{
    String result;

    if (_effectDescriptor == nullptr) {
        result = "NULL";
    } else {
        const _EffectDescriptor_GAIN& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_GAIN>(_effectDescriptor);

        result = effectDescriptor.toString();
    }

    return result;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXGain_AudioEffect::name () const
{
    return "SoX Gain";
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXValueChangeKind
SoXGain_AudioEffect::_setValueInternal
                         (IN String& parameterName,
                          IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    _EffectDescriptor_GAIN& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_GAIN>(_effectDescriptor);
    SoXValueChangeKind result = SoXValueChangeKind::parameterChange;

    if (parameterName == parameterName_gain) {
        const Real dBGain = STR::toReal(value);
        effectDescriptor.gain = SoXAudioHelper::dBToLinear(dBGain);
    }

    Logging_trace1("<<: %1", SoXValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXGain_AudioEffect::setDefaultValues () {
    Logging_trace(">>");
    setValue(parameterName_gain, "0");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXGain_AudioEffect::
_applyForFrame (IN AudioDataPointList& inputDataPointList,
                OUT AudioDataPointList& outputDataPointList)
{
    Logging_trace(">>");

    _EffectDescriptor_GAIN& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_GAIN>(_effectDescriptor);

    const Real gain = effectDescriptor.gain;

    for (Natural channel = 0;  channel < _channelCount; channel++) {
        const AudioDataPoint inputDataPoint = inputDataPointList[channel];
        AudioDataPoint& outputDataPoint     = outputDataPointList[channel];
        outputDataPoint = inputDataPoint * gain;
    }

    Logging_trace("<<");
}
