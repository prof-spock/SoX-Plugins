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

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/*============================================================*/

namespace SoXPlugins::Effects::SoXReverb {

    /**
     * An <C>_EffectDescriptor_RVRB</C> object is the internal
     * implementation of a reverb effect descriptor type where all
     * data point input is routed to and data point output is routed
     * from with several parameters for the freeverb algorithm (like
     * e.g.\ the room scale).
     */
    struct _EffectDescriptor_RVRB {

        /** the associated reverb effect */
        SoXReverb_AudioEffect& effect;
        
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

        /** the pre-delay (in seconds) */
        Real preDelay;

        /** the gain of the wet signal in decibels */
        Real wetDbGain;

        /** the number of channels of this effect */
        Natural channelCount;

        /** the internal reverb effect */
        _SoXReverb reverb;

        /*--------------------*/
        /*--------------------*/

        /**
         * Creates effect descriptor for <C>effect</C>.
         *
         * @param[in]  effect  associated reverb effect
         */
        _EffectDescriptor_RVRB (INOUT SoXReverb_AudioEffect& effect);

        /*--------------------*/

        /**
         * Recalculates derived parameters in reverb effect descriptor
         * for effect from its other parameters, <C>dataPointRate</C>
         * and <C>channelCount</C>.
         *
         * @param[in] dataPointRate  new data point rate
         * @param[in] channelCount   new channel count
         */
        void updateSettings (IN Real dataPointRate,
                             IN Natural channelCount);

        /*--------------------*/

        /**
         * Returns descriptor string representation.
         *
         * @return  string representation of effect
         */
        String toString () const;

    };
}

/*============================================================*/

using SoXPlugins::Effects::SoXReverb::_EffectDescriptor_RVRB;

/*--------------------*/
/*--------------------*/

_EffectDescriptor_RVRB
::_EffectDescriptor_RVRB (INOUT SoXReverb_AudioEffect& effect)
    : effect{effect},
      isWetOnly{false},
      reverberance{50.0},
      hfDamping{50.0},
      roomScale{100.0},
      stereoDepth{100.0},
      preDelay{0.0},
      wetDbGain{0.0},
      channelCount{0},
      reverb{}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

void _EffectDescriptor_RVRB::updateSettings (IN Real dataPointRate,
                                             IN Natural channelCount)
{
    Logging_trace3(">>: descriptor = %1,"
                   " dataPointRate = %2, channelCount = %3",
                   toString(),
                   TOSTRING(dataPointRate), TOSTRING(channelCount));

    effect.setParameterValidity(false);
    this->channelCount = channelCount;
    reverb.setParameters(isWetOnly,
                         reverberance,
                         hfDamping,
                         roomScale,
                         stereoDepth,
                         preDelay,
                         wetDbGain);
    reverb.resize(dataPointRate, channelCount);
    effect.setParameterValidity(true);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

String _EffectDescriptor_RVRB::toString () const
{
    String st =
        STR::expand("_EffectDescriptor_RVRB("
                    "isWetOnly = %1, reverberance = %2%,"
                    " hfDamping = %3%, roomScale = %4%,"
                    " stereoDepth = %5%, preDelay = %6s,"
                    " wetDbGain = %7dB, channelCount = %8,"
                    " reverb = %9)",
                    TOSTRING(isWetOnly), TOSTRING(reverberance),
                    TOSTRING(hfDamping), TOSTRING(roomScale),
                    TOSTRING(stereoDepth), TOSTRING(preDelay),
                    TOSTRING(wetDbGain), TOSTRING(channelCount),
                    reverb.toString());

    return st;
}

/*============================================================*/

/*....................*/
/* parameter names    */
/*....................*/

/** the possible values of a yes-no-combobox (in English language) */
static const StringList _yesNoList =
    StringList::makeBySplit("Yes/No", "/");

/*--------------------*/

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

/*============================================================*/

/*---------------------*/
/* setup & destruction */
/*---------------------*/

SoXReverb_AudioEffect::SoXReverb_AudioEffect ()
{
    Logging_trace(">>");

    /* initialize descriptor */
    _effectDescriptor = new _EffectDescriptor_RVRB{*this};

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

    _channelCount = Natural::maximum(_channelCount, 2);
    recalculate();

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
/*--------------------*/

String SoXReverb_AudioEffect::_effectDescriptorToString () const
{
    String result;

    if (_effectDescriptor == nullptr) {
        result = "NULL";
    } else {
        const _EffectDescriptor_RVRB& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);

        result = effectDescriptor.toString();
    }

    return result;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXReverb_AudioEffect::name() const
{
    return "SoX Reverb";
}

/*--------------------*/

Real SoXReverb_AudioEffect::tailLength () const
{
    Logging_trace(">>");

    const _EffectDescriptor_RVRB& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);
    Real result = (effectDescriptor.preDelay
                   + effectDescriptor.reverb.tailLength());

    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXValueChangeKind
SoXReverb_AudioEffect::_setValueInternal
                           (IN String& parameterName,
                            IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    _EffectDescriptor_RVRB& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);
    SoXValueChangeKind result = SoXValueChangeKind::parameterChange;

    if (parameterName == parameterName_isWetOnly) {
        effectDescriptor.isWetOnly = (value == "Yes");
    } else if (parameterName == parameterName_reverberance) {
        effectDescriptor.reverberance = STR::toPercentage(value);
    } else if (parameterName == parameterName_hfDamping) {
        effectDescriptor.hfDamping = STR::toPercentage(value);
    } else if (parameterName == parameterName_roomScale) {
        effectDescriptor.roomScale = STR::toPercentage(value);
    } else if (parameterName == parameterName_stereoDepth) {
        effectDescriptor.stereoDepth = STR::toPercentage(value);
    } else if (parameterName == parameterName_preDelay) {
        effectDescriptor.preDelay = STR::toReal(value) / 1000.0;
    } else if (parameterName == parameterName_wetGain) {
        effectDescriptor.wetDbGain = STR::toReal(value);
    }

    recalculate();

    Logging_trace1("<<: %1", SoXValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXReverb_AudioEffect::setDefaultValues ()
{
    Logging_trace(">>");

    Dictionary dictionary;

    dictionary.set(parameterName_isWetOnly,    "No");
    dictionary.set(parameterName_reverberance, "50");
    dictionary.set(parameterName_hfDamping,    "50");
    dictionary.set(parameterName_roomScale,    "100");
    dictionary.set(parameterName_stereoDepth,  "100");
    dictionary.set(parameterName_preDelay,     "0");
    dictionary.set(parameterName_wetGain,      "0");
    setValues(dictionary);
    recalculate()

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXReverb_AudioEffect::
_handleDataPointRateChange (IN Real dataPointRate)
{
    Logging_trace1(">>: dataPointRate = %1", TOSTRING(dataPointRate));
    recalculate();
    Logging_trace("<<");
}

/*--------------------*/

void SoXReverb_AudioEffect::
_applyForFrame (IN AudioDataPointList& inputDataPointList,
                OUT AudioDataPointList& outputDataPointList)
{
    Logging_trace(">>");

    _EffectDescriptor_RVRB& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);
    _SoXReverb& reverb = effectDescriptor.reverb;
    reverb.apply(inputDataPointList, outputDataPointList);

    Logging_trace("<<");
}

/*--------------------*/

void SoXReverb_AudioEffect::recalculate ()
{
    Logging_trace(">>");

    _EffectDescriptor_RVRB& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_RVRB>(_effectDescriptor);
    effectDescriptor.updateSettings(_dataPointRate, _channelCount);


    Logging_trace("<<");
}
