/**
 * @file
 * The <C>SoXCompander_AudioEffect</C> body implements the SoX
 * <B>compand</B> and <B>mcompand</B> audio effects; a simple
 * compander is just seen as a multiband compander with a single band.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

/*====================*/

#include "Logging.h"
#include "GenericTuple.h"
#include "RealList.h"
#include "SoXAudioHelper.h"
#include "SoXEffectParameterMap.h"
#include "SoXCompander_AudioEffect.h"
#include "SoXCompanderSupport.h"

/*====================*/

using BaseTypes::Containers::RealList;
using BaseTypes::GenericTypes::GenericTuple;
using SoXPlugins::Helpers::SoXEffectParameterMap;
using SoXPlugins::Effects::SoXCompander::SoXCompander_AudioEffect;
using SoXPlugins::Effects::SoXCompander::SoXMultibandCompander;

/** abbreviation for StringUtil::toNatural */
#define toNatural  StringUtil::toNatural

/** abbreviation for StringUtil::expand */
#define expand  StringUtil::expand

/*====================*/

/** maximum number of bands in a multiband compressor */
#define _maxBandCount  (10)

/** maximum top frequency in a compander band */
const Real _maxTopFrequency{25000.0};

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

        /*--------------------*/
        /*--------------------*/

        /**
         * Returns string representation of compander band data.
         *
         * @return  string representation
         */
        String toString () const
        {
            String st =
                expand("attack = %1s, decay = %2s, knee = %3dB,"
                       " threshold = %4dB, ratio = %5, gain = %6dB,"
                       " topFrequency = %7Hz",
                       TOSTRING(attack), TOSTRING(decay),
                       TOSTRING(knee), TOSTRING(threshold),
                       TOSTRING(ratio), TOSTRING(gain),
                       TOSTRING(topFrequency));

            st = expand("_CompanderBandParameterData(%1)", st);
            return st;
        }
    };

    /*====================*/

    /** fixed-length mapping from natural to compander data */
    using _BandIndexToCompanderDataMap =
        GenericTuple<_CompanderBandParameterData, _maxBandCount>;

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
        AudioSampleList inputSampleList;

        /** the output sample buffer of this effect */
        AudioSampleList outputSampleList;

        /*--------------------*/
        /*--------------------*/

        /**
         * Returns string representation of effect descriptor.
         *
         * @return  string representation
         */
        String toString () const
        {
            String prefix =
                expand("bandCount = %1, channelCount = %2",
                       TOSTRING(bandCount), TOSTRING(channelCount));

            String companderBandDataString;

            for (Natural bandIndex = 0;  bandIndex < bandCount;  bandIndex++) {
                const _CompanderBandParameterData& data =
                    indexToCompanderBandParamDataMap[bandIndex];
                companderBandDataString +=
                    ((bandIndex > 0 ? ", " : "")
                     + expand("bandParameters_%1 = %2",
                              TOSTRING(bandIndex), data.toString()));
            }

            companderBandDataString =
                expand("indexToCompanderBandParamDataMap = (%1)",
                       companderBandDataString);

            String st =
                expand("%1, %2, multibandCompander = %3",
                       prefix, companderBandDataString,
                       multibandCompander.toString());

            st = expand("_EffectDescriptor_CMPD(%1)", st);
            return st;
        }

    };

    /*====================*/

    /** the list of parameter names in a compander band (with units) */
    static const StringList _companderBandParameterNameList =
        StringList::fromList({"Attack [s]", "Decay [s]", "Knee [dB]",
                              "Threshold [dB]", "Ratio", "Gain [dB]",
                              "Top Frequency [Hz]"});

    /*....................*/
    /* parameter names    */
    /*....................*/

    /** the parameter name of the band count (in English language) */
    static const String parameterName_bandCount    = "Band Count";

    /** the parameter name of the band index (in English language) */
    static const String parameterName_bandIndex    = "Band Index";

    /** the parameter name of the attack (in English language) */
    static const String parameterName_attack =
        _companderBandParameterNameList[0];

    /** the parameter name of the decay (in English language) */
    static const String parameterName_decay =
        _companderBandParameterNameList[1];

    /** the parameter name of the knee (in English language) */
    static const String parameterName_dBKnee =
        _companderBandParameterNameList[2];

    /** the parameter name of the threshold (in English language) */
    static const String parameterName_dBThreshold =
        _companderBandParameterNameList[3];

    /** the parameter name of the ratio (in English language) */
    static const String parameterName_ratio =
        _companderBandParameterNameList[4];

    /** the parameter name of the gain (in English language) */
    static const String parameterName_dBGain =
        _companderBandParameterNameList[5];

    /** the parameter name of the top frequency (in English language) */
    static const String parameterName_topFrequency =
        _companderBandParameterNameList[6];

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

        _EffectDescriptor_CMPD* effectDescriptor =
            new _EffectDescriptor_CMPD{
                bandCount,  // bandCount
                0,          // channelCount
                {},         // multibandCompander
                {},         // indexToCompanderBandParamDataMap
                {},         // inputSampleList
                {}          // outputSampleList
            };

        Logging_trace1("<<: %1", effectDescriptor->toString());
        return effectDescriptor;
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
    void _updateSettings (INOUT _EffectDescriptor_CMPD& effectDescriptor,
                          IN Real sampleRate,
                          IN Natural channelCount)
    {
        Logging_trace2(">>: sampleRate = %1, channelCount = %2",
                       TOSTRING(sampleRate), TOSTRING(channelCount));

        effectDescriptor.channelCount = channelCount;
        const Natural bandCount = effectDescriptor.bandCount;
        const Natural allocatedBandCount = _maxBandCount;
        SoXMultibandCompander& compander =
            effectDescriptor.multibandCompander;
        compander.resize(allocatedBandCount, channelCount);
        compander.setEffectiveSize(bandCount);

        for (Natural bandIndex = 0;  bandIndex < allocatedBandCount;
             bandIndex++) {
            Logging_trace1("--: bandIndex = %1", TOSTRING(bandIndex));
            _CompanderBandParameterData& data =
                effectDescriptor.indexToCompanderBandParamDataMap[bandIndex];
            const bool isUnbounded = (bandIndex >= bandCount - 1);
            const Real topFrequency =
                (isUnbounded ? _maxTopFrequency : data.topFrequency);
            compander.setCompanderBandData(bandIndex, sampleRate,
                                           data.attack, data.decay,
                                           data.knee, data.threshold,
                                           data.ratio, data.gain,
                                           topFrequency);
        }

        effectDescriptor.inputSampleList.setLength(channelCount);
        effectDescriptor.outputSampleList.setLength(channelCount);

        Logging_trace1("<<: %1", effectDescriptor.toString());
    }

}

/*============================================================*/

/*---------------------*/
/* setup & destruction */
/*---------------------*/

SoXCompander_AudioEffect::SoXCompander_AudioEffect ()
    : SoXAudioEffect{}
{
    Logging_trace(">>");

    /* initialize descriptor */
    _effectDescriptor = _createEffectDescriptor(_sampleRate);
    _EffectDescriptor_CMPD& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CMPD>(_effectDescriptor);

    /* initialize parameters */
    _effectParameterMap.clear();
    _effectParameterMap.setKindInt("-2#" + parameterName_bandCount,
                                   1, _maxBandCount, 1);
    _effectParameterMap.setKindInt("-1#" + parameterName_bandIndex,
                                   1, _maxBandCount, 1);

    for (Natural bandIndex = 0;  bandIndex < _maxBandCount;  bandIndex++) {
        const auto pagedName =
            [bandIndex] (String st) {
                String result;
                result = SoXEffectParameterMap
                              ::pagedParameterName(st, bandIndex + 1);
                return result;
            };

        _effectParameterMap.setKindReal(pagedName(parameterName_attack),
                                        0.001, 1.0, 0.001);
        _effectParameterMap.setKindReal(pagedName(parameterName_decay),
                                        0.001, 1.0, 0.001);
        _effectParameterMap.setKindReal(pagedName(parameterName_dBKnee),
                                        0.0, 20.0, 0.01);
        _effectParameterMap.setKindReal(pagedName(parameterName_dBThreshold),
                                        -128.0, 0.0, 0.1);
        _effectParameterMap.setKindReal(pagedName(parameterName_ratio),
                                        0.001, 1000.0, 0.001);
        _effectParameterMap.setKindReal(pagedName(parameterName_dBGain),
                                        -20.0, 20.0, 0.01);
        _effectParameterMap.setKindReal(pagedName(parameterName_topFrequency),
                                        0.1, _maxTopFrequency, 0.1);
    }

    effectDescriptor.bandCount = 1;
    _updateSettings(effectDescriptor, _sampleRate, 2);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXCompander_AudioEffect::~SoXCompander_AudioEffect ()
{
    Logging_trace(">>");
    delete (_EffectDescriptor_CMPD*) _effectDescriptor;
    Logging_trace("<<");
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
    _EffectDescriptor_CMPD& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CMPD>(_effectDescriptor);
    return effectDescriptor.toString();
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

SoXParameterValueChangeKind SoXCompander_AudioEffect
::_setValueInternal (IN String& parameterName,
                     IN String& value,
                     IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    SoXParameterValueChangeKind result =
        SoXParameterValueChangeKind::parameterChange;
    _EffectDescriptor_CMPD& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CMPD>(_effectDescriptor);
    const String bandCountParam = parameterName_bandCount;
    const String bandIndexParam = parameterName_bandIndex;

    if (StringUtil::endsWith(parameterName, bandCountParam)) {
        Logging_trace1("--: new bandCount = %1", value);
        const Natural bandCount =
            Natural::forceToInterval(toNatural(value), 1, _maxBandCount);
        effectDescriptor.bandCount = bandCount;
        effectDescriptor.multibandCompander.setEffectiveSize(bandCount);
        _effectParameterMap.setValue(bandCountParam, TOSTRING(bandCount));
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
        result = SoXParameterValueChangeKind::pageCountChange;
    } else if (StringUtil::endsWith(parameterName, bandIndexParam)) {
        const Natural bandIndex =
            Natural::forceToInterval(toNatural(value),
                                     1, effectDescriptor.bandCount);
        _effectParameterMap.setValue(bandIndexParam, TOSTRING(bandIndex));
        result = SoXParameterValueChangeKind::pageChange;
    } else {
        Natural pageIndex;
        String effectiveParameterName;
        SoXEffectParameterMap::splitParameterName(parameterName,
                                                 effectiveParameterName,
                                                 pageIndex);
        const Natural bandIndex = pageIndex - 1;

        if (bandIndex < effectDescriptor.bandCount) {
            _CompanderBandParameterData& data =
                effectDescriptor.indexToCompanderBandParamDataMap[bandIndex];

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

            if (recalculationIsForced) {
                _updateSettings(effectDescriptor, _sampleRate, _channelCount);
            }
        }
    }

    Logging_trace1("<<: %1", SoXParameterValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXCompander_AudioEffect::setDefaultValues ()
{
    Logging_trace(">>");

    _channelCount = 2;

    _effectParameterMap.setValue("0#" + parameterName_bandCount, "1");
    _effectParameterMap.setValue("-1#" + parameterName_bandIndex, "1");

    for (Natural bandIndex = 0;  bandIndex < _maxBandCount;
         bandIndex++) {
        const auto pagedName =
            [bandIndex] (String st) {
                String result;
                result = SoXEffectParameterMap
                              ::pagedParameterName(st, bandIndex + 1);
                return result;
            };

        _effectParameterMap.setValue(pagedName(parameterName_attack),
                                     "0.02");
        _effectParameterMap.setValue(pagedName(parameterName_decay),
                                     "0.15");
        _effectParameterMap.setValue(pagedName(parameterName_dBKnee),
                                     "6");
        _effectParameterMap.setValue(pagedName(parameterName_dBThreshold),
                                     "-20");
        _effectParameterMap.setValue(pagedName(parameterName_ratio),
                                     "2");
        _effectParameterMap.setValue(pagedName(parameterName_dBGain),
                                     "5");
        const Real topFrequency =
            (_maxTopFrequency * (bandIndex + Natural{1}))
              / Real{Natural{_maxBandCount}};
        _effectParameterMap.setValue(pagedName(parameterName_topFrequency),
                                     TOSTRING(topFrequency));
    }

    _EffectDescriptor_CMPD& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CMPD>(_effectDescriptor);
    effectDescriptor.bandCount = 1;
    _updateSettings(effectDescriptor, _sampleRate, _channelCount);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXCompander_AudioEffect::prepareToPlay (IN Real sampleRate)
{
    Logging_trace1(">>: %1", TOSTRING(sampleRate));

    if (sampleRate == _sampleRate) {
        Logging_trace("--: no recalculation");
    } else {
        _EffectDescriptor_CMPD& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_CMPD>(_effectDescriptor);
        _sampleRate = sampleRate;
        // compander has to be recalculated
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
    }

    Logging_trace("<<");
}

/*--------------------*/

void
SoXCompander_AudioEffect::processBlock
                              (IN Real timePosition,
                               INOUT AudioSampleListVector& buffer) {
    Logging_trace1(">>: time = %1", TOSTRING(timePosition));

    SoXAudioEffect::processBlock(timePosition, buffer);

    _EffectDescriptor_CMPD& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CMPD>(_effectDescriptor);

    if (_channelCount != effectDescriptor.channelCount) {
        effectDescriptor.channelCount = _channelCount;
        _updateSettings(effectDescriptor, _sampleRate, _channelCount);
    }

    const Natural sampleCount = buffer[0].size();
    SoXMultibandCompander& compander = effectDescriptor.multibandCompander;
    AudioSampleList& inputSampleList  =
        effectDescriptor.inputSampleList;
    AudioSampleList& outputSampleList =
        effectDescriptor.outputSampleList;

    for (Natural i = 0;  i < sampleCount;  i++) {
        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            const AudioSampleList& inputList = buffer[channel];
            inputSampleList[channel] = inputList[i];
        }

        compander.apply(inputSampleList, outputSampleList);

        // write output sample list onto all channels
        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            AudioSampleList& outputList = buffer[channel];
            outputList[i] = outputSampleList[channel];
        }
    }

    Logging_trace("<<");
}
