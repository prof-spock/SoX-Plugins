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

#include "AudioSampleRingBufferVector.h"
#include "Logging.h"
#include "GenericTuple.h"
#include "RealList.h"
#include "SoXEffectParameterMap.h"
#include "SoXChorusAndEcho_AudioEffect.h"
#include "WaveForm.h"

/*--------------------*/

using Audio::AudioSampleRingBufferVector;
using Audio::WaveForm;
using Audio::WaveFormKind;
using BaseTypes::Containers::RealList;
using BaseTypes::GenericTypes::GenericTuple;
using SoXPlugins::Helpers::SoXEffectParameterMap;
using SoXPlugins::Effects::SoXChorusAndEcho::SoXChorusAndEcho_AudioEffect;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/** abbreviation for pagedParameterName */
#define pagedParameterName SoXEffectParameterMap::pagedParameterName

/*====================*/

/** maximum number of stages in a chorus or echo */
#define _maxStageCount  (10)

/** minimum depth value */
static const Real _minimumDepth = 0.0;

/** depth resolution */
static const Real _depthResolution = 0.1;

/** the default phase of each chorus modulation waveform */
#define _defaultChorusPhase Radians{Real::pi / 2.0}

/** the name of the chorus effect */
static const String _chorusEffectKind = "Chorus";

/** the name of the delay sequence effect */
static const String _delaySequenceEffectKind = "Delay Sequence";

/** the name of the tapped delay effect */
static const String _tappedDelayEffectKind = "Tapped Delay";

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

        /** the stage chorus modulation frequency in Hz (when this is a
         * chorus stage) */
        Real _frequency;

        /** the stage chorus depth for this stage in seconds (when this is a
         * chorus stage) */
        Real _depth;

        /** the stage modulation waveform kind for this stage (when this
         * is a chorus stage) */
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
     * An <C>_EffectDescriptor_ECHO</C> object is the internal
     * implementation of an chorus or echo effect descriptor type
     * where all sample input is routed to and sample output is routed
     * from.
     */
    struct _EffectDescriptor_ECHO {

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

        /** the number of delay lines in this multiline echo */
        Natural _stageCount;

        /** the current sample rate for this effect */
        Real _sampleRate;
        
        /** the number of audio channels in this echo */
        Natural _channelCount;

        /** a map from stage index to the echo stage parameters
          * (like e.g. delay and decay) */
        _StageIndexToStageDataMap _indexToStageParamDataMap;

        /** the list of incoming samples for all channels (one for each) */
        AudioSampleList _inputSampleList;

        /** the list of outgoing samples for all channels (one for each) */
        AudioSampleList _outputSampleList;

        /** the echo delay lines as a vector */
        AudioSampleRingBufferVector _delayLineList;

        /*--------------------*/
        /*--------------------*/

        /**
         * Sets up a new effect descriptor for <C>effect</C>
         *
         * @param[inout] effect  associated chorus and echo effect
         */
        _EffectDescriptor_ECHO (INOUT SoXChorusAndEcho_AudioEffect& effect);

        /*--------------------*/

        /**
         * Applies the echo effect on multiple channel data from
         * <C>inputSampleList</C> and returns echo result in
         * <C>outputSampleList</C>.
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
         * given <C>sampleRate</C> and <C>channelCount</C> and
         * <C>currentTime</C> (because this effect is timelocked).
         *
         * @param[in] sampleRate    the new sample rate for effect
         * @param[in] channelCount  the new channel count
         * @param[in] currentTime   the current play head time
         */
        void updateSettings (IN Real sampleRate,
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
using SoXPlugins::Effects::SoXChorusAndEcho::_EffectDescriptor_ECHO;

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

/*------------------------*/
/* _DelayStageParameterData */
/*------------------------*/

String _DelayStageParameterData::toString () const
{
    String st = STR::expand("isChorusStage = %1, delay = %2s,"
                            " decay = %3s",
                            TOSTRING(_isChorusStage), TOSTRING(_delay),
                            TOSTRING(_decay));

    if (_isChorusStage) {
        st = (st
              + STR::expand(", frequency = %1Hz, depth = %2s, kind = %3",
                            TOSTRING(_frequency), TOSTRING(_depth),
                            Audio::waveFormKindToString(_modulationKind)));
    }

    st = STR::expand("_DelayStageParameterData(%1)", st);
    return st;
}

/*------------------------*/
/* _EffectDescriptor_ECHO */
/*------------------------*/

_EffectDescriptor_ECHO::
_EffectDescriptor_ECHO (INOUT SoXChorusAndEcho_AudioEffect& effect)
    : _effect{effect},
      _kind{_DelayKind::tappedDelay},
      _inGain{0.0},
      _outGain{0.0},
      _stageCount{1},
      _channelCount{0},
      _indexToStageParamDataMap{},
      _inputSampleList{},
      _outputSampleList{},
      _delayLineList{}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

void _EffectDescriptor_ECHO::apply ()
{
    Logging_trace(">>");

    const Boolean isChorus          = (_kind == _DelayKind::chorus);
    const Boolean isSequentialDelay = (_kind == _DelayKind::delaySequence);

    Natural delayLineIndex = 0;

    for (Natural channel = 0;  channel < _channelCount;  channel++) {
        AudioSample inputSample         = _inputSampleList[channel];
        AudioSample outputSample        = inputSample * _inGain;
        AudioSample previousStageSample = 0.0;

        for (Natural stageIndex = 0;  stageIndex < _stageCount;
             stageIndex++) {
            _DelayStageParameterData& stage =
                _indexToStageParamDataMap[stageIndex];
            const Real decay = stage._decay;
            Natural indexOffset;

            if (!isChorus) {
                /* no modulation, take first entry in delay line */
                indexOffset = 0;
            } else {
                /* access a position modulated by modulation wave */
                WaveForm& waveForm{stage._modulationWave};
                indexOffset = Natural{waveForm.current()};

                if (channel == _channelCount - 1) {
                    waveForm.advance();
                }
            }

            Logging_trace2("--: stage = %1, indexOffset = %2",
                           TOSTRING(stageIndex), TOSTRING(indexOffset));

            AudioSampleRingBuffer& delayLine =
                _delayLineList.at(delayLineIndex);
            AudioSample stageSample;

            if (delayLine.length() == 0) {
                stageSample = inputSample;
            } else {
                stageSample = delayLine.at(indexOffset);
                /* the next stage either gets as input the input
                   sample or the input sample plust the current stage
                   sample */
                AudioSample delayedSample =
                    (!isSequentialDelay
                     ? inputSample
                     : previousStageSample + inputSample);
                delayLine.shiftLeft(delayedSample);
            }

            outputSample += stageSample * decay;
            Logging_trace2("--: stageSample = %1, outputSample = %2",
                           TOSTRING(stageSample), TOSTRING(outputSample));
            previousStageSample = stageSample;
            delayLineIndex++;
        }

        _outputSampleList[channel] = outputSample * _outGain;
    }
    
    Logging_trace("<<");
}
        
/*--------------------*/

String _EffectDescriptor_ECHO::internalState () const
{
    const Boolean isChorus = (_kind == _DelayKind::chorus);
    String st;
    Natural delayLineIndex = 0;

    for (Natural stageIndex = 0;  stageIndex < _stageCount;  stageIndex++) {
        const _DelayStageParameterData& stage =
            _indexToStageParamDataMap[stageIndex];
        AudioSample stageSample = 0.0;
        Natural indexOffset =
            (!isChorus ? 0 : Natural{stage._modulationWave.current()});

        if (_channelCount > 0) {
            const AudioSampleRingBuffer& delayLine =
                _delayLineList.at(delayLineIndex);
            stageSample = (indexOffset >= delayLine.length()
                           ? 0.0 : delayLine.at(indexOffset));
            delayLineIndex += _channelCount;
        }

        st = (st
              + STR::expand(", stage = %1, sample = %2",
                            TOSTRING(stageIndex), TOSTRING(stageSample)));

        if (isChorus) {
            Real offsetAsTime =
                (stage._depth - Real{indexOffset} / _sampleRate) * 1000.0;
            st = (st
                  + STR::expand(", offset = %1, offsetAsTime = %2,"
                                " waveState = %3",
                                TOSTRING(indexOffset),
                                TOSTRING(offsetAsTime),
                                TOSTRING(stage._modulationWave.state())));
        }
    }

    return st;
}

/*--------------------*/

void _EffectDescriptor_ECHO::updateSettings (IN Real sampleRate,
                                             IN Natural channelCount,
                                             IN Real currentTime)
{
    Logging_trace2(">>: sampleRate = %1, channelCount = %2",
                   TOSTRING(sampleRate), TOSTRING(channelCount));

    _effect.setParameterValidity(false);
    _sampleRate = sampleRate;
    _channelCount = channelCount;
    _inputSampleList.setLength(_channelCount);
    _outputSampleList.setLength(_channelCount);

    Boolean isChorusEffect = (_kind == _DelayKind::chorus);
    Natural delayLineCount = _channelCount * _maxStageCount;
    _delayLineList.setRingBufferCount(delayLineCount);

    for (Natural stageIndex = 0;  stageIndex < _stageCount; stageIndex++) {
        Natural delayLineIndex = stageIndex;
        _DelayStageParameterData& stage =
            _indexToStageParamDataMap[stageIndex];
        stage._isChorusStage = isChorusEffect;

        Real delayLineLengthInSeconds =
            stage._delay + (isChorusEffect ? stage._depth : 0.0);
        Natural delayLineLength =
            Natural::ceiling((double) (delayLineLengthInSeconds * _sampleRate));
        Logging_trace2("--: stageIndex = %1, delayLineLength = %2",
                       TOSTRING(stageIndex), TOSTRING(delayLineLength));

        /* update modulation waveform for a chorus*/
        if (isChorusEffect && stage._frequency > 0.0001) {
            /* shift waveform by phase caused by time offset*/
            const Radians effectivePhase =
                (_defaultChorusPhase
                 + WaveForm::phaseByTime(stage._frequency,
                                         _timeOffset, currentTime));

            const Real frequency = stage._frequency;
            Real waveFormLength =
                frequency < 0.001 ? 1.0 : _sampleRate / stage._frequency;
            Real maximumModulationValue =
                Real::floor(stage._depth * _sampleRate);
            WaveForm& waveForm = stage._modulationWave;
            waveForm.set(waveFormLength, stage._modulationKind,
                         0.0, maximumModulationValue,
                         effectivePhase, true);
        }

        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            AudioSampleRingBuffer& delayLine =
                _delayLineList.at(delayLineIndex);
            delayLine.setLength(delayLineLength);
            delayLineIndex += _stageCount;
        }                
    }

    _effect.setParameterValidity(true);
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

String _EffectDescriptor_ECHO::toString () const
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
    Natural delayLineIndex = 0;

    for (Natural stageIndex = 0;  stageIndex < _stageCount;  stageIndex++) {
        const _DelayStageParameterData& data =
            _indexToStageParamDataMap[stageIndex];
        Natural delayLineLength = 0;

        if (_channelCount > 0) {
            const AudioSampleRingBuffer& delayLine =
                _delayLineList.at(delayLineIndex);
            delayLineIndex += _channelCount;
            delayLineLength = delayLine.length();
        }

        stageDataString +=
            ((stageIndex > 0 ? ", " : "")
             + STR::expand("stage_%1(%2,"
                           " modulationWave = %3, delayLineLength = %4)",
                           TOSTRING(stageIndex), data.toString(),
                           data._modulationWave.toString(),
                           TOSTRING(delayLineLength)));
    }

    stageDataString = STR::expand("(%1)", stageDataString);

    String st = STR::expand("%1, stages = %2", prefix, stageDataString);
    st = STR::expand("_EffectDescriptor_ECHO(%1)", st);
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
 * @param[in]    delayKind     the new sample rate for effect
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
    Real minimumDelay = (isChorusEffect ?  20.0 :     0.0);

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
                                  0.1, 5.0, 0.01);
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
    _effectDescriptor = new _EffectDescriptor_ECHO(*this);
    _EffectDescriptor_ECHO& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ECHO>(_effectDescriptor);
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
    effectDescriptor.updateSettings(_sampleRate, 2, _currentTimePosition);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXChorusAndEcho_AudioEffect::~SoXChorusAndEcho_AudioEffect ()
{
    Logging_trace(">>");
    delete (_EffectDescriptor_ECHO*) _effectDescriptor;
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
    const _EffectDescriptor_ECHO& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ECHO>(_effectDescriptor);
    return effectDescriptor.toString();
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
    const _EffectDescriptor_ECHO& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ECHO>(_effectDescriptor);
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
    _EffectDescriptor_ECHO& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ECHO>(_effectDescriptor);
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
                effectDescriptor.updateSettings(_sampleRate, _channelCount,
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
        effectDescriptor.updateSettings(_sampleRate, _channelCount,
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
            effectDescriptor.updateSettings(_sampleRate, _channelCount,
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
    setValueViaParent(parameterName_effectKind, delayKindAsString, false);
    setValueViaParent(parameterName_inGain, "0.0", false);
    setValueViaParent(parameterName_outGain, "0.0", false);

    st = pagedParameterName(parameterName_stageCount,
                            SoXEffectParameterMap::pageCounterPage);
    setValueViaParent(st, "1", false);

    st = pagedParameterName(parameterName_stageIndex,
                            SoXEffectParameterMap::selectorPage);
    setValueViaParent(st, "1", false);

    for (Natural stageIndex = 0;  stageIndex < _maxStageCount;
         stageIndex++) {
        const auto pagedName =
            [stageIndex] (const String& st) {
                String result;
                result = pagedParameterName(st, Integer{stageIndex + 1});
                return result;
            };

        setValueViaParent(pagedName(parameterName_delay),     "0.0", false);
        setValueViaParent(pagedName(parameterName_decay),     "0.0", false);

        /* dummy values for chorus */
        setValueViaParent(pagedName(parameterName_depth),     "0.0", false);
        setValueViaParent(pagedName(parameterName_frequency), "1.0", false);
        setValueViaParent(pagedName(parameterName_modulationKind),
                          "Sine", false);
    }

    /* stages */
    _updateParameterMap(delayKind, _effectParameterMap);
    
    _EffectDescriptor_ECHO& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ECHO>(_effectDescriptor);
    effectDescriptor._stageCount = 1;
    effectDescriptor.updateSettings(_sampleRate, _channelCount,
                                    _currentTimePosition);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXChorusAndEcho_AudioEffect::prepareToPlay (IN Real sampleRate)
{
    Logging_trace1(">>: %1", TOSTRING(sampleRate));

    _EffectDescriptor_ECHO& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_ECHO>(_effectDescriptor);

    if (sampleRate == _sampleRate) {
        Logging_trace("--: no recalculation");
    } else {
        _sampleRate = sampleRate;
        /* echo stages have to be recalculated to change the lengths */
        effectDescriptor.updateSettings(_sampleRate, _channelCount,
                                        _currentTimePosition);
    }

    /* reset delay lines */
    Natural delayLineIndex = 0;
    Natural stageCount = effectDescriptor._stageCount;

    for (Natural stageIndex = 0;  stageIndex < stageCount; stageIndex++) {
        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            AudioSampleRingBuffer& delayLine =
                effectDescriptor._delayLineList.at(delayLineIndex++);
            delayLine.setToZero();
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void
SoXChorusAndEcho_AudioEffect::processBlock
                                  (IN Real timePosition,
                                   INOUT AudioSampleListVector& buffer)
{
    Logging_trace1(">>: time = %1", TOSTRING(timePosition));

    if (_parametersAreValid) {
        String st;
        Real effectiveTime = timePosition;
        const Real deltaTime = Real::one / _sampleRate;
        _EffectDescriptor_ECHO& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_ECHO>(_effectDescriptor);

        SoXAudioEffect::processBlock(timePosition, buffer);

        if (_timePositionHasMoved
            || _channelCount != effectDescriptor._channelCount) {
            /* channel count has changed or playhead was moved ==> keep
               time synchronisation of modulation waveforms */
            effectDescriptor.updateSettings(_sampleRate, _channelCount,
                                            _currentTimePosition);
        }

        const Natural sampleCount = buffer[0].size();
        AudioSampleList& inputSampleList  = effectDescriptor._inputSampleList;
        AudioSampleList& outputSampleList = effectDescriptor._outputSampleList;

        for (Natural i = 0;  i < sampleCount;  i++) {
            /* read input sample list at position i for all channels */
            for (Natural channel = 0;  channel < _channelCount;  channel++) {
                const AudioSampleList& inputList = buffer[channel];
                inputSampleList[channel] = inputList[i];
            }

            if (Logging_isActive) {
                st = STR::expand("time = %1ms, inputSample = %2%3",
                                 TOSTRING(effectiveTime * 1000.0),
                                 TOSTRING(inputSampleList[0]),
                                 effectDescriptor.internalState());
            }

            effectDescriptor.apply();

            Logging_trace2("--: %1, outputSample = %2",
                           st, TOSTRING(outputSampleList[0]));

            /* write output sample list at position i onto all channels */
            for (Natural channel = 0;  channel < _channelCount;  channel++) {
                AudioSampleList& outputList = buffer[channel];
                outputList[i] = outputSampleList[channel];
            }

            effectiveTime += deltaTime;
        }
    }

    Logging_trace("<<");
}
