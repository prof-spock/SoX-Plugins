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
#include "SoXChorusAndEcho_AudioEffect.h"
#include "SoXEffectParameterMap.h"
#include "SoXEmulationHelper.h"
#include "WaveForm.h"

/*--------------------*/

using Audio::DelayLineList;
using Audio::ValuePrecision;
using Audio::WaveForm;
using Audio::WaveFormKind;
using BaseTypes::GenericTypes::GenericTuple;
using SoXPlugins::Effects::SoXChorusAndEcho::SoXChorusAndEcho_AudioEffect;
using SoXPlugins::Helpers::SoXEffectParameterMap;
using SoXPlugins::Helpers::SoXEmulationHelper;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/** abbreviation for adaptValueForSoXEmulation */
static const auto adaptValueForSoXEmulation =
    SoXEmulationHelper::adaptValueForSoXEmulation;

/** abbreviation for pagedParameterName */
static const auto pagedParameterName =
    SoXEffectParameterMap::pagedParameterName;

/** abbreviation for soxIsStrictlyEmulated */
static const auto soxIsStrictlyEmulated =
    SoXEmulationHelper::soxIsStrictlyEmulated;

/*====================*/

/** maximum number of stages in a chorus or echo */
constexpr int _maximumStageCount = 10;

/** minimum depth value */
static const Real _minimumDepth = 0.0;

/** depth resolution */
static const Real _depthResolution = 0.1;

/** the default phase of each chorus modulation waveform */
static const Radians _defaultChorusPhase =
    Radians{atan(1) * 4 * 1.5};

/** the maximum order of interpolation, quadratic */
static const Natural _maximumInterpolationOrder = 2;

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

/*....................*/
/* EFFECT KINDS       */
/*....................*/

/** the name of the chorus effect */
static const String _EffectKind_chorus = "Chorus";

/** the name of the delay sequence effect */
static const String _EffectKind_delaySequence = "Delay Sequence";

/** the name of the tapped delay effect */
static const String _EffectKind_tappedDelay = "Tapped Delay";

/*============================================================*/

namespace SoXPlugins::Effects::SoXChorusAndEcho {

    /**
     * A <C>_DelayStageParameterData</C> object contains the parameter
     * data for a single chorus/echo stage.
     */
    struct _DelayStageParameterData {

        /** tells whether this is a chorus stage */
        Boolean isChorusStage;
        
        /** the stage delay in seconds */
        Real delay;

        /** the stage decay for this stage (as a factor) */
        Real decay;

        /** the stage chorus modulation frequency in Hz (when this is
         * a chorus stage) */
        Real frequency;

        /** the stage chorus depth for this stage in seconds (when
         * this is a chorus stage) */
        Real depth;

        /** the stage modulation waveform kind for this stage (when
         * this is a chorus stage) */
        WaveFormKind modulationKind;

        /** the waveform used for the modulation of this stage (when
         * this is a chorus stage) */
        WaveForm modulationWave;

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

    /** fixed-length list with stage data */
    using _StageList = GenericList<_DelayStageParameterData>;

    /*====================*/

    /**
     * An <C>_EffectDescriptor_CECH</C> object is the internal
     * implementation of an chorus or echo effect descriptor type
     * where all data point input is routed to and data point output
     * is routed from.
     */
    struct _EffectDescriptor_CECH {

        /** the associated echo effect */
        SoXChorusAndEcho_AudioEffect& effect;
        
        /** information whether effect is a chorus, an echo or echos */
        String kind;

        /** the input gain as a factor */
        Real inGain;

        /** the output gain as a factor */
        Real outGain;

        /** the time offset in seconds (for timelocking a chorus
         * effect) */
        Real timeOffset;

        /** the number of delay lines in this multiline effect */
        Natural stageCount;

        /** the current data point rate for this effect */
        Real dataPointRate;
        
        /** the number of audio channels in this effect */
        Natural channelCount;

        /** a list of stage parameters (like e.g. delay and decay) */
        _StageList stageList;

        /** the order of interpolation for the delay lines (for a
         * chorus): 0 is no interpolation, 1 is linear and 2 is
         * quadratic */
        Natural interpolationOrder;

        /** the echo delay lines as a list */
        DelayLineList delayLineList;

        /*--------------------*/
        /*--------------------*/

        /**
         * Returns const reference to delay line for <C>channel</C> and
         *<C>stage</C>
         *
         * @param[in] channel     channel represented by delay line
         * @param[in] stageIndex  stage represented by delay line
         */
        const DelayLine& delayLineAt (Natural channel,
                                       Natural stageIndex) const;

        /*--------------------*/

        /**
         * Returns reference to delay line for <C>channel</C> and
         *<C>stage</C>
         *
         * @param[in] channel     channel represented by delay line
         * @param[in] stageIndex  stage represented by delay line
         */
        DelayLine& delayLineAt (Natural channel,
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
         * Applies the effect on multiple channel frame from
         * <C>inputDataPointList</C> and returns result in
         * <C>outputDataPointList</C>.
         *
         * @param [in]  inputDataPointList   list of input data points
         *                                   for each channel
         * @param [out] outputDataPointList  list of output data points
         *                                   for each channel
         */
        void applyForFrame (IN AudioDataPointList& inputDataPointList,
                            OUT AudioDataPointList& outputDataPointList);

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

using SoXPlugins::Effects::SoXChorusAndEcho::_DelayStageParameterData;
using SoXPlugins::Effects::SoXChorusAndEcho::_EffectDescriptor_CECH;

/*--------------------------*/
/* _DelayStageParameterData */
/*--------------------------*/

String _DelayStageParameterData::toString () const
{
    String st = STR::expand("isChorusStage = %1, delay = %2s,"
                            " decay = %3s",
                            TOSTRING(isChorusStage), TOSTRING(delay),
                            TOSTRING(decay));

    if (isChorusStage) {
        st += STR::expand(", frequency = %1Hz, depth = %2s, kind = %3",
                          TOSTRING(frequency), TOSTRING(depth),
                          Audio::waveFormKindToString(modulationKind));
    }

    st = STR::expand("_DelayStageParameterData(%1)", st);
    return st;
}

/*------------------------*/
/* _EffectDescriptor_CECH */
/*------------------------*/

_EffectDescriptor_CECH::
_EffectDescriptor_CECH (INOUT SoXChorusAndEcho_AudioEffect& parent)
    : effect{parent},
      kind{_EffectKind_tappedDelay},
      inGain{0.0},
      outGain{0.0},
      stageCount{1},
      channelCount{0},
      stageList{_maximumStageCount},
      interpolationOrder{0},
      delayLineList{}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

const DelayLine&
_EffectDescriptor_CECH::delayLineAt (Natural channel,
                                     Natural stageIndex) const
{
    Logging_trace2(">>: channel = %1, stageIndex = %2",
                   TOSTRING(channel), TOSTRING(stageIndex));

    const Natural delayLineIndex = channel * stageCount + stageIndex;
    const DelayLine& result = delayLineList.at(delayLineIndex);
   
    Logging_trace("<<");
    return result;
}

/*--------------------*/

DelayLine& _EffectDescriptor_CECH::delayLineAt (Natural channel,
                                                 Natural stageIndex)
{
    return const_cast<DelayLine&>(std::as_const(*this).delayLineAt(channel, stageIndex));
}

/*--------------------*/

void
_EffectDescriptor_CECH::applyForFrame
                            (IN AudioDataPointList& inputDataPointList,
                             OUT AudioDataPointList& outputDataPointList)
{
    Logging_trace(">>");

    const Boolean isChorus          = (kind == _EffectKind_chorus);
    const Boolean isSequentialDelay = (kind == _EffectKind_delaySequence);

    String st;

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        AudioDataPoint inputDataPoint         = inputDataPointList[channel];
        AudioDataPoint outputDataPoint        = inputDataPoint * inGain;
        AudioDataPoint previousStageDataPoint = 0.0;
        const Boolean isLastChannel = (channel == channelCount - 1);

        for (Natural stageIndex = 0;  stageIndex < stageCount;
             stageIndex++) {
            _DelayStageParameterData& stage = stageList[stageIndex];
            const Real decay = stage.decay;
            Real indexOffset;

            if (!isChorus) {
                /* no modulation, take first entry in delay line */
                indexOffset = 0.0;
            } else {
                /* access a position modulated by modulation wave */
                WaveForm& waveForm{stage.modulationWave};
                indexOffset = -waveForm.current();

                if (isLastChannel) {
                    waveForm.advance();
                }
            }

            DelayLine& delayLine = delayLineAt(channel, stageIndex);
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

        outputDataPointList[channel] = outputDataPoint * outGain;
    }
    
    Logging_trace("<<");
}
        
/*--------------------*/

String _EffectDescriptor_CECH::internalState () const
{
    const Boolean isChorus = (kind == _EffectKind_chorus);
    String st;

    for (Natural stageIndex = 0;  stageIndex < stageCount;
         stageIndex++) {
        const _DelayStageParameterData& stage = stageList[stageIndex];
        Real indexOffset =
            (!isChorus ? 0.0 : stage.modulationWave.current());

        st += ((stageIndex == 0 ? "" : ", ")
               + STR::expand("stage = %1", TOSTRING(stageIndex)));

        if (isChorus) {
            Real offsetAsTime = indexOffset * 1000.0 / dataPointRate;
            st += STR::expand(", offset = %1smp, offsetAsTime = %2ms,"
                              " waveState = %3",
                              TOSTRING(indexOffset),
                              TOSTRING(offsetAsTime),
                              TOSTRING(stage.modulationWave.state()));
        }

        AudioDataPointList stageDataPointList;

        for (Natural channel = 0;  channel < channelCount;  channel ++) {
            const DelayLine& delayLine = delayLineAt(channel, stageIndex);
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

    effect.setParameterValidity(false);
    this->dataPointRate = dataPointRate;
    this->channelCount = channelCount;

    const Boolean isChorusEffect = (kind == _EffectKind_chorus);
    const Natural delayLineCount = channelCount * _maximumStageCount;
    const Real delayLineSlack =
        Real{(soxIsStrictlyEmulated ? interpolationOrder : 0)};

    /* the delay in SoX applies an interpolation using earlier points
     * in the delay line for interpolation order of one and the value
     * precision is integer for an interpolation order of zero */
    const ListInterpolatorPointLocation pointLocation =
        (soxIsStrictlyEmulated && interpolationOrder == 1
         ? ListInterpolatorPointLocation::atBegin
         : ListInterpolatorPointLocation::atCenter);
    const ValuePrecision waveFormValuePrecision =
        (soxIsStrictlyEmulated() && interpolationOrder == 0
         ? ValuePrecision::integerPrecision
         : ValuePrecision::doublePrecision);

    delayLineList.setLength(delayLineCount);
    delayLineList.setElementInterpolation(interpolationOrder,
                                          pointLocation);
    delayLineList.setToZero();

    for (Natural stageIndex = 0;  stageIndex < stageCount; stageIndex++) {
        _DelayStageParameterData& stage = stageList[stageIndex];
        stage.isChorusStage = isChorusEffect;

        Real delayLineLengthInDataPoints =
            (stage.delay + stage.depth) * dataPointRate;
        const Real delayLineLength =
            (adaptValueForSoXEmulation(delayLineLengthInDataPoints, 'c')
             + delayLineSlack);

        /* update modulation waveform for a chorus*/
        if (isChorusEffect && stage.frequency > 0.0001) {
            /* shift waveform by phase caused by time offset*/
            const Radians effectivePhase =
                (_defaultChorusPhase
                 + WaveForm::phaseByTime(stage.frequency,
                                         timeOffset, currentTime));

            const Real frequency = stage.frequency;
            Real waveFormLength = (frequency < 0.001
                                   ? 1.0
                                   : dataPointRate / stage.frequency);
            waveFormLength =
                adaptValueForSoXEmulation(waveFormLength, 'r');
            Real minimumModulationValue = stage.delay * dataPointRate;
            Real maximumModulationValue = delayLineLengthInDataPoints;

            WaveForm& waveForm = stage.modulationWave;
            waveForm.set(waveFormLength, stage.modulationKind,
                         minimumModulationValue, maximumModulationValue,
                         effectivePhase, waveFormValuePrecision);
        }

        Logging_trace2("--: stageIndex = %1, delayLineLength = %2smp",
                       TOSTRING(stageIndex), TOSTRING(delayLineLength));

        for (Natural channel = 0;  channel < channelCount;  channel++) {
            DelayLine& delayLine = delayLineAt(channel, stageIndex);
            delayLine.setLength(delayLineLength);
        }                
    }

    effect.setParameterValidity(true);
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

String _EffectDescriptor_CECH::toString () const
{
    String prefix =
        STR::expand("kind = %1,"
                    " inGain = %2, outGain = %3,"
                    " timeOffset = %4,"
                    " stageCount = %5, channelCount = %6,"
                    " interpolationOrder = %7",
                    kind,
                    TOSTRING(inGain), TOSTRING(outGain),
                    TOSTRING(timeOffset),
                    TOSTRING(stageCount), TOSTRING(channelCount),
                    TOSTRING(interpolationOrder));

    String stageDataString;

    for (Natural stageIndex = 0;  stageIndex < stageCount;  stageIndex++) {
        const _DelayStageParameterData& data = stageList[stageIndex];
        Real delayLineLength = 0.0;

        if (channelCount > 0) {
            const DelayLine& delayLine = delayLineAt(0, stageIndex);
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
                           data.modulationWave.toString(),
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
    StringList::fromList({_EffectKind_chorus, _EffectKind_tappedDelay,
                          _EffectKind_delaySequence});

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

/** the name of the effect kind parameter */
static const String parameterName_effectKind = "Effect Kind";

/** the name of the input gain parameter */
static const String parameterName_inGain     = "In Gain";

/** the name of the interpolation order parameter */
static const String parameterName_interpolationOrder =
    "Interp. Order";

/** the name of the output gain parameter */
static const String parameterName_outGain    = "Out Gain";

/** the name of the stage count parameter */
static const String parameterName_stageCount   = "Stage Count";

/** the name of the stage index parameter */
static const String parameterName_stageIndex   = "Stage Index";

/** the name of the time offset parameter */
static const String parameterName_timeOffset   = "Time Offset [s]";

/** the name of the delay parameter */
static const String parameterName_delay =
    _delayStageParameterNameList[0];

/** the name of the decay parameter */
static const String parameterName_decay =
    _delayStageParameterNameList[1];

/** the name of the delay parameter */
static const String parameterName_depth =
    _delayStageParameterNameList[3];

/** the name of the modulation frequency parameter */
static const String parameterName_frequency =
    _delayStageParameterNameList[2];

/** the name of the modulation waveform parameter */
static const String parameterName_modulationKind =
    _delayStageParameterNameList[4];

/*--------------------*/
/* internal routines  */
/*--------------------*/

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
    const String effectKindString = _EffectKind_tappedDelay;
    _effectParameterMap.setKindAndValueEnum(parameterName_effectKind,
                                            _kindList,
                                            effectKindString);
    _effectParameterMap.setKindReal(parameterName_inGain,
                                    0.0, 1.0, 0.001);
    _effectParameterMap.setKindReal(parameterName_outGain,
                                    0.0, 5.0, 0.001);
    _effectParameterMap.setKindInt(parameterName_interpolationOrder,
                                   0, _maximumInterpolationOrder, 1);
    _effectParameterMap.setKindReal(parameterName_timeOffset,
                                    -8192.0, +8192.0,
                                    Real::two.power(-16.0));

    /* page count and page selection */
    st = pagedParameterName(parameterName_stageCount,
                            SoXEffectParameterMap::pageCounterPage);
    _effectParameterMap.setKindInt(st, 1, _maximumStageCount, 1);

    st = pagedParameterName(parameterName_stageIndex,
                            SoXEffectParameterMap::selectorPage);
    _effectParameterMap.setKindInt(st, 1, _maximumStageCount, 1);

    /* stages */
    _updateEffectForKind(effectKindString);
    
    effectDescriptor.stageCount = 1;
    recalculate();

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

String SoXChorusAndEcho_AudioEffect::_effectDescriptorToString () const
{
    String result;

    if (_effectDescriptor == nullptr) {
        result = "NULL";
    } else {
        const _EffectDescriptor_CECH& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);

        result = effectDescriptor.toString();
    }

    return result;
}

/*-----------------------*/

String SoXChorusAndEcho_AudioEffect::_internalState () const
{
    String result;

    if (_effectDescriptor == nullptr) {
        result = "";
    } else {
        const _EffectDescriptor_CECH& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
        result = effectDescriptor.internalState();
    }

    return result;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

Boolean SoXChorusAndEcho_AudioEffect::isTimeVariant () const
{
    return true;
}

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
    const Natural stageCount = effectDescriptor.stageCount;
    const String kind = effectDescriptor.kind;
    const Boolean isChorusEffect    = (kind == _EffectKind_chorus);
    const Boolean isSequentialDelay = (kind == _EffectKind_delaySequence);

    Real result = 0.0;

    for (Natural stageIndex = 0;  stageIndex < stageCount;
         stageIndex++) {
        const _DelayStageParameterData& stage =
            effectDescriptor.stageList[stageIndex];
        const Real delayLineLengthInSeconds =
            stage.delay + (isChorusEffect ? stage.depth : 0.0);

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

SoXValueChangeKind SoXChorusAndEcho_AudioEffect
::_setValueInternal (IN String& parameterName,
                     IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    SoXValueChangeKind result =
        SoXValueChangeKind::parameterChange;
    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
    const String stageCountParam = parameterName_stageCount;
    const String stageIndexParam = parameterName_stageIndex;

    if (parameterName == parameterName_effectKind) {
        const String chorusKind = _EffectKind_chorus;
        const String previousKind = effectDescriptor.kind;

        if (value != previousKind) {
            effectDescriptor.kind = value;

            if (previousKind == chorusKind || value == chorusKind) {
                /* when change is from or to chorus, update the
                 * parameter ranges and the complete effect */
                _updateEffectForKind(value);
                recalculate();
                result = SoXValueChangeKind::globalChange;
            }
        }
    } else if (parameterName == parameterName_inGain) {
        effectDescriptor.inGain = STR::toReal(value);
    } else if (parameterName == parameterName_interpolationOrder) {
        effectDescriptor.interpolationOrder = STR::toNatural(value);
    } else if (parameterName == parameterName_outGain) {
        effectDescriptor.outGain = STR::toReal(value);
    } else if (parameterName == parameterName_timeOffset) {
        effectDescriptor.timeOffset = STR::toReal(value);
    } else if (STR::endsWith(parameterName, stageCountParam)) {
        Logging_trace1("--: new stageCount = %1", value);
        const Natural stageCount =
            Natural::forceToInterval(STR::toNatural(value),
                                     1, _maximumStageCount);
        effectDescriptor.stageCount = stageCount;
        _effectParameterMap.setValue(stageCountParam, TOSTRING(stageCount));
        recalculate();
    } else if (STR::endsWith(parameterName, stageIndexParam)) {
        const Natural stageIndex =
            Natural::forceToInterval(STR::toNatural(value),
                                     1, effectDescriptor.stageCount);
        _effectParameterMap.setValue(stageIndexParam, TOSTRING(stageIndex));
    } else {
        Natural pageIndex;
        String effectiveParameterName;
        SoXEffectParameterMap::splitParameterName(parameterName,
                                                  effectiveParameterName,
                                                  pageIndex);
        const Natural stageIndex = pageIndex - 1;

        if (stageIndex < effectDescriptor.stageCount) {
            _DelayStageParameterData& data =
                effectDescriptor.stageList[stageIndex];

            const auto updateFromPartner =
                [this, pageIndex] (IN String& parameterName,
                                   IN Real valueInSeconds) {
                    Real valueInMilliseconds{valueInSeconds * 1000.0};
                    String longParameterName =
                        pagedParameterName(parameterName, pageIndex);
                    this->setValue(longParameterName,
                                   TOSTRING(valueInMilliseconds));
                };

            if (effectiveParameterName == parameterName_delay) {
                data.delay = STR::toReal(value) / 1000.0;

                if (data.delay < data.depth) {
                    updateFromPartner(effectiveParameterName, data.depth);
                }
            } else if (effectiveParameterName == parameterName_decay) {
                data.decay = STR::toReal(value);
            } else if (effectiveParameterName == parameterName_frequency) {
                data.frequency = STR::toReal(value);
            } else if (effectiveParameterName == parameterName_depth) {
                data.depth = STR::toReal(value) / 1000.0;

                if (data.depth > data.delay) {
                    updateFromPartner(effectiveParameterName, data.delay);
                }
            } else if (effectiveParameterName == parameterName_modulationKind) {
                data.modulationKind = Audio::stringToWaveFormKind(value);
            }
        }
    }

    Logging_trace1("<<: %1", SoXValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXChorusAndEcho_AudioEffect::setDefaultValues ()
{
    Logging_trace(">>");

    String st;
    _channelCount = 2;

    const String effectKindString = _EffectKind_tappedDelay;

    /* update map for the stages */
    _updateEffectForKind(effectKindString);

    Dictionary dictionary;

    dictionary.set(parameterName_effectKind, effectKindString);
    dictionary.set(parameterName_inGain, "0.0");
    dictionary.set(parameterName_outGain, "0.0");

    st = pagedParameterName(parameterName_stageCount,
                            SoXEffectParameterMap::pageCounterPage);
    dictionary.set(st, "1");

    for (Natural stageIndex = 0;  stageIndex < _maximumStageCount;
         stageIndex++) {
        const auto pagedName =
            [stageIndex] (const String& st) {
                String result;
                result = pagedParameterName(st, Integer{stageIndex + 1});
                return result;
            };

        dictionary.set(pagedName(parameterName_delay),     "0.0");
        dictionary.set(pagedName(parameterName_decay),     "0.0");

        /* dummy values for chorus */
        dictionary.set(pagedName(parameterName_depth),     "0.0");
        dictionary.set(pagedName(parameterName_frequency), "1.0");
        dictionary.set(pagedName(parameterName_modulationKind), "Sine");
    }

    st = pagedParameterName(parameterName_stageIndex,
                            SoXEffectParameterMap::selectorPage);
    dictionary.set(st, "1");
    setValues(dictionary);

    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
    effectDescriptor.stageCount = 1;
    recalculate();

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXChorusAndEcho_AudioEffect::
_applyForFrame (IN AudioDataPointList& inputDataPointList,
                OUT AudioDataPointList& outputDataPointList)
{
    Logging_trace(">>");

    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
    effectDescriptor.applyForFrame(inputDataPointList,
                                   outputDataPointList);

    Logging_trace("<<");
}

/*--------------------*/

void SoXChorusAndEcho_AudioEffect::
_handleDataPointRateChange (IN Real dataPointRate)
{
    Logging_trace1(">>: %1", TOSTRING(dataPointRate));

    recalculate();
    
    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);

    /* reset delay lines */
    Natural stageCount = effectDescriptor.stageCount;

    for (Natural stageIndex = 0;  stageIndex < stageCount; stageIndex++) {
        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            DelayLine& delayLine =
                effectDescriptor.delayLineAt(channel, stageIndex);
            delayLine.setToZero();
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void
SoXChorusAndEcho_AudioEffect::recalculate ()
{
    Logging_trace(">>");

    _EffectDescriptor_CECH& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_CECH>(_effectDescriptor);
    effectDescriptor.updateSettings(_dataPointRate,
                                    _channelCount,
                                    _currentTimePosition);

    Logging_trace("<<");
}

/*--------------------*/

void SoXChorusAndEcho_AudioEffect::
_updateEffectForKind (IN String& effectKind)
{
    Logging_trace1(">>: %1", effectKind);

    Boolean isChorusEffect = (effectKind == _EffectKind_chorus);
    SoXEffectParameterMap& parameterMap = _effectParameterMap;

    /* activate or deactivate time offset for chorus */
    parameterMap.setActiveness(parameterName_timeOffset, isChorusEffect);
    parameterMap.setActiveness(parameterName_interpolationOrder,
                               isChorusEffect);

    /* delay value range (in milliseconds) */
    Real maximumDelay = (isChorusEffect ? 100.0 : 20000.0);
    Real minimumDelay = (isChorusEffect ?  _minimumChorusDelay : 0.0);

    for (Natural stageIndex = 0;  stageIndex < _maximumStageCount;
         stageIndex++) {
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
