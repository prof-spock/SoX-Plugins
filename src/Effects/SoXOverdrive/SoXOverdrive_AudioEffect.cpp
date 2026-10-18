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
     * where all data point input is routed to and data point output is routed
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

        /** the list of the previously incoming data points for all
         * channels (one for each) */
        AudioDataPointList previousInputDataPointList;

        /** the list of the previously outgoing data points for all
         * channels (one for each) */
        AudioDataPointList previousOutputDataPointList;

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
         * <C>inputDataPointList</C> and returns effect result in
         * <C>outputDataPointList</C>.
         *
         * @param [in]  inputDataPointList   list of input data points
         *                                   for each channel
         * @param [out] outputDataPointList  list of output data points
         *                                   for each channel
         */
        void apply (IN AudioDataPointList& inputDataPointList,
                    OUT AudioDataPointList& outputDataPointList);

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
      previousInputDataPointList{channelCount},
      previousOutputDataPointList{channelCount}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

void
_EffectDescriptor_ODRV::apply (IN AudioDataPointList& inputDataPointList,
                               OUT AudioDataPointList& outputDataPointList)
{
    Logging_trace(">>");

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        const AudioDataPoint inputDataPoint = inputDataPointList[channel];
        const AudioDataPoint previousInputDataPoint =
            previousInputDataPointList[channel];
        const AudioDataPoint previousOutputDataPoint =
            previousOutputDataPointList[channel];
        AudioDataPoint newValue;

        newValue = (AudioDataPoint) (inputDataPoint * gain + colour);
        newValue = newValue.forceToInterval(-1.0, 1.0);
        newValue = (newValue - (newValue * newValue * newValue) / Real{3.0});
        const AudioDataPoint outputDataPoint =
            (newValue - previousInputDataPoint
             + Real{0.995} * previousOutputDataPoint);
        outputDataPointList[channel] = (inputDataPoint / Real::two
                                        + outputDataPoint * Real{0.75});
        previousInputDataPointList[channel]  = newValue;
        previousOutputDataPointList[channel] = outputDataPoint;
    }

    Logging_trace("<<");
}

/*--------------------*/

void _EffectDescriptor_ODRV::updateSettings (IN Natural newChannelCount)
{
    Logging_trace1(">>: channelCount = %1", TOSTRING(newChannelCount));

    effect.setParameterValidity(false);
    channelCount = newChannelCount;
    previousInputDataPointList.setLength(channelCount);
    previousOutputDataPointList.setLength(channelCount);
    previousInputDataPointList.setToZero();
    previousOutputDataPointList.setToZero();
    effect.setParameterValidity(true);

    Logging_trace("<<");
}

/*---------------------*/

String _EffectDescriptor_ODRV::toString () const
{
    String st =
        STR::expand("gain = %1dB, colour = %2,"
                    " previousInputDataPointList = %3,"
                    " previousOutputDataPointList = %4",
                    TOSTRING(gain), TOSTRING(colour),
                    previousInputDataPointList.toString(),
                    previousOutputDataPointList.toString());

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
    setDefaultValues();

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
/*--------------------*/

String SoXOverdrive_AudioEffect::_effectDescriptorToString () const
{
    String result;

    if (_effectDescriptor == nullptr) {
        result = "NULL";
    } else {
        const _EffectDescriptor_ODRV& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_ODRV>(_effectDescriptor);

        result = effectDescriptor.toString();
    }

    return result;
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
    Real result = Real{10000.0} / _dataPointRate;

    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXValueChangeKind
SoXOverdrive_AudioEffect::_setValueInternal
                              (IN String& parameterName,
                               IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    _EffectDescriptor_ODRV& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ODRV>(_effectDescriptor);
    SoXValueChangeKind result = SoXValueChangeKind::parameterChange;

    if (parameterName == parameterName_gain) {
        const Real dBGain = STR::toReal(value);
        effectDescriptor.gain = SoXAudioHelper::dBToLinear(dBGain);
    } else if (parameterName == parameterName_colour) {
        effectDescriptor.colour = STR::toReal(value) * colourFactor;
    }

    Logging_trace1("<<: %1", SoXValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXOverdrive_AudioEffect::setDefaultValues () {
    Logging_trace(">>");

    setValue(parameterName_gain, "20");
    setValue(parameterName_colour, "20");
    recalculate();

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXOverdrive_AudioEffect::
_applyForFrame (IN AudioDataPointList& inputDataPointList,
                OUT AudioDataPointList& outputDataPointList)
{
    Logging_trace(">>");

    _EffectDescriptor_ODRV& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ODRV>(_effectDescriptor);
    effectDescriptor.apply(inputDataPointList, outputDataPointList);

    Logging_trace("<<");
}

/*--------------------*/

void SoXOverdrive_AudioEffect::recalculate ()
{
    Logging_trace(">>");

    _EffectDescriptor_ODRV& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ODRV>(_effectDescriptor);
    effectDescriptor.updateSettings(_channelCount);

    Logging_trace("<<");
}
