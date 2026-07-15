/**
 * @file
 * The <C>SoXChorusAndEcho_AudioEffect</C> body implements the SoX
 * <B>chorus</B>, <B>echo</B> and <B>echos</B> audio effects; the
 * difference between both echos is that <B>echo</B> has parallel
 * stages all fed by the input signal while <B>echos</B> has
 * sequential stages fed by previous stages.
 *
 * @author Dr. Thomas Tensi
 * @date   2024-12
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include <cmath>
#include <utility>

#include "DelayLineList.h"
#include "Logging.h"
#include "GenericTuple.h"
#include "SoXEffectParameterMap.h"
#include "SoXChorusAndEcho_AudioEffect.h"
#include "WaveForm.h"

/*--------------------*/

using Audio::DelayLineList;
using Audio::WaveForm;
using Audio::WaveFormKind;
using BaseTypes::GenericTypes::GenericTuple;
using SoXPlugins::Helpers::SoXEffectParameterMap;
using SoXPlugins::Effects::SoXChorusAndEcho::SoXChorusAndEcho_AudioEffect;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/** abbreviation for pagedParameterName */
static const auto pagedParameterName =
    SoXEffectParameterMap::pagedParameterName;

/*====================*/

/** maximum number of stages in a chorus or echo */
constexpr int _maxStageCount = 10;

/** minimum depth value */
static const Real _minimumDepth = 0.0;

/** depth resolution */
static const Real _depthResolution = 0.1;

/** the default phase of each chorus modulation waveform */
static const Radians _defaultChorusPhase =
    Radians{atan(1) * 4 * 1.5};

/** the name of the chorus effect */
static const String _chorusEffectKind = "Chorus";

/** the name of the delay sequence effect */
static const String _delaySequenceEffectKind = "Delay Sequence";

/** the name of the tapped delay effect */
static const String _tappedDelayEffectKind = "Tapped Delay";

/** when logging is active, the number of elements shown for the delay
 * lines involved */
static const Natural _delayLineLoggingLength = 12;

/* adaptation for external test drivers */
#ifdef EFFECTS_HAVE_EXTENDED_PARAMETER_RANGES
    static const Real _minimumChorusDelay  = 0.0;
    static const Real _maximumFrequency    = 100.0;
    static const Real _frequencyResolution = 0.00001;
#else
    static const Real _minimumChorusDelay  = 20.0;
    static const Real _maximumFrequency    = 5.0;
    static const Real _frequencyResolution = 0.01;
#endif

/*============================================================*/

namespace SoXPlugins::Effects::SoXChorusAndEcho {

    /** the kind of delay represented by some plugin */
    enum struct _DelayKind {
        chorus, tappedDelay, delaySequence
    };

    /*----------------------*/

    /**
     * A <C>_DelayStageParameterData</C> object contains the parameter
     * data for a single chorus/echo stage.
     */
    struct _DelayStageParameterData {

        /** tells whether this is a chorus stage */
        Boolean _isChorusStage;
        
        /** the stage delay in seconds */
        Real _delay;

        /** the stage decay for this stage (as a factor) */
        Real _decay;

        /** the stage chorus modulation frequency in Hz (when this is
         * a chorus stage) */
        Real _frequency;

        /** the stage chorus depth for this stage in seconds (when
         * this is a chorus stage) */
        Real _depth;

        /** the stage modulation waveform kind for this stage (when
         * this is a chorus stage) */
        WaveFormKind _modulationKind;

        /** the waveform used for the modulation of this stage (when
         * this is a chorus stage) */
        WaveForm _modulationWave;

        /*--------------------*/
        /*--------------------*/

        /**
         * Returns string representation of delay stage parameter
         * data.
         *
         * @return  string representation
         */
        String toString () const;

    };

    /*====================*/

    /** fixed-length mapping from natural to echo stage data */
    using _StageIndexToStageDataMap =
        GenericTuple<_DelayStageParameterData, _maxStageCount>;

    /*====================*/

    /**
     * An <C>_EffectDescriptor_CECH</C> object is the internal
     * implementation of an chorus or echo effect descriptor type
     * where all data point input is routed to and data point output
     * is routed from.
     */
    struct _EffectDescriptor_CECH {

        /** the associated echo effect */
        SoXChorusAndEcho_AudioEffect& _effect;
        
        /** information whether effect is a chorus, an echo or echos */
        _DelayKind _kind;

        /** the input gain as a factor */
        Real _inGain;

        /** the output gain as a factor */
        Real _outGain;

        /** the time offset in seconds (for timelocking a chorus
         * effect) */
        Real _timeOffset;

        /** the number of delay lines in this multiline effect */
        Natural _stageCount;

        /** the current data point rate for this effect */
        Real _dataPointRate;
        
        /** the number of audio channels in this effect */
        Natural _channelCount;

        /** a map from stage index to the echo stage parameters
          * (like e.g. delay and decay) */
        _StageIndexToStageDataMap _indexToStageParamDataMap;

        /** the list of incoming data points for all channels (one for
         * each) */
        AudioDataPointList _inputDataPointList;

        /** the list of outgoing data points for all channels (one for
         * each) */
        AudioDataPointList _outputDataPointList;

        /** the echo delay lines as a list */
        DelayLineList _delayLineList;

        /*--------------------*/
        /*--------------------*/

        /**
         * Returns const reference to delay line for <C>channel</C> and
         *<C>stage</C>
         *
         * @param[in] channel     channel represented by delay line
         * @param[in] stageIndex  stage represented by delay line
         */
        const DelayLine& _delayLineAt (Natural channel,
                                       Natural stageIndex) const;

        /*--------------------*/

        /**
         * Returns reference to delay line for <C>channel</C> and
         *<C>stage</C>
         *
         * @param[in] channel     channel represented by delay line
         * @param[in] stageIndex  stage represented by delay line
         */
        DelayLine& _delayLineAt (Natural channel,
                                 Natural stageIndex);

        /*--------------------*/

        /**
         * Sets up a new effect descriptor for <C>effect</C>
         *
         * @param[inout] effect  associated chorus and echo effect
         */
        _EffectDescriptor_CECH (INOUT SoXChorusAndEcho_AudioEffect& effect);

        /*--------------------*/

        /**
         * Applies the echo effect on multiple channel data from
         * <C>inputDataPointList</C> and returns echo result in
         * <C>outputDataPointList</C>.
         */
        void apply ();

        /*--------------------*/

        /**
         * Returns string representation of delay internal data.
         *
         * @return  string representation
         */
        String internalState () const;

        /*--------------------*/

        /**
         * Recalculates derived parameters in effect descriptor with a
         * given <C>dataPointRate</C> and <C>channelCount</C> and
         * <C>currentTime</C> (because this effect is timelocked).
         *
         * @param[in] dataPointRate  the new data point rate for effect
         * @param[in] channelCount   the new channel count
         * @param[in] currentTime    the current play head time
         */
        void updateSettings (IN Real dataPointRate,
                             IN Natural channelCount,
                             IN Real currentTime);

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

using SoXPlugins::Effects::SoXChorusAndEcho::_DelayKind;
using SoXPlugins::Effects::SoXChorusAndEcho::_DelayStageParameterData;
using SoXPlugins::Effects::SoXChorusAndEcho::_EffectDescriptor_CECH;

/*----------------------*/
/* FORWARD DECLARATIONS */
/*----------------------*/

static String _delayKindToString (IN _DelayKind kind);

static _DelayKind _stringToDelayKind (IN String& st);

static void
_updateRealParameterRange (INOUT SoXEffectParameterMap& parameterMap,
                           IN String& parameterName,
                           IN Real lowValue,
                           IN Real highValue,
                           IN Real delta);

/*--------------------------*/
/* _DelayStageParameterData */
/*--------------------------*/

String _DelayStageParameterData::toString () const
{
    String st = STR::expand("isChorusStage = %1, delay = %2s,"
                            " decay = %3s",
                            TOSTRING(_isChorusStage), TOSTRING(_delay),
                            TOSTRING(_decay));

    if (_isChorusStage) {
        st += STR::expand(", frequency = %1Hz, depth = %2s, kind = %3",
                          TOSTRING(_frequency), TOSTRING(_depth),
                          Audio::waveFormKindToString(_modulationKind));
    }

    st = STR::expand("_DelayStageParameterData(%1)", st);
    return st;
}

/*------------------------*/
/* _EffectDescriptor_CECH */
/*------------------------*/

_EffectDescriptor_CECH::
_EffectDescriptor_CECH (INOUT SoXChorusAndEcho_AudioEffect& effect)
    : _effect{effect},
      _kind{_DelayKind::tappedDelay},
      _inGain{0.0},
      _outGain{0.0},
      _stageCount{1},
      _channelCount{0},
      _indexToStageParamDataMap{},
      _inputDataPointList{},
      _outputDataPointList{},
      _delayLineList{}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

const DelayLine&
_EffectDescriptor_CECH::_delayLineAt (Natural channel,
                                      Natural stageIndex) const
{
    Logging_trace2(">>: channel = %1, stageIndex = %2",
                   TOSTRING(channel), TOSTRING(stageIndex));

    const Natural delayLineIndex = channel * _stageCount + stageIndex;
    const DelayLine& result = _delayLineList.at(delayLineIndex);
   
    Logging_trace("<<");
    return result;
}

/*--------------------*/

DelayLine& _EffectDescriptor_CECH::_delayLineAt (Natural channel,
                                                 Natural stageIndex)
{
    return const_cast<DelayLine&>(std::as_const(*this)._delayLineAt(channel, stageIndex));
}

/*--------------------*/

void _EffectDescriptor_CECH::apply ()
{
    Logging_trace(">>");

    const Boolean isChorus          = (_kind == _DelayKind::chorus);
    const Boolean isSequentialDelay = (_kind == _DelayKind::delaySequence);

    String st;

    for (Natural channel = 0;  channel < _channelCount;  channel++) {
        AudioDataPoint inputDataPoint         = _inputDataPointList[channel];
        AudioDataPoint outputDataPoint        = inputDataPoint * _inGain;
        AudioDataPoint previousStageDataPoint = 0.0;
        const Boolean isLastChannel = (channel == _channelCount - 1);

        for (Natural stageIndex = 0;  stageIndex < _stageCount;
             stageIndex++) {
            _DelayStageParameterData& stage =
                _indexToStageParamDataMap[stageIndex];
            const Real decay = stage._decay;
            Real indexOffset;

            if (!isChorus) {
                /* no modulation, take first entry in delay line */
                indexOffset = 0.0;
            } else {
                /* access a position modulated by modulation wave */
                WaveForm& waveForm{stage._modulationWave};
                indexOffset = -waveForm.current();

                if (isLastChannel) {
                    waveForm.advance();
                }
            }

            DelayLine& delayLine = _delayLineAt(channel, stageIndex);
            AudioDataPoint stageDataPoint;
            AudioDataPoint delayedDataPoint;

            if (Logging_isActive) {
                st = STR::expand("channel = %1, stage = %2,"
                                 " indexOffset = %3, delayLine = %4",
                                 TOSTRING(channel), TOSTRING(stageIndex),
                                 TOSTRING(indexOffset),
                                 delayLine.toString(_delayLineLoggingLength));
            }

            if (delayLine.isEmpty()) {
                stageDataPoint = inputDataPoint;
            } else {
                /* the next stage either gets as input the input data
                   point or the input data point plus the current
                   stage data point */
                delayedDataPoint =
                    (!isSequentialDelay
                     ? inputDataPoint
                     : previousStageDataPoint + inputDataPoint);
                stageDataPoint = delayLine.at(indexOffset);
                delayLine.advance(delayedDataPoint);
            }

            outputDataPoint += stageDataPoint * decay;

            if (Logging_isActive) {
                Logging_trace4("--: %1, stageDataPoint = %2,"
                               " delayedDataPoint = %3,"
                               " outputDataPoint = %4",
                               st,
                               TOSTRING(stageDataPoint),
                               TOSTRING(delayedDataPoint),
                               TOSTRING(outputDataPoint));
            }

            previousStageDataPoint = stageDataPoint;
        }

        _outputDataPointList[channel] = outputDataPoint * _outGain;
    }
    
    Logging_trace("<<");
}
        
/*--------------------*/

String _EffectDescriptor_CECH::internalState () const
{
    const Boolean isChorus = (_kind == _DelayKind::chorus);
    String st;

    for (Natural stageIndex = 0;  stageIndex < _stageCount;
         stageIndex++) {
        const _DelayStageParameterData& stage =
            _indexToStageParamDataMap[stageIndex];
        Real indexOffset =
            (!isChorus ? 0.0 : stage._modulationWave.current());

        st += STR::expand(", stage = %1", TOSTRING(stageIndex));

        if (isChorus) {
            Real offsetAsTime = indexOffset * 1000.0 / _dataPointRate;
            st += STR::expand(", offset = %1smp, offsetAsTime = %2ms,"
                              " waveState = %3",
                              TOSTRING(indexOffset),
                              TOSTRING(offsetAsTime),
                              TOSTRING(stage._modulationWave.state()));
        }

        AudioDataPointList stageDataPointList;

        for (Natural channel = 0;  channel < _channelCount;  channel ++) {
            const DelayLine& delayLine = _delayLineAt(channel, stageIndex);
            const AudioDataPoint stageDataPoint =
                (delayLine.isEmpty()
                 ? AudioDataPoint{-999.0}
                 : delayLine.at(-indexOffset));
            stageDataPointList.append(stageDataPoint);
        }
        
        st += STR::expand(", stageDataPoints = %1",
                          stageDataPointList.toString(false));
    }

    return st;
}

/*--------------------*/

void _EffectDescriptor_CECH::updateSettings (IN Real dataPointRate,
                                             IN Natural channelCount,
                                             IN Real currentTime)
{
    Logging_trace2(">>: dataPointRate = %1, channelCount = %2",
                   TOSTRING(dataPointRate), TOSTRING(channelCount));

    _effect.setParameterValidity(false);
    _dataPointRate = dataPointRate;
    _channelCount = channelCount;
    _inputDataPointList.setLength(_channelCount);
    _outputDataPointList.setLength(_channelCount);

    Boolean isChorusEffect = (_kind == _DelayKind::chorus);
    Natural delayLineCount = _channelCount * _maxStageCount;
    _delayLineList.setLength(delayLineCount);
    _delayLineList.setToZero();

    for (Natural stageIndex = 0;  stageIndex < _stageCount; stageIndex++) {
        _DelayStageParameterData& stage =
            _indexToStageParamDataMap[stageIndex];
        stage._isChorusStage = isChorusEffect;

        const Real delayLineLengthInSamples =
            (stage._delay + stage._depth) * _dataPointRate;

        #ifndef EMULATE_SOX_STRICTLY
            const Real delayLineLength = delayLineLengthInSamples;
        #else
            /* use natural values for the delay line lengths */
            const Real delayLineLength = delayLineLengthInSamples.ceiling();
        #endif

        /* update modulation waveform for a chorus*/
        if (isChorusEffect && stage._frequency > 0.0001) {
            /* shift waveform by phase caused by time offset*/
            const Radians effectivePhase =
                (_defaultChorusPhase
                 + WaveForm::phaseByTime(stage._frequency,
                                         _timeOffset, currentTime));

            const Real frequency = stage._frequency;
            Real waveFormLength = (frequency < 0.001
                                   ? 1.0
                                   : _dataPointRate / stage._frequency);
            Boolean hasIntegerValues = false;

            #ifdef EMULATE_SOX_STRICTLY
                /* use natural values for the wave form and
                   indexing */
                waveFormLength   = waveFormLength.round();
                hasIntegerValues = true;
            #endif

            Real minimumModulationValue = stage._delay * _dataPointRate;
            Real maximumModulationValue = delayLineLengthInSamples;

            WaveForm& waveForm = stage._modulationWave;
            waveForm.set(waveFormLength, stage._modulationKind,
                         minimumModulationValue, maximumModulationValue,
                         effectivePhase, hasIntegerValues);
        }

        Logging_trace2("--: stageIndex = %1, delayLineLength = %2smp",
                       TOSTRING(stageIndex), TOSTRING(delayLineLength));

        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            DelayLine& delayLine = _delayLineAt(channel, stageIndex);
            delayLine.setLength(delayLineLength);
        }                
    }

    _effect.setParameterValidity(true);
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

String _EffectDescriptor_CECH::toString () const
{
    String prefix =
        STR::expand("kind = %1,"
                    " inGain = %2, outGain = %3,"
                    " timeOffset = %4,"
                    " stageCount = %5, channelCount = %6",
                    _delayKindToString(_kind),
                    TOSTRING(_inGain), TOSTRING(_outGain),
                    TOSTRING(_timeOffset),
                    TOSTRING(_stageCount), TOSTRING(_channelCount));

    String stageDataString;

    for (Natural stageIndex = 0;  stageIndex < _stageCount;  stageIndex++) {
        const _DelayStageParameterData& data =
            _indexToStageParamDataMap[stageIndex];
        Real delayLineLength = 0.0;

        if (_channelCount > 0) {
            const DelayLine& delayLine = _delayLineAt(0, stageIndex);
            delayLineLength = delayLine.length();
        }

        stageDataString +=
            ((stageIndex > 0 ? ", " : "")
             + STR::expand("stage_%1("
                           "parameterData = %2,"
                           " modulationWave = %3,"
                           " delayLineLength = %4)",
                           TOSTRING(stageIndex),
                           data.toString(),
                           data._modulationWave.toString(),
                           TOSTRING(delayLineLength)));
    }

    stageDataString = STR::expand("(%1)", stageDataString);

    String st = STR::expand("%1, stages = %2", prefix, stageDataString);
    st = STR::expand("_EffectDescriptor_CECH(%1)", st);
    return st;
}

/*============================================================*/

/** the list of effect kinds in this plugin */
static const StringList _kindList =
    StringList::fromList({_chorusEffectKind, _tappedDelayEffectKind,
                          _delaySequenceEffectKind});

/*====================*/

/** the list of parameter names in a delay stage (with units) */
static const StringList _delayStageParameterNameList =
    StringList::fromList({"Delay [ms]", "Decay",
                          "Frequency [Hz]", "Depth [ms]", "Waveform"});

/** the list of parameter values for the modulation kind */
static const StringList _modulationKindValueList =
    StringList::fromList({"Sine", "Triangle"});

/*....................*/
/* parameter names    */
/*....................*/

/** the parameter name of the effect kind (in English language) */
static const String parameterName_effectKind = "Effect Kind";

/** the parameter name for the input gain (in English language) */
static const String parameterName_inGain     = "In Gain";

/** the parameter name for the output gain (in English language) */
static const String parameterName_outGain    = "Out Gain";

/** the parameter name of the stage count (in English language) */
static const String parameterName_stageCount   = "Stage Count";

/** the parameter name of the stage index (in English language) */
static const String parameterName_stageIndex   = "Stage Index";

/** the parameter name for the time offset (in English language) */
static const String parameterName_timeOffset   = "Time Offset [s]";

/** the parameter name of the delay (in English language) */
static const String parameterName_delay =
    _delayStageParameterNameList[0];

/** the parameter name of the decay (in English language) */
static const String parameterName_decay =
    _delayStageParameterNameList[1];

/** the parameter name of the delay (in English language) */
static const String parameterName_depth =
    _delayStageParameterNameList[3];

/** the parameter name of the modulation frequency (in English language) */
static const String parameterName_frequency =
    _delayStageParameterNameList[2];

/** the parameter name of the modulation waveform (in English language) */
static const String parameterName_modulationKind =
    _delayStageParameterNameList[4];

/*--------------------*/
/* internal routines  */
/*--------------------*/

static String _delayKindToString (IN _DelayKind kind)
{
    return (kind == _DelayKind::chorus
            ? _chorusEffectKind
            : (kind == _DelayKind::tappedDelay
               ? _tappedDelayEffectKind
               : _delaySequenceEffectKind));
}

/*--------------------*/

static _DelayKind _stringToDelayKind (IN String& st)
{
    return (st == _chorusEffectKind
            ? _DelayKind::chorus
            : (st == _tappedDelayEffectKind
               ? _DelayKind::tappedDelay
               : _DelayKind::delaySequence));
}

/*--------------------*/

/**
 * Updates parameter ranges in <C>parameterMap</C> depending on
 * <C>delayKind</C>.
 *
 * @param[in]    delayKind     the new data point rate for effect
 * @param[inout] parameterMap  the effect parameter map for the
 *                             current effect
 */
static
void _updateParameterMap (IN _DelayKind delayKind,
                          INOUT SoXEffectParameterMap& parameterMap)
{
    Logging_trace2(">>: delayKind = %1, parameterMap = %2",
                   _delayKindToString(delayKind), parameterMap.toString());

    Boolean isChorusEffect = (delayKind == _DelayKind::chorus);

    /* activate or deactivate time offset for chorus */
    parameterMap.setActiveness(parameterName_timeOffset, isChorusEffect);

    /* delay value range (in milliseconds) */
    Real maximumDelay = (isChorusEffect ? 100.0 : 20000.0);
    Real minimumDelay = (isChorusEffect ?  _minimumChorusDelay : 0.0);

    for (Natural stageIndex = 0;  stageIndex < _maxStageCount;  stageIndex++) {
        const auto pagedName =
            [stageIndex] (IN String& st) {
                String result;
                result = pagedParameterName(st, Integer{stageIndex + 1});
                return result;
            };

        _updateRealParameterRange(parameterMap,
                                  pagedName(parameterName_delay),
                                  minimumDelay, maximumDelay, 0.1);
        _updateRealParameterRange(parameterMap,
                                  pagedName(parameterName_decay),
                                  0.0, 1.0, 0.00001);
        _updateRealParameterRange(parameterMap,
                                  pagedName(parameterName_frequency),
                                  0.1, _maximumFrequency,
                                  _frequencyResolution);
        _updateRealParameterRange(parameterMap,
                                  pagedName(parameterName_depth),
                                  0.0, 10.0, _depthResolution);
        /*
         * improved version with arbitrary depth:
         * _updateRealParameterRange(parameterMap,
         *                           pagedName(parameterName_depth),
         *                           _minimumDepth, maximumDelay,
         *                           _depthResolution);
        */
        parameterMap.setKindEnum(pagedName(parameterName_modulationKind),
                                 _modulationKindValueList);

        /* activate or deactivate stage parameters for chorus */
        parameterMap.setActiveness(pagedName(parameterName_frequency),
                                   isChorusEffect);
        parameterMap.setActiveness(pagedName(parameterName_depth),
                                   isChorusEffect);
        parameterMap.setActiveness(pagedName(parameterName_modulationKind),
                                   isChorusEffect);
    }

    Logging_trace1("<<: %1", parameterMap.toString());
}

/*--------------------*/

/**
 * Sets range of real parameter named <C>parameterName</C> in
 * <C>parameterMap</C> to interval [<C>lowValue</C>, <C>highValue</C>]
 * with step width <C>delta</C> keeping original value (if possible).
 *
 * @param[inout] parameterMap   the effect parameter map to be updated
 * @param[in]    parameterName  the name of the affected parameter
 * @param[in]    lowValue       the minimum allowed value for this parameter
 * @param[in]    highValue      the maximum allowed value for this parameter
 * @param[in]    delta          the delta step width
 */
static
void _updateRealParameterRange (INOUT SoXEffectParameterMap& parameterMap,
                                IN String& parameterName,
                                IN Real lowValue,
                                IN Real highValue,
                                IN Real delta)
{
    Logging_trace4(">>: name = %1,"
                   " lowValue = %2, highValue = %3, delta = %4",
                   parameterName,
                   TOSTRING(lowValue), TOSTRING(highValue), TOSTRING(delta));

    String value = parameterMap.value(parameterName);
    parameterMap.setKindAndValueReal(parameterName,
                                     lowValue, highValue, delta,
                                     STR::toReal(value));

    Logging_trace("<<");
}

/*============================================================*/

/*---------------------*/
/* setup & destruction */
/*---------------------*/

SoXChorusAndEcho_AudioEffect::SoXChorusAndEcho_AudioEffect ()
    : SoXAudioEffect{}
{
    Logging_trace(">>");

    /* initialize descriptor */
    _effectDescriptor = new _EffectDescriptor_CECH(*this);
    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
    String st;

    /* --- initialize parameters --- */
    _effectParameterMap.clear();

    /* effect kind, in gain, out gain and time offset */
    String delayKindAsString = _tappedDelayEffectKind;
    _DelayKind delayKind = _stringToDelayKind(delayKindAsString);
    _effectParameterMap.setKindAndValueEnum(parameterName_effectKind,
                                            _kindList,
                                            delayKindAsString);
    _effectParameterMap.setKindReal(parameterName_inGain,
                                    0.0, 1.0, 0.001);
    _effectParameterMap.setKindReal(parameterName_outGain,
                                    0.0, 5.0, 0.001);
    _effectParameterMap.setKindReal(parameterName_timeOffset,
                                    -8192.0, +8192.0,
                                    Real::two.power(-16.0));

    /* page count and page selection */
    st = pagedParameterName(parameterName_stageCount,
                            SoXEffectParameterMap::pageCounterPage);
    _effectParameterMap.setKindInt(st, 1, _maxStageCount, 1);

    st = pagedParameterName(parameterName_stageIndex,
                            SoXEffectParameterMap::selectorPage);
    _effectParameterMap.setKindInt(st, 1, _maxStageCount, 1);

    /* stages */
    _updateParameterMap(delayKind, _effectParameterMap);
    
    effectDescriptor._stageCount = 1;
    effectDescriptor.updateSettings(_dataPointRate, 2, _currentTimePosition);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXChorusAndEcho_AudioEffect::~SoXChorusAndEcho_AudioEffect ()
{
    Logging_trace(">>");
    delete (_EffectDescriptor_CECH*) _effectDescriptor;
    Logging_trace("<<");
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXChorusAndEcho_AudioEffect::toString () const
{
    String st = "SoXChorusAndEcho_AudioEffect(";
    st += _asRawString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXChorusAndEcho_AudioEffect::_effectDescriptorToString () const
{
    String result;

    if (_effectDescriptor == NULL) {
        result = "NULL";
    } else {
        const _EffectDescriptor_CECH& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
        result = effectDescriptor.toString();
    }

    return result;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXChorusAndEcho_AudioEffect::name () const
{
    return "SoX ChorusAndEcho";
}

/*--------------------*/

Real SoXChorusAndEcho_AudioEffect::tailLength () const
{
    const _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
    const Natural stageCount = effectDescriptor._stageCount;
    const _DelayKind kind = effectDescriptor._kind;
    const Boolean isChorusEffect    = (kind == _DelayKind::chorus);
    const Boolean isSequentialDelay = (kind == _DelayKind::delaySequence);

    Real result = 0.0;

    for (Natural stageIndex = 0;  stageIndex < stageCount;
         stageIndex++) {
        const _DelayStageParameterData& stage =
            effectDescriptor._indexToStageParamDataMap[stageIndex];
        const Real delayLineLengthInSeconds =
            stage._delay + (isChorusEffect ? stage._depth : 0.0);

        if (isSequentialDelay) {
            result += delayLineLengthInSeconds;
        } else {
            result = Real::maximum(result, delayLineLengthInSeconds);
        }
    }

    return result;
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXParameterValueChangeKind SoXChorusAndEcho_AudioEffect
::_setValueInternal (IN String& parameterName,
                     IN String& value,
                     IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalculationIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    SoXParameterValueChangeKind result =
        SoXParameterValueChangeKind::parameterChange;
    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
    const String stageCountParam = parameterName_stageCount;
    const String stageIndexParam = parameterName_stageIndex;

    if (parameterName == parameterName_effectKind) {
        _DelayKind previousKind = effectDescriptor._kind;
        _DelayKind newKind = _stringToDelayKind(value);
        _DelayKind chorusKind = _DelayKind::chorus;

        if (newKind != previousKind) {
            effectDescriptor._kind = newKind;

            if (previousKind == chorusKind || newKind == chorusKind) {
                /* when change is from or to chorus, update the
                 * parameter ranges and the complete effect */
                _updateParameterMap(newKind, _effectParameterMap);
                effectDescriptor.updateSettings(_dataPointRate,
                                                _channelCount,
                                                _currentTimePosition);
                result = SoXParameterValueChangeKind::globalChange;
            }
        }
    } else if (parameterName == parameterName_inGain) {
        effectDescriptor._inGain = STR::toReal(value);
    } else if (parameterName == parameterName_outGain) {
        effectDescriptor._outGain = STR::toReal(value);
    } else if (parameterName == parameterName_timeOffset) {
        effectDescriptor._timeOffset = STR::toReal(value);
    } else if (STR::endsWith(parameterName, stageCountParam)) {
        Logging_trace1("--: new stageCount = %1", value);
        const Natural stageCount =
            Natural::forceToInterval(STR::toNatural(value), 1, _maxStageCount);
        effectDescriptor._stageCount = stageCount;
        _effectParameterMap.setValue(stageCountParam, TOSTRING(stageCount));
        effectDescriptor.updateSettings(_dataPointRate, _channelCount,
                                        _currentTimePosition);
        result = SoXParameterValueChangeKind::pageCountChange;
    } else if (STR::endsWith(parameterName, stageIndexParam)) {
        const Natural stageIndex =
            Natural::forceToInterval(STR::toNatural(value),
                                     1, effectDescriptor._stageCount);
        _effectParameterMap.setValue(stageIndexParam, TOSTRING(stageIndex));
        result = SoXParameterValueChangeKind::pageChange;
    } else {
        Natural pageIndex;
        String effectiveParameterName;
        SoXEffectParameterMap::splitParameterName(parameterName,
                                                  effectiveParameterName,
                                                  pageIndex);
        const Natural stageIndex = pageIndex - 1;

        if (stageIndex < effectDescriptor._stageCount) {
            _DelayStageParameterData& data =
                effectDescriptor._indexToStageParamDataMap[stageIndex];

            const auto updateFromPartner =
                [this, pageIndex] (IN String& parameterName,
                                   IN Real valueInSeconds) {
                    Real valueInMilliseconds{valueInSeconds * 1000.0};
                    String longParameterName =
                        pagedParameterName(parameterName, pageIndex);
                    this->setValue(longParameterName,
                                   TOSTRING(valueInMilliseconds),
                                   true);
                };

            if (effectiveParameterName == parameterName_delay) {
                data._delay = STR::toReal(value) / 1000.0;

                if (data._delay < data._depth) {
                    updateFromPartner(effectiveParameterName, data._depth);
                }
            } else if (effectiveParameterName == parameterName_decay) {
                data._decay = STR::toReal(value);
            } else if (effectiveParameterName == parameterName_frequency) {
                data._frequency = STR::toReal(value);
            } else if (effectiveParameterName == parameterName_depth) {
                data._depth = STR::toReal(value) / 1000.0;

                if (data._depth > data._delay) {
                    updateFromPartner(effectiveParameterName, data._delay);
                }
            } else if (effectiveParameterName == parameterName_modulationKind) {
                data._modulationKind = Audio::stringToWaveFormKind(value);
            }
        }

        if (recalculationIsForced) {
            effectDescriptor.updateSettings(_dataPointRate,
                                            _channelCount,
                                            _currentTimePosition);
        }
    }

    Logging_trace1("<<: %1", SoXParameterValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXChorusAndEcho_AudioEffect::setDefaultValues ()
{
    Logging_trace(">>");

    String st;
    _channelCount = 2;

    String delayKindAsString = _tappedDelayEffectKind;
    _DelayKind delayKind = _stringToDelayKind(delayKindAsString);
    setValueViaParent(parameterName_effectKind, delayKindAsString);
    setValueViaParent(parameterName_inGain, "0.0");
    setValueViaParent(parameterName_outGain, "0.0");

    st = pagedParameterName(parameterName_stageCount,
                            SoXEffectParameterMap::pageCounterPage);
    setValueViaParent(st, "1");

    for (Natural stageIndex = 0;  stageIndex < _maxStageCount;
         stageIndex++) {
        const auto pagedName =
            [stageIndex] (const String& st) {
                String result;
                result = pagedParameterName(st, Integer{stageIndex + 1});
                return result;
            };

        setValueViaParent(pagedName(parameterName_delay),     "0.0");
        setValueViaParent(pagedName(parameterName_decay),     "0.0");

        /* dummy values for chorus */
        setValueViaParent(pagedName(parameterName_depth),     "0.0");
        setValueViaParent(pagedName(parameterName_frequency), "1.0");
        setValueViaParent(pagedName(parameterName_modulationKind), "Sine");
    }

    st = pagedParameterName(parameterName_stageIndex,
                            SoXEffectParameterMap::selectorPage);
    setValueViaParent(st, "1", true);

    /* stages */
    _updateParameterMap(delayKind, _effectParameterMap);
    
    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
    effectDescriptor._stageCount = 1;
    effectDescriptor.updateSettings(_dataPointRate, _channelCount,
                                    _currentTimePosition);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXChorusAndEcho_AudioEffect::prepareToPlay (IN Real dataPointRate)
{
    Logging_trace1(">>: %1", TOSTRING(dataPointRate));

    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);

    if (dataPointRate == _dataPointRate) {
        Logging_trace("--: no recalculation");
    } else {
        _dataPointRate = dataPointRate;
        /* echo stages have to be recalculated to change the lengths */
        effectDescriptor.updateSettings(_dataPointRate, _channelCount,
                                        _currentTimePosition);
    }

    /* reset delay lines */
    Natural stageCount = effectDescriptor._stageCount;

    for (Natural stageIndex = 0;  stageIndex < stageCount; stageIndex++) {
        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            DelayLine& delayLine =
                effectDescriptor._delayLineAt(channel, stageIndex);
            delayLine.setToZero();
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void
SoXChorusAndEcho_AudioEffect::processBlock
                                  (IN Real timePosition,
                                   INOUT AudioDataPointListVector& buffer)
{
    Logging_trace1(">>: time = %1", TOSTRING(timePosition));

    if (_parametersAreValid) {
        String st;
        Real effectiveTime = timePosition;
        const Real deltaTime = Real::one / _dataPointRate;
        _EffectDescriptor_CECH& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);

        SoXAudioEffect::processBlock(timePosition, buffer);

        if (_timePositionHasMoved
            || _channelCount != effectDescriptor._channelCount) {
            /* channel count has changed or playhead was moved ==> keep
               time synchronisation of modulation waveforms */
            effectDescriptor.updateSettings(_dataPointRate,
                                            _channelCount,
                                            _currentTimePosition);
        }

        const Natural dataPointCount = buffer[0].size();
        AudioDataPointList& inputDataPointList  =
            effectDescriptor._inputDataPointList;
        AudioDataPointList& outputDataPointList =
            effectDescriptor._outputDataPointList;

        for (Natural i = 0;  i < dataPointCount;  i++) {
            /* read input data point list at position i for all channels */
            for (Natural channel = 0;  channel < _channelCount;  channel++) {
                const AudioDataPointList& inputList = buffer[channel];
                inputDataPointList[channel] = inputList[i];
            }

            if (Logging_isActive) {
                st = STR::expand("time = %1ms, inputDataPoints = %2%3",
                                 TOSTRING(effectiveTime * 1000.0),
                                 inputDataPointList.toString(false),
                                 effectDescriptor.internalState());
            }

            effectDescriptor.apply();

            Logging_trace2("--: %1, outputDataPoints = %2",
                           st, outputDataPointList.toString(false));

            /* write output data point list at position i onto all
             * channels */
            for (Natural channel = 0;  channel < _channelCount;  channel++) {
                AudioDataPointList& outputList = buffer[channel];
                outputList[i] = outputDataPointList[channel];
            }

            effectiveTime += deltaTime;
        }
    }

    Logging_trace1("--: outputBuffer = %1", buffer.toString());

    Logging_trace("<<");
}
