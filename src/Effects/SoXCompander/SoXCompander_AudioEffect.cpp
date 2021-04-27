/**
 * @file
 * The <C>SoXCompander_AudioEffect</C> body implements the SoX
 * <B>compand</B> and <B>mcompand</B> audio effects; a simple
 * compander is just seen as a multiband compander with a single band.
 *
 * @author Dr. Thomas Tensi
 */

/*====================*/

#include "Logging.h"
#include "Integer.h"
#include "Natural.h"
#include "Real.h"
#include "GenericTuple.h"
#include "RealList.h"
#include "SoXAudioHelper.h"
#include "SoXAudioParameterMap.h"
#include "SoXAudioValueChangeKind.h"
#include "SoXCompander_AudioEffect.h"
#include "SoXCompanderSupport.h"
#include "StringUtil.h"

/*====================*/

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;

using SoXPlugins::BaseTypes::Primitives::Integer;
using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::BaseTypes::Containers::GenericTuple;
using SoXPlugins::BaseTypes::Containers::RealList;
using SoXPlugins::BaseTypes::Containers::StringList;
using SoXPlugins::CommonAudio::SoXAudioParameterMap;
using SoXPlugins::CommonAudio::SoXAudioValueChangeKind;
using SoXPlugins::Effects::SoXCompander::SoXCompander_AudioEffect;
using SoXPlugins::Effects::SoXCompander::SoXMultibandCompander;

using StringUtil::toNatural;

/*====================*/

/** maximum number of bands in a multiband compressor */
constexpr int _maxBandCount = 10;

/** maximum top frequency in a compander band */
constexpr double _maxTopFrequency = 25000.0;

/** function to adjust a bandindex to range of 1 to maximum band count */
#define _adjustBandIndex(bandIndex) \
    Natural::maximum(1, Natural::minimum(effectDescriptor->bandCount,\
                                         bandIndex))

/*--------------------*/

namespace SoXPlugins::Effects::SoXCompander {

    /**
     * A <C>_CompanderBandParameterData</C> object contains the
     * parameter data for a single compander band.
     */
    struct _CompanderBandParameterData {

        /** the compander attack in seconds */
        Real attack;

        /** the compander decay in seconds */
        Real decay;

        /** the compander knee in decibels */
        Real knee;

        /** the compander threshold in decibels */
        Real threshold;

        /** the compander ratio as a real factor */
        Real ratio;

        /** the compander gain in decibels */
        Real gain;

        /** the compander top crossover frequency in Hz */
        Real topFrequency;
    };

    /*====================*/

    /** fixed-length mapping from natural to compander data */
    typedef GenericTuple<_CompanderBandParameterData, _maxBandCount>
            _BandIndexToCompanderDataMap;

    /*====================*/

    /**
     * An <C>_EffectDescriptor_CMPD</C> object is the internal
     * implementation of a multiband compander effect descriptor type
     * where all sample input is routed to and sample output is routed
     * from.
     */
    struct _EffectDescriptor_CMPD {

        /** the number of bands in this multiband compander */
        Natural bandCount;

        /** the number of audio channels in this multiband compander */
        Natural channelCount;

        /** the multiband compander object itself */
        SoXMultibandCompander multibandCompander;

        /** a map from band index to the compander band parameters
          * (like e.g. attack) */
        _BandIndexToCompanderDataMap indexToCompanderBandParamDataMap;

        /** the input sample buffer of this effect */
        SoXAudioSampleList inputSampleList;

        /** the output sample buffer of this effect */
        SoXAudioSampleList outputSampleList;
    };

    /*====================*/

    /** the list of parameter names in a compander band (with units) */
    static const StringList _nameList =
        StringList::makeBySplit("Attack [s]/Decay [s]/Knee [dB]"
                                "/Threshold [dB]/Ratio/Gain [dB]"
                                "/Top Frequency [Hz]", "/");

    /*....................*/
    /* parameter names    */
    /*....................*/

    /** the parameter name of the band count (in English language) */
    static const String parameterName_bandCount    = "Band Count";

    /** the parameter name of the band index (in English language) */
    static const String parameterName_bandIndex    = "Band Index";

    /** the parameter name of the attack (in English language) */
    static const String parameterName_attack       = _nameList[0];

    /** the parameter name of the decay (in English language) */
    static const String parameterName_decay        = _nameList[1];

    /** the parameter name of the knee (in English language) */
    static const String parameterName_dBKnee       = _nameList[2];

    /** the parameter name of the threshold (in English language) */
    static const String parameterName_dBThreshold  = _nameList[3];

    /** the parameter name of the ratio (in English language) */
    static const String parameterName_ratio        = _nameList[4];

    /** the parameter name of the gain (in English language) */
    static const String parameterName_dBGain       = _nameList[5];

    /** the parameter name of the top frequency (in English language) */
    static const String parameterName_topFrequency = _nameList[6];

    /*--------------------*/
    /* internal routines  */
    /*--------------------*/

    /**
     * Sets up a new effect descriptor based on <C>sampleRate</C>
     *
     * @return new compander effect descriptor
     */
    static _EffectDescriptor_CMPD* _createEffectDescriptor (Real)
    {
        Logging_trace(">>");
        const Natural bandCount = 1;

        _EffectDescriptor_CMPD* _effectDescriptor =
            new _EffectDescriptor_CMPD{
                bandCount,  // bandCount
                0,          // channelCount
                {},         // multibandCompander
                {},         // indexToCompanderBandParamDataMap
                {},         // inputSampleList
                {}          // outputSampleList
            };

        Logging_trace("<<");
        return _effectDescriptor;
    }

    /*--------------------*/

    /**
     * Returns string representation of <C>effectDescriptor</C>.
     *
     * @param[in] effectDescriptor  effect descriptor to be converted
     * @return  string representation
     */
    String _descriptorToString (IN _EffectDescriptor_CMPD* effectDescriptor)
    {
        const Natural bandCount = effectDescriptor->bandCount;

        String st = "_EffectDescriptor_CMPD(";
        st += "bandCount = " + TOSTRING(bandCount);
        st += ", channelCount = " + TOSTRING(effectDescriptor->channelCount);
        st += ", indexToCompanderBandParamDataMap = (";

        for (Natural bandIndex = 0;  bandIndex < bandCount;  bandIndex++) {
            const _CompanderBandParameterData& data =
                effectDescriptor->indexToCompanderBandParamDataMap[bandIndex];
            st += (bandIndex > 0 ? ", " : "");
            st += "bandParameters_" + TOSTRING(bandIndex) + " = (";
            st += "attack = " + TOSTRING(data.attack) + "s";
            st += ", decay = " + TOSTRING(data.decay) + "s";
            st += ", knee = " + TOSTRING(data.knee) + "dB";
            st += ", threshold = " + TOSTRING(data.threshold) + "dB";
            st += ", ratio = " + TOSTRING(data.ratio);
            st += ", gain = " + TOSTRING(data.gain) + "dB";
            st += ", topFrequency = " + TOSTRING(data.topFrequency) + "Hz";
            st += ")";
        }

        st += ")";
        st += (", multibandCompander = "
               + effectDescriptor->multibandCompander.toString());
        st += ")";

        return st;
    }

    /*--------------------*/

    /**
     * Sets up all audio editor parameters in <C>parameterMap</C> for
     * effect.
     *
     * @param parameterMap  the map of audio parameters for compander
     *                      effect
     */
    static
    void _initializeAllParameters (INOUT SoXAudioParameterMap& parameterMap)
    {
        Logging_trace(">>");

        parameterMap.setKindInt("-2#" + parameterName_bandCount,
                                Integer{1}, Integer{_maxBandCount}, Integer{1});
        parameterMap.setKindInt("-1#" + parameterName_bandIndex,
                                Integer{1}, Integer{_maxBandCount}, Integer{1});

        for (Natural bandIndex = 0;  bandIndex < _maxBandCount;  bandIndex++) {
            const auto pagedName =
                [bandIndex] (String st) {
                    String result;
                    result = SoXAudioParameterMap
                                  ::pagedParameterName(st, bandIndex + 1);
                    return result;
                };

            parameterMap.setKindReal(pagedName(parameterName_attack),
                                     0.001, 1, 0.001);
            parameterMap.setKindReal(pagedName(parameterName_decay),
                                     0.001, 1, 0.001);
            parameterMap.setKindReal(pagedName(parameterName_dBKnee),
                                     0, 20, 0.01);
            parameterMap.setKindReal(pagedName(parameterName_dBThreshold),
                                     -128, 0, 0.1);
            parameterMap.setKindReal(pagedName(parameterName_ratio),
                                     0.001, 1000, 0.001);
            parameterMap.setKindReal(pagedName(parameterName_dBGain),
                                     -20, 20, 0.01);
            parameterMap.setKindReal(pagedName(parameterName_topFrequency),
                                     0, _maxTopFrequency, 1);
        }

        Logging_trace1("<<: %1", parameterMap.toString());
    }

    /*--------------------*/

    /**
     * Recalculates derived parameters in <C>effectDescriptor</C> with a
     * given <C>sampleRate</C> and <C>channelCount</C>.
     *
     * @param[inout] effectDescriptor  the compander effect descriptor
     * @param[in] sampleRate           the new sample rate for effect
     * @param[in] channelCount         the new channel count
     */
    static
    void _updateSettings (INOUT _EffectDescriptor_CMPD* effectDescriptor,
                          IN Real sampleRate,
                          IN Natural channelCount)
    {
        Logging_trace2(">>: sampleRate = %1, channelCount = %2",
                       TOSTRING(sampleRate), TOSTRING(channelCount));

        effectDescriptor->channelCount = channelCount;
        const Natural bandCount = effectDescriptor->bandCount;
        const Natural allocatedBandCount = _maxBandCount;
        SoXMultibandCompander& compander =
            effectDescriptor->multibandCompander;
        compander.resize(allocatedBandCount, channelCount);
        compander.setEffectiveSize(bandCount);

        for (Natural bandIndex = 0;  bandIndex < allocatedBandCount;
             bandIndex++) {
            Logging_trace1("--: bandIndex = %1", TOSTRING(bandIndex));
            _CompanderBandParameterData& data =
                effectDescriptor->indexToCompanderBandParamDataMap[bandIndex];
            const bool isUnbounded = (bandIndex >= bandCount - 1);
            const Real topFrequency =
                (isUnbounded ? _maxTopFrequency : data.topFrequency);
            compander.setCompanderBandData(bandIndex, sampleRate,
                                           data.attack, data.decay, data.knee,
                                           data.threshold, data.ratio, data.gain,
                                           topFrequency);
        }

        effectDescriptor->inputSampleList.setLength(channelCount);
        effectDescriptor->outputSampleList.setLength(channelCount);

        Logging_trace1("<<: %1", _descriptorToString(effectDescriptor));
    }

};

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXCompander_AudioEffect::SoXCompander_AudioEffect ()
    : SoXAudioEffect{}
{
    Logging_trace(">>");
    _EffectDescriptor_CMPD* effectDescriptor =
        _createEffectDescriptor(_sampleRate);
    _effectDescriptor = effectDescriptor;
    _initializeAllParameters(_audioParameterMap);
    effectDescriptor->bandCount = 1;
    _updateSettings(effectDescriptor, _sampleRate, 2);
    _audioParameterMap.changeActivenessByPage(effectDescriptor->bandCount);
    Logging_trace1("<<: %1", toString());
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXCompander_AudioEffect::toString () const
{
    String st = "SoXCompander_AudioEffect(";
    st += _asRawString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXCompander_AudioEffect::_effectDescriptorToString () const
{
    _EffectDescriptor_CMPD* effectDescriptor =
        static_cast<_EffectDescriptor_CMPD*>(_effectDescriptor);
    return _descriptorToString(effectDescriptor);
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXCompander_AudioEffect::name() const
{
    return "SoX Compander";
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXAudioValueChangeKind SoXCompander_AudioEffect
::_setValueInternal (IN String& parameterName,
                     IN String& value,
                     IN bool recalculationIsSuppressed)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsSuppressed = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsSuppressed));

    SoXAudioValueChangeKind result =
        SoXAudioValueChangeKind::parameterChange;
    _EffectDescriptor_CMPD* effectDescriptor =
        static_cast<_EffectDescriptor_CMPD*>(_effectDescriptor);
    const String bandCountParam = parameterName_bandCount;
    const String bandIndexParam = parameterName_bandIndex;

    if (StringUtil::endsWith(parameterName, bandCountParam)) {
        Logging_trace1("--: new bandCount = %1", value);
        const Natural bandCount =
            Natural::minimum(Natural::maximum(toNatural(value), 1),
                             _maxBandCount);
        effectDescriptor->bandCount = bandCount;
        effectDescriptor->multibandCompander.setEffectiveSize(bandCount);
        _audioParameterMap.setValue(bandCountParam, TOSTRING(bandCount));
        _audioParameterMap.changeActivenessByPage(bandCount);
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
        result = SoXAudioValueChangeKind::pageCountChange;
    } else if (StringUtil::endsWith(parameterName, bandIndexParam)) {
        const Natural bandIndex = _adjustBandIndex(toNatural(value));
        _audioParameterMap.setValue(bandIndexParam, TOSTRING(bandIndex));
        result = SoXAudioValueChangeKind::pageChange;
    } else {
        Natural pageIndex;
        String effectiveParameterName;
        SoXAudioParameterMap::splitParameterName(parameterName,
                                                 effectiveParameterName,
                                                 pageIndex);
        const Natural bandIndex = pageIndex - 1;

        if (bandIndex < effectDescriptor->bandCount) {
            _CompanderBandParameterData& data =
                effectDescriptor->indexToCompanderBandParamDataMap[bandIndex];

            if (effectiveParameterName == parameterName_attack) {
                data.attack = StringUtil::toReal(value);
            } else if (effectiveParameterName == parameterName_decay) {
                data.decay = StringUtil::toReal(value);
            } else if (effectiveParameterName == parameterName_dBKnee) {
                data.knee = StringUtil::toReal(value);
            } else if (effectiveParameterName == parameterName_dBThreshold) {
                data.threshold = StringUtil::toReal(value);
            } else if (effectiveParameterName == parameterName_ratio) {
                data.ratio = StringUtil::toReal(value);
            } else if (effectiveParameterName == parameterName_dBGain) {
                data.gain = StringUtil::toReal(value);
            } else if (effectiveParameterName == parameterName_topFrequency) {
                data.topFrequency = StringUtil::toReal(value);
            }

            if (!recalculationIsSuppressed) {
                _updateSettings(effectDescriptor, _sampleRate, _channelCount);
            }
        }
    }

    Logging_trace1("<<: %1", SoXAudioValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXCompander_AudioEffect::setDefaultValues ()
{
    Logging_trace(">>");

    _channelCount = 2;

    _audioParameterMap.setValue("0#" + parameterName_bandCount, "1");
    _audioParameterMap.setValue("-1#" + parameterName_bandIndex, "1");

    for (Natural bandIndex = 0;  bandIndex < _maxBandCount;
         bandIndex++) {
        const auto pagedName =
            [bandIndex] (String st) {
                String result;
                result = SoXAudioParameterMap
                              ::pagedParameterName(st, bandIndex + 1);
                return result;
            };

        _audioParameterMap.setValue(pagedName(parameterName_attack),
                                    "0.02");
        _audioParameterMap.setValue(pagedName(parameterName_decay),
                                    "0.15");
        _audioParameterMap.setValue(pagedName(parameterName_dBKnee),
                                    "6");
        _audioParameterMap.setValue(pagedName(parameterName_dBThreshold),
                                    "-20");
        _audioParameterMap.setValue(pagedName(parameterName_ratio),
                                    "2");
        _audioParameterMap.setValue(pagedName(parameterName_dBGain),
                                    "5");
        const Real topFrequency =
            (_maxTopFrequency * ((size_t) bandIndex + 1)) / _maxBandCount;
        _audioParameterMap.setValue(pagedName(parameterName_topFrequency),
                                    TOSTRING(topFrequency));
    }

    _EffectDescriptor_CMPD* effectDescriptor =
        static_cast<_EffectDescriptor_CMPD*>(_effectDescriptor);
    effectDescriptor->bandCount = 1;
    _updateSettings(effectDescriptor, _sampleRate, _channelCount);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXCompander_AudioEffect::prepareToPlay (Real sampleRate)
{
    Logging_trace1(">>: %1", TOSTRING(sampleRate));

    if (sampleRate == _sampleRate) {
        Logging_trace("--: no recalculation");
    } else {
        _EffectDescriptor_CMPD* effectDescriptor =
            static_cast<_EffectDescriptor_CMPD*>(_effectDescriptor);
        _sampleRate = sampleRate;
        // compander has to be recalculated
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
    }

    Logging_trace("<<");
}

/*--------------------*/

void
SoXCompander_AudioEffect::processBlock (Real timePosition,
                                        SoXAudioSampleBuffer& buffer) {
    SoXAudioEffect::processBlock(timePosition, buffer);

    _EffectDescriptor_CMPD* effectDescriptor =
        static_cast<_EffectDescriptor_CMPD*>(_effectDescriptor);

    if (_channelCount != effectDescriptor->channelCount) {
        effectDescriptor->channelCount = _channelCount;
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
    }

    const Natural sampleCount = buffer[0].size();
    SoXMultibandCompander& compander = effectDescriptor->multibandCompander;
    SoXAudioSampleList& inputSampleList  =
        effectDescriptor->inputSampleList;
    SoXAudioSampleList& outputSampleList =
        effectDescriptor->outputSampleList;

    for (Natural i = 0;  i < sampleCount;  i++) {
        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            const SoXAudioSampleList& inputList = buffer[channel];
            inputSampleList[channel] = inputList[i];
        }

        compander.apply(inputSampleList, outputSampleList);

        // write output sample list onto all channels
        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            SoXAudioSampleList& outputList = buffer[channel];
            outputList[i] = outputSampleList[channel];
        }
    }
}
