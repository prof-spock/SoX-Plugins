/**
 * @file
 * The <C>SoXFlangerPhaserAndTremolo_AudioEffect</C> body implements
 * the SoX <B>flanger</B>, <B>phaser</B> and <B>tremolo</B> modulation
 * effects that have similar parameters and hence are combined into a
 * single plugin.
 *
 * The original algorithms can be found in the <A
 * HREF="https://sourceforge.net/p/sox/code">SoX repository</A>
 * in the files
 *   - "flanger.c" by Rob Sewell with the code for the flanger effect,
 *   - "phaser.c" by Juergen Mueller and Sundry Contributors with the
 *     code for the phaser effect,
 *   - "synth.c" by Carsten Borchardt and Rob Sowell with the
 *     code for the synth base effect, and
 *   - "tremolo.c" by Rob Sewell with the code for the (wrapper) tremolo
 *     effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-09
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "DelayLineList.h"
#include "Logging.h"
#include "Percentage.h"
#include "WaveForm.h"
#include "SoXEmulationHelper.h"
#include "SoXFlangerPhaserAndTremolo_AudioEffect.h"

/*--------------------*/

using Audio::DelayLineList;
using Audio::ValuePrecision;
using Audio::WaveForm;
using Audio::WaveFormKind;
using Audio::WaveFormIteratorState;
using BaseTypes::Primitives::Percentage;
using SoXPlugins::Effects::SoXFlangerPhaserAndTremolo
      ::SoXFlangerPhaserAndTremolo_AudioEffect;
using SoXPlugins::Helpers::SoXEmulationHelper;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/** abbreviation for adaptValueForSoXEmulation */
static const auto adaptValueForSoXEmulation =
    SoXEmulationHelper::adaptValueForSoXEmulation;

/** abbreviation for soxIsStrictlyEmulated */
static const auto soxIsStrictlyEmulated =
    SoXEmulationHelper::soxIsStrictlyEmulated;

/*============================================================*/

namespace SoXPlugins::Effects::SoXFlangerPhaserAndTremolo {

    /**
     * An <C>_EffectDescriptor_FPHT</C> object is the internal
     * implementation of a effect descriptor type for flanger, phaser
     * and tremolo; both effects share parameters and have similar
     * implementations: hence they can be combined.
     *
     * For the tremolo the phaser parameters are assumed as follows:
     * delay is 0s, inGain and outGain are 1.0, the regeneration is
     * not relevant.
     *
     * For the phaser the tremolo parameter depth is not relevant and
     * ignored.
     */
    struct _EffectDescriptor_FPHT {

        /** information whether effect is a flanger, phaser or
         * tremolo */
        String kind;

        /** the delay in seconds */
        Real delay;

        /** the modulation depth in seconds */
        Real depth;

        /** the modulation depth in percent */
        Percentage depthInPct;

        /** the modulation frequency in Hz */
        Real frequency;

        /** the input gain as a factor */
        Real inGain;

        /** the order of interpolation for the delay lines: 0 is no
         * interpolation, 1 is linear and 2 is quadratic */
        Natural interpolationOrder;

        /** the output gain as a factor */
        Real outGain;

        /** the phase shift as a percent value */
        Percentage phase;

        /** the regeneration as a factor */
        Real regeneration;

        /** the time offset of this effect in seconds */
        Real timeOffset;

        /** the waveform data */
        WaveForm waveForm;

        /** the modulation waveform kind */
        WaveFormKind waveFormKind;

        /** the output gain of the delayed signal as a percent value */
        Percentage width;

        /** the previous data point values for each channel */
        AudioDataPointList previousDataPointList;

        /** the list of delay lines */
        DelayLineList delayLineList;

        /** the delay buffer length in data points */
        Real delayLineLengthInDataPoints;

        /*--------------------*/
        /*--------------------*/

        /**
         * Returns descriptor string representation.
         *
         * @return  string representation of effect
         */
        String toString () const;

        /*--------------------*/

        /**
         * Applies the effect on multiple channel data from
         * <C>inputDataPointList</C> and returns echo result in
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
         * Returns string representation of internal data.
         *
         * @return  string representation
         */
        String internalState () const;

        /*--------------------*/

        /**
         * Recalculates derived parameters in current effect
         * descriptor from parameter map and <C>channelCount</C>,
         * <C>dataPointRate</C> and start time in <C>currentTime</C>.
         *
         * @param[in]  channelCount   the number of channels
         * @param[in]  dataPointRate  new data point rate for effect
         * @param[in]  currentTime    current time for effect
         */
        void updateSettings (IN Natural channelCount,
                             IN Real dataPointRate,
                             IN Real currentTime);

    };

}

/*============================================================*/

using SoXPlugins::Effects::SoXFlangerPhaserAndTremolo
          ::_EffectDescriptor_FPHT;

/*--------------------------------*/
/* support constants and routines */
/*--------------------------------*/

/** pi */
static const Radians _pi = 3.141592653589793238462643383279;

/** the standard starting phase of a flanger (270°) */
static const Radians _defaultPhase_flanger = _pi * 1.5;

/** the standard starting phase of a phaser (90°) */
static const Radians _defaultPhase_phaser  = _pi / 2.0;

/** the standard starting phase of a tremolo (90°) */
static const Radians _defaultPhase_tremolo = _pi / 2.0;

/** the maximum order of interpolation, quadratic */
static const Natural _maximumInterpolationOrder = 2;

/*....................*/
/* EFFECT KINDS       */
/*....................*/

/** the name of the flanger effect */
static const String _EffectKind_flanger = "Flanger";

/** the name of the phaser effect */
static const String _EffectKind_phaser  = "Phaser";

/** the name of the tremolo effect */
static const String _EffectKind_tremolo = "Tremolo";

/** the name of the unknown effect */
static const String _EffectKind_unknown = "Unknown";

/*--------------------*/

/**
 * Calculates the effective width for a flanger effect from
 * <C>width</C> and <C>regeneration</C>.
 *
 * @param[in] width         width of flanger effect
 * @param[in] regeneration  regeneration factor
 */
INLINE
static Real _effectiveWidthForFlanger (IN Percentage width,
                                       IN Real regeneration)
{
    Real result = Percentage::toReal(width);
    result /= (result + Real::one);
    result *= Real::one - regeneration.abs();
    return result;
}

/*--------------------*/

/**
 * Calculates flanger variables <C>phaseOffset</C>,
 * <C>waveFormValue</C>, and <C>delayedDataPoint</C> for single
 * calculation step of effect descriptor <C>descriptor</C> and given
 * <C>channel</C> and <C>interpolationOrder</C> setting.  Phase offset
 * depends on the channel and the descriptor phase variable and tells
 * how much the offset in the wave form is shifted for the given
 * channel.
 *
 * @param[inout]  descriptor          effect descriptor
 * @param[in]     channel             channel to be processed
 * @param[in]     interpolationOrder  order of interpolation
 * @param[out]    phaseOffset         phase offset variable
 * @param[out]    waveFormValue       selected wave form value
 * @param[out]    delayedDataPoint    data point from delay line
 */
INLINE static
void _calculateFlangerVariables (IN _EffectDescriptor_FPHT& descriptor,
                                 IN Natural channel,
                                 IN Natural interpolationOrder,
                                 OUT Real& phaseOffset,
                                 OUT Real& waveFormValue,
                                 OUT Real& delayedDataPoint)
{
    Logging_trace1(">>: %1", TOSTRING(channel));

    const DelayLine& delayLine = descriptor.delayLineList[channel];
    const Real waveFormLength =
        adaptValueForSoXEmulation(descriptor.waveForm.length(), 'r');
    phaseOffset = descriptor.phase.of(waveFormLength * Real{channel});
    phaseOffset = adaptValueForSoXEmulation(phaseOffset, 'r');
    waveFormValue = descriptor.waveForm.at(phaseOffset);

    if (interpolationOrder == 0) {
        /* when interpolation is constant and SoX is strictly
           emulated, round the wave form value, otherwise leave it as
           is */
        waveFormValue = adaptValueForSoXEmulation(waveFormValue, 'r');
    }

    delayedDataPoint = delayLine.at(waveFormValue);

    Logging_trace3("<<: phaseOffset = %1, waveFormValue = %2,"
                   " delayedDataPoint = %3",
                   TOSTRING(phaseOffset), TOSTRING(waveFormValue),
                   TOSTRING(delayedDataPoint));
}
    
/*------------------------*/
/* _EffectDescriptor_FPHT */
/*------------------------*/

String
_EffectDescriptor_FPHT::toString () const
{
    String st1 =
        STR::expand("kind = %1, delay = %2s,"
                    " depth = %3%, depthInPct = %4%,"
                    " frequency = %5Hz, inGain = %6,"
                    " interpolationOrder = %7, outGain = %8,"
                    " phase = %9",
                    kind, TOSTRING(delay),
                    TOSTRING(depth), TOSTRING(depthInPct),
                    TOSTRING(frequency), TOSTRING(inGain),
                    TOSTRING(interpolationOrder), TOSTRING(outGain),
                    TOSTRING(phase));

    String st2 =
        STR::expand(" regeneration = %1, timeOffset = %2,"
                    " width = %3, waveForm = %4,"
                    " previousDataPointList = %5,"
                    " delayLineLengthInDataPoints = %6,"
                    " delayLineList = %7",
                    TOSTRING(regeneration), TOSTRING(timeOffset),
                    TOSTRING(width), waveForm.toString(),
                    previousDataPointList.toString(),
                    TOSTRING(delayLineLengthInDataPoints),
                    delayLineList.toString());

    return STR::expand("_EffectDescriptor_FPHT(%1, %2)",
                       st1, st2);
}

/*--------------------*/

void
_EffectDescriptor_FPHT::applyForFrame
                            (IN AudioDataPointList& inputDataPointList,
                             OUT AudioDataPointList& outputDataPointList)
{
    const Natural channelCount = inputDataPointList.length();
    Real effectiveWidth;

    if (kind == _EffectKind_flanger) {
        effectiveWidth =
            _effectiveWidthForFlanger(width, regeneration);
    }
    
    for (Natural channel = 0;  channel < channelCount; channel++) {
        const AudioDataPoint inputDataPoint = inputDataPointList[channel];
        AudioDataPoint& outputDataPoint     = outputDataPointList[channel];
        DelayLine& delayLine                = delayLineList[channel];

        outputDataPoint = 0.0;

        if (kind == _EffectKind_flanger) {
            Real phaseOffset, waveFormValue;
            AudioDataPoint delayedDataPoint, previousDataPoint;
            _calculateFlangerVariables(*this, channel,
                                       interpolationOrder,
                                       phaseOffset, waveFormValue,
                                       delayedDataPoint);
            previousDataPoint = previousDataPointList.at(channel);
            previousDataPointList.set(channel, delayedDataPoint);
            const AudioDataPoint newDelayedDataPoint =
                inputDataPoint + previousDataPoint * regeneration;
            delayLine.advance(newDelayedDataPoint);
            outputDataPoint =
                inputDataPoint * inGain + delayedDataPoint * effectiveWidth;
        } else if (kind == _EffectKind_phaser) {
            outputDataPoint = inputDataPoint * inGain;

            if (delayLineLengthInDataPoints > 0.0) {
                Real waveFormValue = waveForm.current();
                waveFormValue = adaptValueForSoXEmulation(waveFormValue, 'r');
                outputDataPoint +=
                    delayLine.at(waveFormValue) * regeneration;
                delayLine.advance(outputDataPoint);
            }

            outputDataPoint *= outGain;
        } else if (kind == _EffectKind_tremolo) {
            const Real factor = waveForm.current();
            outputDataPoint = inputDataPoint * factor;
        }

    }

    waveForm.advance();
}

/*--------------------*/

String _EffectDescriptor_FPHT::internalState () const
{
    const Natural channelCount = previousDataPointList.length();
    String st = STR::expand("effectKind = %1", kind);

    if (kind == _EffectKind_tremolo) {
        RealList factorList{channelCount};

        for (Natural channel = 0;  channel < channelCount;  channel++) {
            const Real factor = waveForm.current();
            factorList.set(channel, factor);
        }
        
        st += STR::expand(", factors = %1", factorList.toString(false));
    } else if (kind == _EffectKind_phaser) {
        RealList channelDataPointList{channelCount};
        Real waveFormValue = waveForm.current();
        waveFormValue = adaptValueForSoXEmulation(waveFormValue, 'r');

        for (Natural channel = 0;  channel < channelCount;  channel++) {
            const DelayLine delayLine = delayLineList.at(channel);
            Real channelDataPoint =
                delayLine.at(waveFormValue) * regeneration;
            channelDataPointList.set(channel, channelDataPoint);
        }

        st += STR::expand(", channelDataPoints = %1",
                          channelDataPointList.toString(false));
    } else if (kind == _EffectKind_flanger) {
        RealList delayedDataPointList{channelCount};
        RealList newDataPointList{channelCount};
        RealList phaseOffsetList{channelCount};
        RealList waveFormValueList{channelCount};

        const Real effectiveWidth =
            _effectiveWidthForFlanger(width, regeneration);

        for (Natural channel = 0;  channel < channelCount;  channel++) {
            Real phaseOffset, waveFormValue;
            AudioDataPoint delayedDataPoint, newDataPoint;
            _calculateFlangerVariables(*this, channel,
                                       interpolationOrder,
                                       phaseOffset, waveFormValue,
                                       delayedDataPoint);
            newDataPoint = previousDataPointList[channel] * regeneration;
            delayedDataPointList.set(channel, delayedDataPoint);
            newDataPointList.set(channel, newDataPoint);
            phaseOffsetList.set(channel, phaseOffset);
            waveFormValueList.set(channel, waveFormValue);
        }
        
        st += STR::expand(", effectiveWidth = %1", TOSTRING(effectiveWidth));
        st += STR::expand(", previousDataPoints = %1",
                          previousDataPointList.toString(false));
        st += STR::expand(", phaseOffsets = %1",
                          phaseOffsetList.toString(false));
        st += STR::expand(", waveFormValues = %1",
                          waveFormValueList.toString(false));
        st += STR::expand(", delayedDataPoints = %1",
                          delayedDataPointList.toString(false));
        st += STR::expand(", newDataPointIncrements = %1",
                          newDataPointList.toString(false));
    }

    return st;
}

/*--------------------*/

void _EffectDescriptor_FPHT::updateSettings (IN Natural channelCount,
                                             IN Real dataPointRate,
                                             IN Real currentTime)
{
    Logging_trace3(">>: channelCount = %1, dataPointRate = %2,"
                   " currentTime = %3",
                   TOSTRING(channelCount), TOSTRING(dataPointRate),
                   TOSTRING(currentTime));

    const Real waveFormLength = dataPointRate / frequency;
    Radians defaultPhase;
    Real minimumModulationValue;
    Real maximumModulationValue;

    if (kind == _EffectKind_flanger) {
        inGain = Real::one / (Percentage::toReal(width) + 1.0);
        delayLineLengthInDataPoints = (delay + depth) * dataPointRate;
        delayLineLengthInDataPoints =
            adaptValueForSoXEmulation(delayLineLengthInDataPoints, 'c');
        defaultPhase                = _defaultPhase_flanger;
        /* ensure that minimum modulation value is an integer */
        const Real amplitude = depth * dataPointRate;
        const Real delayLineSlack =
            Real{(soxIsStrictlyEmulated ? _maximumInterpolationOrder : 0)};
        minimumModulationValue      = amplitude.ceiling() + delayLineSlack;
        maximumModulationValue      = minimumModulationValue - amplitude;
        delayLineLengthInDataPoints += delayLineSlack;
        previousDataPointList.setLength(channelCount);
        previousDataPointList.setToZero();
    } else if (kind == _EffectKind_phaser) {
        defaultPhase                = _defaultPhase_phaser;
        delayLineLengthInDataPoints =
            adaptValueForSoXEmulation(delay * dataPointRate, 'r');
        delayLineLengthInDataPoints +=
            Natural{(soxIsStrictlyEmulated ? interpolationOrder : 0)};
        minimumModulationValue      = 0.0;
        maximumModulationValue      = delayLineLengthInDataPoints - 1.0;
    } else if (kind == _EffectKind_tremolo) {
        inGain              = 1.0;
        outGain             = 1.0;
        delay               = 0.0;
        waveFormKind        = WaveFormKind::sine;

        delayLineLengthInDataPoints = 0.0;
        minimumModulationValue      =
            Real{1.0} - Percentage::toReal(depthInPct);
        maximumModulationValue      = 1.0;
        defaultPhase                = _defaultPhase_tremolo;
    }

    /* delay settings */

    /* flanger and phaser in SoX apply an interpolation using later
     * points in the delay line*/
    const ListInterpolatorPointLocation pointLocation =
        (soxIsStrictlyEmulated && interpolationOrder == 1
         ? ListInterpolatorPointLocation::atBegin
         : ListInterpolatorPointLocation::atCenter);
    delayLineList.setElementLength(delayLineLengthInDataPoints);
    delayLineList.setElementInterpolation(interpolationOrder,
                                          pointLocation);
    delayLineList.setToZero();

    /* waveform */
    if (soxIsStrictlyEmulated && kind != _EffectKind_tremolo) {
        /* adapt default phase for flanger and phaser because of some
           subsequent rounding in the SoX lsx_generate_wave_table
           function; tremolo does not use that algorithm, hence its
           phase _must not_ be adapted */
        const Real twoPi = _pi * 2.0;
        const Real phaseOffset =
            Real::round(defaultPhase / twoPi * waveFormLength);
        defaultPhase = phaseOffset * twoPi / waveFormLength;
    }

    const Radians effectivePhase =
        (defaultPhase
         + WaveForm::phaseByTime(frequency, timeOffset, currentTime));

    const ValuePrecision precision = ValuePrecision::singlePrecision;
    waveForm.set(waveFormLength, waveFormKind,
                 minimumModulationValue, maximumModulationValue,
                 effectivePhase, precision);

    Logging_trace1("<<: %1", toString());
}

/*====================*/

/** the maximum allowable delay in seconds */
static const Real _maximumDelay = 1.0;

/** the internal separator for lists */
static const String separator = "/";

/** the list of effect kinds in this plugin */
static const StringList _effectKindList =
    StringList::fromList({_EffectKind_flanger, _EffectKind_phaser,
                          _EffectKind_tremolo});

/*....................*/
/* parameter names    */
/*....................*/

/** the name of the delay parameter */
static const String parameterName_delay             = "Delay [ms]";

/** the name of the depth parameter */
static const String parameterName_depthInMs         = "Depth [ms]";

/** the name of the depth parameter */
static const String parameterName_depthInPct        = "Depth [%]";

/** the name of the effect kind parameter */
static const String parameterName_effectKind        = "Effect Kind";

/** the name of the frequency parameter */
static const String parameterName_frequency         = "Frequency [Hz]";

/** the alias parameter name of the frequency parameter */
static const String parameterName_frequency_OLD     = "Modulation [Hz]";

/** the name of the input gain parameter */
static const String parameterName_inGain            = "In Gain";

/** the name of the interpolation order parameter */
static const String parameterName_interpolationOrder =
    "Interp. Order";

/** the name of the output gain parameter */
static const String parameterName_outGain           = "Out Gain";

/** the name of the phase parameter */
static const String parameterName_phase             = "Phase [%]";

/** the name of the regeneration parameter */
static const String parameterName_regeneration      = "Regeneration";

/** the original name of the regeneration parameter */
static const String parameterName_regeneration_OLD  = "Decay";

/** the original name of the regeneration parameter */
static const String parameterName_regenerationInPct = "Regeneration [%]";

/** the name of the time offset parameter */
static const String parameterName_timeOffset        = "Time Offset [s]";

/** the name of the waveform kind parameter */
static const String parameterName_waveFormKind      = "Waveform";

/** the name of the width parameter */
static const String parameterName_width             = "Width [%]";

/** the list of all parameter names */
static const StringList _allParameterNameList =
    StringList::fromList({parameterName_delay,
                          parameterName_depthInMs,
                          parameterName_depthInPct,
                          parameterName_effectKind,
                          parameterName_frequency,
                          parameterName_inGain,
                          parameterName_interpolationOrder,
                          parameterName_outGain,
                          parameterName_phase,
                          parameterName_regeneration,
                          parameterName_regenerationInPct,
                          parameterName_timeOffset,
                          parameterName_waveFormKind,
                          parameterName_width});

/** the list of the flanger parameter names in appearance order */
static const StringList _flangerParameterNameList =
    StringList::fromList({parameterName_effectKind,
                          parameterName_delay,
                          parameterName_depthInMs,
                          parameterName_regenerationInPct,
                          parameterName_width,
                          parameterName_frequency,
                          parameterName_waveFormKind,
                          parameterName_phase,
                          parameterName_interpolationOrder,
                          parameterName_timeOffset});

/** the list of the phaser parameter names in appearance order */
static const StringList _phaserParameterNameList =
    StringList::fromList({parameterName_effectKind,
                          parameterName_inGain,
                          parameterName_outGain,
                          parameterName_delay,
                          parameterName_regeneration,
                          parameterName_frequency,
                          parameterName_waveFormKind,
                          parameterName_interpolationOrder,
                          parameterName_timeOffset});

/** the list of the tremolo parameter names in appearance order */
static const StringList _tremoloParameterNameList =
    StringList::fromList({parameterName_effectKind,
                          parameterName_frequency,
                          parameterName_depthInPct,
                          parameterName_timeOffset});

/*--------------------*/
/* internal routines  */
/*--------------------*/

/**
 * Sets up a new phaser/tremolo effect descriptor based on
 * <C>dataPointRate</C>
 *
 * @param[in] dataPointRate  the data point rate for this effect
 * @return  new phaser/tremolo effect descriptor
 */
static
_EffectDescriptor_FPHT* _createEffectDescriptor (IN Real dataPointRate)
{
    Logging_trace1(">>: %1", TOSTRING(dataPointRate));

    const Natural maximumDelayBufferLength =
        Natural(Real::ceiling(_maximumDelay * dataPointRate));

    _EffectDescriptor_FPHT* result =
        new _EffectDescriptor_FPHT{
            _EffectKind_unknown,            /* kind of effect */

            0.003,                          /* delay */
            2.0,                            /* depth in s */
            40.0,                           /* depth in pct */
            0.5,                            /* frequency */
            0.4,                            /* inGain */
            0,                              /* interpolationOrder */
            0.74,                           /* outGain */
            0.25,                           /* phase */
            0.4,                            /* regeneration */
            0.0,                            /* timeOffset */
            {},                             /* waveForm */
            WaveFormKind::sine,             /* waveFormKind */
            0.0,                            /* width */

            {},                             /* previousDataPointList */
            {2, maximumDelayBufferLength},  /* delayLineList */
            maximumDelayBufferLength        /* delayLineLengthInDataPoints */
        };

    
    Logging_trace1("<<: %1",
                   result->toString());
    return result;
}

/*============================================================*/

/*---------------------*/
/* setup & destruction */
/*---------------------*/

SoXFlangerPhaserAndTremolo_AudioEffect::
SoXFlangerPhaserAndTremolo_AudioEffect ()
{
    Logging_trace(">>");

    /* initialize descriptor */
    _effectDescriptor = _createEffectDescriptor(_dataPointRate);

    /* initialize parameters */
    _updateEffectForKind(_EffectKind_unknown);
    setDefaultValues();

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXFlangerPhaserAndTremolo_AudioEffect::
~SoXFlangerPhaserAndTremolo_AudioEffect ()
{
    Logging_trace(">>");
    delete (_EffectDescriptor_FPHT*) _effectDescriptor;
    Logging_trace("<<");
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXFlangerPhaserAndTremolo_AudioEffect::
_effectDescriptorToString () const
{
    String result;

    if (_effectDescriptor == nullptr) {
        result = "NULL";
    } else {
        const _EffectDescriptor_FPHT& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_FPHT>(_effectDescriptor);

        result = effectDescriptor.toString();
    }

    return result;
}

/*-----------------------*/

String SoXFlangerPhaserAndTremolo_AudioEffect::
_internalState () const
{
    String result;

    if (_effectDescriptor == nullptr) {
        result = "";
    } else {
        const _EffectDescriptor_FPHT& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_FPHT>(_effectDescriptor);

        result = effectDescriptor.internalState();
    }

    return result;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

Boolean SoXFlangerPhaserAndTremolo_AudioEffect::isTimeVariant () const
{
    return true;
}

/*--------------------*/

String SoXFlangerPhaserAndTremolo_AudioEffect::name () const
{
    return "SoX FlangerPhaserAndTremolo";
}

/*--------------------*/

Real SoXFlangerPhaserAndTremolo_AudioEffect::tailLength () const
{
    const _EffectDescriptor_FPHT& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FPHT>(_effectDescriptor);
    return effectDescriptor.delay;
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXValueChangeKind SoXFlangerPhaserAndTremolo_AudioEffect::
_setValueInternal (IN String& parameterName,
                   IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    _EffectDescriptor_FPHT& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FPHT>(_effectDescriptor);
    SoXValueChangeKind result = SoXValueChangeKind::parameterChange;

    if (parameterName == parameterName_effectKind) {
        const String previousKind = effectDescriptor.kind;

        if (value != _EffectKind_unknown && value != previousKind) {
            _updateEffectForKind(value);
            effectDescriptor.kind = value;
            setDefaultValues();
            result = SoXValueChangeKind::globalChange;
        }
    } else if (!_effectParameterMap.isActive(parameterName)) {
        Logging_trace("--: parameter is inactive -> ignored");
    } else {
        if (parameterName == parameterName_delay) {
            effectDescriptor.delay = STR::toReal(value) / 1000.0;
        } else if (parameterName == parameterName_depthInMs) {
            effectDescriptor.depth = STR::toReal(value) / 1000.0;
        } else if (parameterName == parameterName_depthInPct) {
            effectDescriptor.depthInPct = STR::toPercentage(value);
        } else if (parameterName == parameterName_frequency) {
            effectDescriptor.frequency = STR::toReal(value);
        } else if (parameterName == parameterName_inGain) {
            effectDescriptor.inGain = STR::toReal(value);
        } else if (parameterName == parameterName_interpolationOrder) {
            effectDescriptor.interpolationOrder = STR::toNatural(value);
        } else if (parameterName == parameterName_outGain) {
            effectDescriptor.outGain = STR::toReal(value);
        } else if (parameterName == parameterName_phase) {
            effectDescriptor.phase = STR::toPercentage(value);
        } else if (parameterName == parameterName_regeneration) {
            effectDescriptor.regeneration = STR::toReal(value);
        } else if (parameterName == parameterName_regenerationInPct) {
            /* adapt from percent to factor */
            effectDescriptor.regeneration =
                Percentage::toReal(STR::toReal(value));
        } else if (parameterName == parameterName_timeOffset) {
            effectDescriptor.timeOffset = STR::toReal(value);
        } else if (parameterName == parameterName_waveFormKind) {
            effectDescriptor.waveFormKind =
                Audio::stringToWaveFormKind(value);
        } else if (parameterName == parameterName_width) {
            effectDescriptor.width = STR::toPercentage(value);
        }
    }

    Logging_trace1("<<: %1", SoXValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXFlangerPhaserAndTremolo_AudioEffect::setDefaultValues ()
{
    Logging_trace(">>");

    Dictionary dictionary;
    const String effectKind =
        _effectParameterMap.value(parameterName_effectKind);

    dictionary.set(parameterName_effectKind, effectKind);

    if (effectKind == _EffectKind_flanger) {
        dictionary.set(parameterName_delay,              "0.0");
        dictionary.set(parameterName_depthInMs,          "2");
        dictionary.set(parameterName_regenerationInPct,  "0.0");
        dictionary.set(parameterName_width,              "71");
        dictionary.set(parameterName_frequency,          "0.5");
        dictionary.set(parameterName_waveFormKind,       "Sine");
        dictionary.set(parameterName_phase,              "25");
        dictionary.set(parameterName_interpolationOrder, "0");
    } else if (effectKind == _EffectKind_phaser) {
        dictionary.set(parameterName_inGain,             "0.4");
        dictionary.set(parameterName_outGain,            "0.74");
        dictionary.set(parameterName_delay,              "3.0");
        dictionary.set(parameterName_regeneration,       "0.4");
        dictionary.set(parameterName_frequency,          "0.5");
        dictionary.set(parameterName_waveFormKind,       "Triangle");
        dictionary.set(parameterName_interpolationOrder, "0");
    } else if (effectKind == _EffectKind_tremolo) {
        dictionary.set(parameterName_frequency,          "0.5");
        dictionary.set(parameterName_depthInPct,         "40");
    }

    dictionary.set(parameterName_timeOffset, "0");
    setValues(dictionary);
    recalculate();

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXFlangerPhaserAndTremolo_AudioEffect::
_applyForFrame (IN AudioDataPointList& inputDataPointList,
                OUT AudioDataPointList& outputDataPointList)
{
    Logging_trace(">>");

    _EffectDescriptor_FPHT& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FPHT>(_effectDescriptor);
    effectDescriptor.applyForFrame(inputDataPointList, outputDataPointList);

    Logging_trace("<<");
}

/*--------------------*/

void SoXFlangerPhaserAndTremolo_AudioEffect::
_handleDataPointRateChange (IN Real dataPointRate)
{
    Logging_trace1(">>: %1", TOSTRING(dataPointRate));
    recalculate();
    Logging_trace("<<");
}

/*--------------------*/

void SoXFlangerPhaserAndTremolo_AudioEffect::recalculate ()
{
    Logging_trace(">>");

    setParameterValidity(false);
    _EffectDescriptor_FPHT& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FPHT>(_effectDescriptor);
    effectDescriptor.updateSettings(_channelCount, _dataPointRate,
                                    _currentTimePosition);
    setParameterValidity(true);

    Logging_trace("<<");
}

/*--------------------*/

void SoXFlangerPhaserAndTremolo_AudioEffect::
_updateEffectForKind (IN String& effectKind)
{
    Logging_trace1(">>: %1", effectKind);

    SoXEffectParameterMap& parameterMap = _effectParameterMap;
    const StringList waveFormKindValueList =
        StringList::makeBySplit("Sine/Triangle", separator);

    parameterMap.clear();
    parameterMap.setKindEnum(parameterName_effectKind, _effectKindList);
    parameterMap.setValue(parameterName_effectKind, effectKind);

    parameterMap.setKindReal(parameterName_delay, 0.0, 5.0, 0.001);
    parameterMap.setKindReal(parameterName_depthInMs,
                             0.0, 10.0, 0.001);
    parameterMap.setKindReal(parameterName_depthInPct,
                             0.0, 100.0, 0.001);
    parameterMap.setKindReal(parameterName_frequency, 0.1, 2.0, 0.001);
    parameterMap.setKindReal(parameterName_inGain, 0.0, 1.0, 0.001);
    parameterMap.setKindInt(parameterName_interpolationOrder,
                            0, _maximumInterpolationOrder, 1);
    parameterMap.setKindReal(parameterName_outGain, 0.0, 1000.0, 0.001);
    parameterMap.setKindReal(parameterName_phase, 0.0, 100.0, 0.001);
    parameterMap.setKindReal(parameterName_regeneration,
                             0.0, 0.99, 0.001);
    parameterMap.setKindReal(parameterName_regenerationInPct,
                             -95.0, 95.0, 0.001);
    parameterMap.setKindReal(parameterName_timeOffset,
                             -8192.0, +8192.0,
                             Real::two.power(-16.0));
    parameterMap.setKindEnum(parameterName_waveFormKind,
                             waveFormKindValueList);
    parameterMap.setKindReal(parameterName_width, 0.0, 100.0, 0.001);

    StringList parameterNameList;

    if (effectKind == _EffectKind_unknown) {
        Logging_trace("--: effect kind is unknown -> no change");
    } else if (effectKind == _EffectKind_flanger) {
        parameterNameList = _flangerParameterNameList;
        parameterMap.setKindReal(parameterName_delay,
                                 0.0, 30.0, 0.001);
        parameterMap.setKindReal(parameterName_frequency,
                                 0.1, 10.0, 0.001);
    } else if (effectKind == _EffectKind_phaser) {
        parameterNameList = _phaserParameterNameList;
        parameterMap.setKindReal(parameterName_delay,
                                 0.0, 5.0, 0.001);
        parameterMap.setKindReal(parameterName_frequency,
                                 0.1, 2.0, 0.001);
    } else if (effectKind == _EffectKind_tremolo) {
        parameterNameList = _tremoloParameterNameList;
        parameterMap.setKindReal(parameterName_frequency,
                                 0.0, 20.0, 0.001);
    }

    /* set the parameter activeness by effect kind and update
       ranges of parameters */
    parameterMap.setActivenessForNameList(_allParameterNameList,
                                          false);
    parameterMap.setActivenessForNameList(parameterNameList, true);
    parameterMap.invalidateValues(parameterNameList);
    parameterMap.setValue(parameterName_effectKind, effectKind);
    parameterMap.reorderByNameList(parameterNameList);

    /* add alias parameter names */
    parameterMap.setAsAlias(parameterName_regeneration_OLD,
                            parameterName_regeneration);
    parameterMap.setAsAlias(parameterName_frequency_OLD,
                            parameterName_frequency);

    Logging_trace1("<<: parameterMap = %1", parameterMap.toString());
}
