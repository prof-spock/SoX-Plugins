/**
 * @file
 * The <C>SoXFilter_AudioEffect</C> body implements a universal SoX
 * filter plugin representing different IIR filters: <B>allpass</B>,
 * <B>band</B>, <B>bandpass</B>, <B>bandreject</B>, <B>bass</B>,
 * <B>biquad</B>, <B>equalizer</B>, <B>highpass</B>, <B>lowpass</B>
 * and <B>treble</B>.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-11
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXFilter_AudioEffect.h"

#include <cmath>

#include "AudioSampleRingBufferVector.h"
#include "Dictionary.h"
#include "FilterBandwidthUnit.h"
#include "IIRFilter.h"
#include "Logging.h"
#include "SoXAudioHelper.h"

/*--------------------*/

using Audio::AudioSampleRingBufferVector;
using Audio::FilterBandwidthUnit;
using Audio::IIRFilter;
using BaseTypes::Containers::Dictionary;
using SoXPlugins::Effects::SoXFilter::SoXFilter_AudioEffect;
using SoXPlugins::Helpers::SoXAudioHelper;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/*============================================================*/

/** the filter order of a biquad filter */
static const Natural _biquadFilterOrder = 3;

/** the universal separator character */
static const String separator = "/";

/** the comma character */
static const String comma = ",";

/** the list of answers in a yes-no-combobox (in English language) */
static const StringList _yesNoList =
    StringList::makeBySplit("Yes/No", separator);

/*....................*/
/* FILTER KINDS       */
/*....................*/

/** the name of the allpass filter */
static const String filterKind_allpass    = "Allpass";

/** the name of the band filter */
static const String filterKind_band       = "Band";

/** the name of the bass filter */
static const String filterKind_bass       = "Bass";

/** the name of the bandpass filter */
static const String filterKind_bandpass   = "BandPass";

/** the name of the bandreject filter */
static const String filterKind_bandreject = "BandReject";

/** the name of the biquad filter */
static const String filterKind_biquad     = "Biquad";

/** the name of the equalizer filter */
static const String filterKind_equalizer  = "Equalizer";

/** the name of the highpass filter */
static const String filterKind_highpass   = "HighPass";

/** the name of the lowpass filter */
static const String filterKind_lowpass    = "LowPass";

/** the name of the treble filter */
static const String filterKind_treble     = "Treble";

/** the list of all filter kinds as a string (in English language) */
static const StringList _kindList =
    StringList::fromList({filterKind_allpass, filterKind_band,
                          filterKind_bass, filterKind_bandpass,
                          filterKind_bandreject, filterKind_biquad,
                          filterKind_equalizer, filterKind_highpass,
                          filterKind_lowpass, filterKind_treble});

/*--------------------*/

/** the parameter names of a biquad filter */
static const StringList _biquadFilterParameterNameList =
    StringList::fromList({"b0", "b1", "b2", "a0", "a1", "a2"});

/*----------------------*/
/* FORWARD DECLARATIONS */
/*----------------------*/

static Real _alphaForBandwidth (IN Real sampleRate,
                                IN Real bandwidth,
                                IN FilterBandwidthUnit unit,
                                IN Real frequency,
                                IN Real dBGain);

static String _bwUnitToString (IN FilterBandwidthUnit value);

static StringList _unitCodeListForKind (IN String& filterKind);

/*============================================================*/

namespace SoXPlugins::Effects::SoXFilter {

    /**
     * An <C>_EffectDescriptor_FLTR</C> object is the internal
     * implementation of a biquad filter effect descriptor type where
     * all sample input is routed to and sample output is routed from.
     */
    struct _EffectDescriptor_FLTR {

        /** the associated filter effect */
        SoXFilter_AudioEffect& effect;

        /** the filter kind (as a string) */
        String kind;

        Real b0; /**< IIR filter coefficient b0 */
        Real b1; /**< IIR filter coefficient b1 */
        Real b2; /**< IIR filter coefficient b2 */
        Real a0; /**< IIR filter coefficient a0 */
        Real a1; /**< IIR filter coefficient a1 */
        Real a2; /**< IIR filter coefficient a2 */

        /** a vector of sample ring buffers for the IIR filter */
        AudioSampleRingBufferVector sampleRingBufferVector;

        /** the underlying IIR filter */
        IIRFilter filter;

        /** the characteristic frequency of the filter */
        Real frequency;

        /** the nominal value of the bandwidth of the filter */
        Real bandwidth;

        /** the unit of the bandwidth of the filter */
        FilterBandwidthUnit bandwidthUnit;

        /** the filter gain */
        Real dBGain;

        /** the gain for an equalizer */
        Real equGain;

        /** tells whether filter uses mode for unpitched audio (for a
         *  band filter) */
        Boolean usesUnpitchedAudioMode;

        /** tells whether filter uses constant skirt gain (for a
         * bandpass) */
        Boolean usesConstantSkirtGain;

        /** tells whether filter is a 1-pole filter */
        Boolean isSinglePole;

        /*--------------------*/
        /*--------------------*/

        /**
         * Sets up a new effect descriptor for <C>effect</C>
         *
         * @param[inout] effect  associated filter effect
         */
        _EffectDescriptor_FLTR (INOUT SoXFilter_AudioEffect& effect);

        /*--------------------*/

        /**
         * Recalculates IIR filter coefficients from other parameters
         * and <C>sampleRate</C> and updates IIR filter accordingly.
         *
         * @param[in] sampleRate  new sample rate
         */
        void updateSettings (IN Real sampleRate);

        /*--------------------*/

        /**
         * Returns string representation of descriptor
         *
         * @return string representation
         */
        String toString () const;

    };
}

/*============================================================*/

using SoXPlugins::Effects::SoXFilter::_EffectDescriptor_FLTR;

/*------------------------*/
/* _EffectDescriptor_FLTR */
/*------------------------*/

_EffectDescriptor_FLTR::
_EffectDescriptor_FLTR (INOUT SoXFilter_AudioEffect& effect)
    : effect(effect),
      kind{filterKind_biquad},
      b0{0.0}, b1{0.0}, b2{0.0}, a0{0.0}, a1{0.0}, a2{0.0},
      sampleRingBufferVector{2, true, _biquadFilterOrder},
      filter{_biquadFilterOrder},
      frequency{1000.0},
      bandwidth{1.5},
      bandwidthUnit{FilterBandwidthUnit::slope},
      dBGain{0.0},
      equGain{0.0},
      usesUnpitchedAudioMode{false},
      usesConstantSkirtGain{false},
      isSinglePole{true}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

void _EffectDescriptor_FLTR::updateSettings (IN Real sampleRate)
{
    Logging_trace2(">>: kind = %1, sampleRate = %2",
                   kind, TOSTRING(sampleRate));

    effect.setParameterValidity(false);

    if (kind == filterKind_biquad) {
        /* we can just keep the direct coefficients, no
           calculation is necessary */
    } else {
        const Real four{4.0};
        const Real ten{10.0};

        const Real w0 = Real::twoPi * frequency / sampleRate;
        const Real cw0 = Real::cos(w0);
        const Real sw0 = Real::sin(w0);
        const Real alpha = _alphaForBandwidth(sampleRate,
                                              bandwidth,
                                              bandwidthUnit,
                                              frequency,
                                              dBGain);
        const Real a = SoXAudioHelper::dBToLinear(dBGain, 40.0);

        if (kind == filterKind_allpass) {
            b0 =  Real::one - alpha;
            b1 = -Real::two * cw0;
            b2 =  Real::one + alpha;
            a0 =  b2;
            a1 =  b1;
            a2 =  b0;
        } else if (kind == filterKind_band) {
            const Real bandwidthAsFrequency =
                     (bandwidthUnit == FilterBandwidthUnit::quality
                      ? frequency / bandwidth
                      : (bandwidthUnit == FilterBandwidthUnit::octaves
                         ? Real{(frequency * Real::two.power(bandwidth - Real::one)
                                 * Real::two.power(-bandwidth / Real::two))}
                         : bandwidth));
            a2 = (-Real::twoPi * bandwidthAsFrequency / sampleRate).exp();
            a1 = -four * a2 / (Real::one + a2) * cw0;
            a0 = Real::one;
            b2 = Real::zero;
            b1 = Real::zero;
            b0 = Real::sqrt(Real::one - a1.sqr()
                            / (four * a2)) * (Real::one - a2);

            if (usesUnpitchedAudioMode) {
                const Real factor =
                    Real::sqrt(((Real::one + a2).sqr() - a1.sqr())
                               * (Real::one - a2) / (Real::one + a2))
                    / b0;
                b0 *= factor;
            }
        } else if (kind == filterKind_bandpass
                   || kind == filterKind_bandreject) {
            if (kind == filterKind_bandreject) {
                b0 =  Real::one;
                b1 = -cw0 * Real::two;
                b2 =  Real::one;
            } else {
                b0 =  (usesConstantSkirtGain ? sw0 / Real::two : alpha);
                b1 =  Real::zero;
                b2 = -b0;
            }

            a0 =  alpha + Real::one;
            a1 = -cw0 * Real::two;
            a2 = -alpha + Real::one;
        } else if (kind == filterKind_bass || kind == filterKind_treble) {
            const Real f = (kind == filterKind_bass ? Real::one : -Real::one);
            const Real sqrtAlphaA = Real::two * Real::sqrt(a) * alpha;
            const Real aP1 = a + Real::one;
            const Real aM1 = a - Real::one;
            const Real twoF = Real::two * f;
            b0 =         a * ( (aP1) - f * (aM1) * cw0 + sqrtAlphaA );
            b1 =  twoF * a * ( (aM1) - f * (aP1) * cw0              );
            b2 =         a * ( (aP1) - f * (aM1) * cw0 - sqrtAlphaA );
            a0 =               (aP1) + f * (aM1) * cw0 + sqrtAlphaA;
            a1 = -twoF *     ( (aM1) + f * (aP1) * cw0              );
            a2 =               (aP1) + f * (aM1) * cw0 - sqrtAlphaA;
        } else if (kind == filterKind_equalizer) {
            const Real filterGain = ten.power(equGain / 40.0);
            b0 = Real::one + alpha * filterGain;
            b1 = -Real::two * cw0;
            b2 = Real::one - alpha * filterGain;
            a0 = Real::one + alpha / filterGain;
            a1 = b1;
            a2 = Real::one - alpha / filterGain;
        } else if (kind == filterKind_highpass
                   || kind == filterKind_lowpass) {
            Real factorA, factorB, factorC;

            if (isSinglePole) {
                if (kind == filterKind_highpass) {
                    factorA = -Real::one;
                    factorB = 0.5;
                    factorC = -Real::one;
                } else {
                    factorA = Real::one;
                    factorB = Real::one;
                    factorC = Real::zero;
                }

                a0 = Real::one;
                a1 = -Real::exp(-w0);
                a2 = Real::zero;
                b0  = (Real::one + factorA * a1) * factorB;
                b1  = factorC * b0;
                b2  = Real::zero;
            } else {
                if (kind == filterKind_highpass) {
                    factorA = (Real::one + cw0);
                    factorB = -Real::one;
                } else {
                    factorA = (Real::one - cw0);
                    factorB = Real::one;
                }

                b0  = factorA / Real::two;
                b1  = factorB * factorA;
                b2  = b0;
                a0 = Real::one + alpha;
                a1 = -Real::two * cw0;
                a2 = Real::one - alpha;
            }
        }
    }

    filter.set(b0, b1, b2, a0, a1, a2);
    effect.setParameterValidity(true);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

String _EffectDescriptor_FLTR::toString () const
{
    String st1 =
        STR::expand("kind = %1, frequency = %2Hz,"
                    " bandwidth = %3%4,"
                    " dBGain = %5dB, equGain = %6dB,"
                    " usesUnpitchedAudioMode = %7,"
                    " usesConstantSkirtGain = %8,"
                    " isSinglePole = %9",
                    kind, TOSTRING(frequency),
                    TOSTRING(bandwidth), _bwUnitToString(bandwidthUnit),
                    TOSTRING(dBGain), TOSTRING(equGain),
                    TOSTRING(usesUnpitchedAudioMode),
                    TOSTRING(usesConstantSkirtGain),
                    TOSTRING(isSinglePole));

    String st2 =
        STR::expand("b0 = %1, b1 = %2, b2 = %3,"
                    " a0 = %4, a1 = %5, a2 = %6,"
                    " filter = %7, sampleRingBufferVector = %8",
                    TOSTRING(b0), TOSTRING(b1), TOSTRING(b2),
                    TOSTRING(a0), TOSTRING(a1), TOSTRING(a2),
                    filter.toString(),
                    sampleRingBufferVector.toString());

    return STR::expand("_EffectDescriptor_FLTR(%1, %2)", st1, st2);
}

/*--------------------*/
/* internal features  */
/*--------------------*/

/*====================*/

/*....................*/
/* PARAMETER NAMES    */
/*....................*/

/** the parameter name of the bandwidth parameter */
static const String parameterName_bandwidth     = "Bandwidth";

/** the parameter name of the bandwidthUnit parameter */
static const String parameterName_bandwidthUnit = "Bandwidth Unit";

/** the parameter name of the cstSkirtGain parameter */
static const String parameterName_cstSkirtGain  = "Cst. Skirt Gain?";

/** the parameter name of the dBGain parameter */
static const String parameterName_dBGain        = "Gain [dB]";

/** the parameter name of the equGain parameter */
static const String parameterName_equGain       = "Eq. Gain [dB]";

/** the parameter name of the frequency parameter */
static const String parameterName_frequency     = "Frequency [Hz]";

/** the parameter name of the kind parameter */
static const String parameterName_kind          = "Filter Kind";

/** the parameter name of the poleCount parameter */
static const String parameterName_poleCount     = "Number of Poles";

/** the parameter name of the unpitchedMode parameter */
static const String parameterName_unpitchedMode = "Unpitched Mode?";

/*....................*/
/* BANDWIDTH UNITS    */
/*....................*/

/** the name of the butterworth bandwidth unit */
static const String _bwUnitText_butterworth = "Butterworth";

/** the name of the frequency bandwidth unit */
static const String _bwUnitText_frequency   = "Frequency";

/** the name of the octave bandwidth unit */
static const String _bwUnitText_octave      = "Octave(s)";

/** the name of the quality bandwidth unit */
static const String _bwUnitText_quality     = "Quality";

/** the name of the slope bandwidth unit */
static const String _bwUnitText_slope       = "Slope";

/** the list of all bandwidth units */
static const StringList _defaultBandwidthUnitTextList =
    StringList::fromList({_bwUnitText_butterworth,
                          _bwUnitText_frequency,
                          _bwUnitText_octave,
                          _bwUnitText_quality,
                          _bwUnitText_slope});

/*....................*/
/* PARAMETER SETS     */
/*....................*/

/** the character flag for the bandwidth parameter set in a dialog */
static const String paramFlag_bandwidth     = "B";

/** the character flag for the biquad parameter set in a dialog */
static const String paramFlag_biquad        = "Q";

/** the character flag for the cstSkirtGain parameter set in a dialog */
static const String paramFlag_cstSkirtGain  = "C";

/** the character flag for the dBGain parameter set in a dialog */
static const String paramFlag_dBGain        = "D";

/** the character flag for the equGain parameter set in a dialog */
static const String paramFlag_equGain       = "E";

/** the character flag for the frequency parameter set in a dialog */
static const String paramFlag_frequency     = "F";

/** the character flag for the poleCount parameter set in a dialog */
static const String paramFlag_poleCount     = "P";

/** the character flag for the unpitchedMode parameter set in a dialog */
static const String paramFlag_unpitchedMode = "U";

/**
 * mapping from filter kind to a slash-delimited string of code
 * letters: "Q" is the biquad parameter set, "F" the frequency widget,
 * "B" the bandwidth widgets, "U" for the unpitched audio mode etc.
*/
static const Dictionary _filterKindToWidgetDataMap =
    Dictionary::fromList(
        StringList::makeBySplit(
            filterKind_allpass    + comma + "F/B"   + comma +
            filterKind_band       + comma + "U/F/B" + comma +
            filterKind_bandpass   + comma + "C/F/B" + comma +
            filterKind_bandreject + comma + "F/B"   + comma +
            filterKind_bass       + comma + "D/F/B" + comma +
            filterKind_biquad     + comma + "Q"     + comma +
            filterKind_equalizer  + comma + "F/B/E" + comma +
            filterKind_highpass   + comma + "P/F/B" + comma +
            filterKind_lowpass    + comma + "P/F/B" + comma +
            filterKind_treble     + comma + "D/F/B",
            comma)
        );

/*.........................*/
/* ACCEPTABLE FILTER UNITS */
/*.........................*/

/** the character flag for the frequency bandwidth unit */
static const String unitFlag_frequency   = "f";

/** the character flag for the octave bandwidth unit */
static const String unitFlag_octave      = "o";

/** the character flag for the quality bandwidth unit */
static const String unitFlag_quality     = "q";

/** the character flag for the butterworth bandwidth unit */
static const String unitFlag_butterworth = "b";

/** the character flag for the slope bandwidth unit */
static const String unitFlag_slope       = "s";

/**
 * mapping from filter kind to a slash-delimited string of code
 * letters for the acceptable filter units; allowed units are
 * frequency ("f"), octave ("o"), quality ("q"), butterworth ("b") and
 * slope ("s")
 */
static const Dictionary _filterKindToUnitMap =
    Dictionary::fromList(
        StringList::makeBySplit(
            filterKind_allpass    + comma + "f/o/q/b"   + comma +
            filterKind_band       + comma + "f/o/q/b"   + comma +
            filterKind_bandpass   + comma + "f/o/q/b"   + comma +
            filterKind_bandreject + comma + "f/o/q/b"   + comma +
            filterKind_bass       + comma + "f/o/q/b/s" + comma +
            filterKind_equalizer  + comma + "f/o/q/b"   + comma +
            filterKind_highpass   + comma + "f/o/q/b"   + comma +
            filterKind_lowpass    + comma + "f/o/q/b"   + comma +
            filterKind_treble     + comma + "f/o/q/b/s",
            comma)
        );

/** mapping from unit code letter to text for combo boxes */
static const Dictionary _unitCodeToTextMap =
    Dictionary::fromList(
        StringList::makeBySplit(
            "b" + comma + _bwUnitText_butterworth + comma +
            "f" + comma + _bwUnitText_frequency   + comma +
            "o" + comma + _bwUnitText_octave      + comma +
            "q" + comma + _bwUnitText_quality     + comma +
            "s" + comma + _bwUnitText_slope,
            comma)
        );

/*--------------------*/
/* internal routines  */
/*--------------------*/

/**
 * Returns the alpha for given <C>bandwidth</C> measured as
 * <C>unit</C> with <C>frequency</C> and <C>dBgain</C>;
 * <C>sampleRate</C> gives the current samplerate for the effect.
 *
 * @param[in] sampleRate  current effect sample rate
 * @param[in] bandWidth   the nominal of the filter band width
 * @param[in] unit        the unit of the filter band width
 * @param[in] frequency   the characteristic frequency of the filter
 * @param[in] dBGain      the filter gain (in decibels)
 * @return  alpha parameter of filter
 */
static Real _alphaForBandwidth (IN Real sampleRate,
                                IN Real bandwidth,
                                IN FilterBandwidthUnit unit,
                                IN Real frequency,
                                IN Real dBGain)
{
    Logging_trace5(">>: sampleRate = %1, bandwidth = %2%3,"
                   " frequency = %4, gain = %5",
                   TOSTRING(sampleRate), TOSTRING(bandwidth),
                   _bwUnitToString(unit), TOSTRING(frequency),
                   TOSTRING(dBGain));

    const Real w0 = Real::twoPi * frequency / sampleRate;
    const Real sinW0 = w0.sin();
    const Real one{1.0};
    const Real two{2.0};
    const Real oneHalf{0.5};
    Real alpha = 0.0;

    switch(unit) {
        case FilterBandwidthUnit::quality:
            alpha = sinW0 / (two * bandwidth);
            break;

        case FilterBandwidthUnit::octaves:
            alpha = sinW0 * Real::sinh(two.log() / two
                                       * (bandwidth * w0 / sinW0));
            break;

        case FilterBandwidthUnit::butterworth:
            alpha = sinW0 / (two * oneHalf.sqrt());
            break;

        case FilterBandwidthUnit::frequency:
            alpha = sinW0 / (two * frequency / bandwidth);
            break;

        case FilterBandwidthUnit::slope:
            const Real a = SoXAudioHelper::dBToLinear(dBGain, 40.0);
            alpha = (sinW0 / two
                     * Real::sqrt((a + one / a)
                                  * (one / bandwidth - one)
                                  + two));
    }

    Logging_trace1("<<: %1", TOSTRING(alpha));
    return alpha;
}

/*--------------------*/

/**
 * Returns string representation of band width unit <C>value</C>.
 *
 * @param[in] value  band width unit
 * @return  string representation
 */
static String _bwUnitToString (IN FilterBandwidthUnit value)
{
    String result;

    switch (value) {
        case FilterBandwidthUnit::frequency:
            result = _bwUnitText_frequency;
            break;
        case FilterBandwidthUnit::octaves:
            result = _bwUnitText_octave;
            break;
        case FilterBandwidthUnit::quality:
            result = _bwUnitText_quality;
            break;
        case FilterBandwidthUnit::slope:
            result = _bwUnitText_slope;
            break;
        case FilterBandwidthUnit::butterworth:
            result = _bwUnitText_butterworth;
            break;
    }

    return result;
}

/*--------------------*/

/**
 * Sets up bandwidth unit parameter in <C>parameterMap</C> for
 * filter kind given as <C>filterKind</C>.
 *
 * @param[inout] parameterMap  effect parameter map of filter effect
 *                             to be updated for bandwidth unit parameter
 * @param[in] filterKind       kind of filter
 */
static void
_setBandwidthUnitParameter(INOUT SoXEffectParameterMap& parameterMap,
                           IN String& filterKind)
{
    Logging_trace1(">>: %1", filterKind);

    const StringList unitCodeList = _unitCodeListForKind(filterKind);

    if (unitCodeList.size() > 0) {
        StringList bwUnitTextList;

        for (Natural i = 0;  i < unitCodeList.size();  i++) {
            const String bandwidthUnitText =
                _unitCodeToTextMap.at(unitCodeList[i]);
            bwUnitTextList.append(bandwidthUnitText);
        }

        Logging_trace1("--: units = %1", bwUnitTextList.toString());
        parameterMap.setKindAndValueEnum(parameterName_bandwidthUnit,
                                         bwUnitTextList,
                                         _bwUnitText_quality);
    }

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Converts string <C>value</C> to bandwidth unit.
 *
 * @param[in] value  string representation of bandwidth unit
 * @return  associated bandwidth unit
 */
static FilterBandwidthUnit _toBWUnit (IN String& value)
{
    FilterBandwidthUnit result;

    if (value == _bwUnitText_frequency) {
        result = FilterBandwidthUnit::frequency;
    } else if (value == _bwUnitText_octave) {
        result = FilterBandwidthUnit::octaves;
    } else if (value == _bwUnitText_quality) {
        result = FilterBandwidthUnit::quality;
    } else if (value == _bwUnitText_slope) {
        result = FilterBandwidthUnit::slope;
    } else {
        result = FilterBandwidthUnit::butterworth;
    }

    return result;
}


/*--------------------*/

/**
 * Returns list of bandwidth unit codes for given
 * <C>filterKind</C>.
 *
 * @param[in] filterKind  kind of filter to be used
 * @return  list of bandwidth unit codes for filter
 */
static StringList _unitCodeListForKind (IN String& filterKind)
{
    Logging_trace1(">>: %1", filterKind);
    StringList result;
    const String unitCodeListAsString =
        _filterKindToUnitMap.atWithDefault(filterKind, "");

    if (STR::contains(unitCodeListAsString, separator)) {
        result = StringList::makeBySplit(unitCodeListAsString, separator);
    }

    Logging_trace1("<<: %1", result.toString());
    return result;
}

/*--------------------*/

/**
 * Updates effect parameters in <C>parameterMap</C> for filter
 * kind given as <C>filterKind</C>.
 *
 * @param[inout] parameterMap  effect parameter map of filter
 *                             effect to be initialized
 * @param[in] filterKind       kind of filter to be set up
 */
static void
_updateParametersForKind (INOUT SoXEffectParameterMap& parameterMap,
                          IN String& filterKind)
{
    Logging_trace1(">>: %1", filterKind);
    Assertion_pre(_kindList.contains(filterKind),
                  "filter kind must be known");

    _setBandwidthUnitParameter(parameterMap, filterKind);

    /* activate or deactivate parameter names */
    Boolean isActive;
    const String widgetDataString =
        _filterKindToWidgetDataMap.at(filterKind);
    const StringList widgetCodeList =
        StringList::makeBySplit(widgetDataString, separator);

    isActive = widgetCodeList.contains(paramFlag_biquad);

    for (const String& parameterName : _biquadFilterParameterNameList) {
        parameterMap.setActiveness(parameterName, isActive);
    }

    isActive = widgetCodeList.contains(paramFlag_poleCount);
    parameterMap.setActiveness(parameterName_poleCount, isActive);

    isActive = widgetCodeList.contains(paramFlag_dBGain);
    parameterMap.setActiveness(parameterName_dBGain, isActive);

    isActive = widgetCodeList.contains(paramFlag_unpitchedMode);
    parameterMap.setActiveness(parameterName_unpitchedMode, isActive);

    isActive = widgetCodeList.contains(paramFlag_cstSkirtGain);
    parameterMap.setActiveness(parameterName_cstSkirtGain, isActive);

    isActive = widgetCodeList.contains(paramFlag_frequency);
    parameterMap.setActiveness(parameterName_frequency, isActive);

    isActive = widgetCodeList.contains(paramFlag_bandwidth);
    parameterMap.setActiveness(parameterName_bandwidth, isActive);
    parameterMap.setActiveness(parameterName_bandwidthUnit, isActive);

    isActive = widgetCodeList.contains(paramFlag_equGain);
    parameterMap.setActiveness(parameterName_equGain, isActive);

    Logging_trace1("<<: parameterMap = %1", parameterMap.toString());
}

/*============================================================*/

/*---------------------*/
/* setup & destruction */
/*---------------------*/

SoXFilter_AudioEffect::SoXFilter_AudioEffect ()
{
    Logging_trace(">>");

    /* initialize descriptor */
    _effectDescriptor = new _EffectDescriptor_FLTR(*this);

    /* calculate list of bandwidth units */
    const String filterKind = _kindList[0];
    const StringList unitCodeList = _unitCodeListForKind(filterKind);
    StringList bwUnitTextList;

    if (unitCodeList.size() == 0) {
        bwUnitTextList.append(_bwUnitText_quality);
    } else {
        for (Natural i = 0; i < unitCodeList.size(); i++) {
            const String bandwidthUnitText =
                _unitCodeToTextMap.at(unitCodeList[i]);
            bwUnitTextList.append(bandwidthUnitText);
        }
    }

    /* initialize parameters */
    _effectParameterMap.clear();

    /* first of all define kind of filter and all parameter names */
    _effectParameterMap.setKindAndValueEnum(parameterName_kind,
                                            _kindList, filterKind);
    _effectParameterMap.setKindAndValueReal(parameterName_frequency,
                                            10.0, 20000.0, 0.01, 1000.0);
    _effectParameterMap.setKindAndValueReal(parameterName_bandwidth,
                                            0.001, 20000.0, 0.001, 1.0);
    _effectParameterMap.setKindAndValueEnum(parameterName_bandwidthUnit,
                                            bwUnitTextList,
                                            _bwUnitText_quality);
    _effectParameterMap.setKindAndValueReal(parameterName_dBGain,
                                            -25.0, 25.0, 0.01, 0.0);
    _effectParameterMap.setKindAndValueEnum(parameterName_cstSkirtGain,
                                            _yesNoList, "No");
    _effectParameterMap.setKindAndValueReal(parameterName_equGain,
                                            -25.0, 25.0, 0.01, 0.0);
    _effectParameterMap.setKindAndValueReal(parameterName_poleCount,
                                            1.0, 2.0, 1.0, 1.0);
    _effectParameterMap.setKindAndValueEnum(parameterName_unpitchedMode,
                                            _yesNoList, "No");

    for (const String& parameterName : _biquadFilterParameterNameList) {
        _effectParameterMap.setKindAndValueReal(parameterName,
                                                -10.0, 10.0, 1e-6, 0.0);
    }

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXFilter_AudioEffect::~SoXFilter_AudioEffect ()
{
    Logging_trace(">>");
    delete (_EffectDescriptor_FLTR*) _effectDescriptor;
    Logging_trace("<<");
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXFilter_AudioEffect::toString () const
{
    String st = "SoXFilter_AudioEffect(";
    st += _asRawString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXFilter_AudioEffect::_effectDescriptorToString () const
{
    const _EffectDescriptor_FLTR& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FLTR>(_effectDescriptor);
    return effectDescriptor.toString();
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXFilter_AudioEffect::name () const
{
    return "SoX Filter";
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXParameterValueChangeKind
SoXFilter_AudioEffect::_setValueInternal
                           (IN String& parameterName,
                            IN String& value,
                            IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalculationIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    _EffectDescriptor_FLTR& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FLTR>(_effectDescriptor);
    SoXParameterValueChangeKind result =
        SoXParameterValueChangeKind::parameterChange;

    if (parameterName == parameterName_kind) {
        _updateParametersForKind(_effectParameterMap, value);
        effectDescriptor.kind = _effectParameterMap.value(parameterName);
        result = SoXParameterValueChangeKind::globalChange;
    } else {
        Boolean effectIsUpdated =
            (recalculationIsForced
             && _effectParameterMap.isActive(parameterName));

        if (parameterName == "a0") {
            effectDescriptor.a0 = STR::toReal(value);
        } else if (parameterName == "a1") {
            effectDescriptor.a1 = STR::toReal(value);
        } else if (parameterName == "a2") {
            effectDescriptor.a2 = STR::toReal(value);
        } else if (parameterName == "b0") {
            effectDescriptor.b0 = STR::toReal(value);
        } else if (parameterName == "b1") {
            effectDescriptor.b1 = STR::toReal(value);
        } else if (parameterName == "b2") {
            effectDescriptor.b2 = STR::toReal(value);
        } else if (parameterName == parameterName_bandwidth) {
            effectDescriptor.bandwidth = STR::toReal(value);
        } else if (parameterName == parameterName_bandwidthUnit) {
            effectDescriptor.bandwidthUnit = _toBWUnit(value);
        } else if (parameterName == parameterName_cstSkirtGain) {
            effectDescriptor.usesConstantSkirtGain = (value == "Yes");
        } else if (parameterName == parameterName_dBGain) {
            effectDescriptor.dBGain = STR::toReal(value);
        } else if (parameterName == parameterName_equGain) {
            effectDescriptor.equGain = STR::toReal(value);
        } else if (parameterName == parameterName_frequency) {
            effectDescriptor.frequency = STR::toReal(value);
        } else if (parameterName == parameterName_poleCount) {
            effectDescriptor.isSinglePole = (STR::toReal(value) == 1.0);
        } else if (parameterName == parameterName_unpitchedMode) {
            effectDescriptor.usesUnpitchedAudioMode = (value == "Yes");
        }

        if (effectIsUpdated) {
            effectDescriptor.updateSettings(_sampleRate);
        }
    }

    Logging_trace1("<<: %1", SoXParameterValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXFilter_AudioEffect::setDefaultValues () {
    Logging_trace(">>");
    const String filterKind =
        _effectParameterMap.value(parameterName_kind);
    _updateParametersForKind(_effectParameterMap, filterKind);
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXFilter_AudioEffect::prepareToPlay (IN Real sampleRate)
{
    Logging_trace1(">>: %1", TOSTRING(sampleRate));

    if (sampleRate != _sampleRate) {
        _EffectDescriptor_FLTR& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_FLTR>(_effectDescriptor);
        _sampleRate = sampleRate;

        /* filter has to be recalculated */
        effectDescriptor.updateSettings(_sampleRate);
    }

    Logging_trace("<<");
}

/*--------------------*/

void
SoXFilter_AudioEffect::processBlock (IN Real timePosition,
                                     INOUT AudioSampleListVector& buffer)
{
    Logging_trace1(">>: time = %1", TOSTRING(timePosition));

    if (_parametersAreValid) {
        SoXAudioEffect::processBlock(timePosition, buffer);

        _EffectDescriptor_FLTR& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_FLTR>(_effectDescriptor);
        const Natural sampleCount = buffer[0].size();
        IIRFilter& filter{effectDescriptor.filter};
        AudioSampleRingBufferVector& sampleRingBufferVector =
            effectDescriptor.sampleRingBufferVector;

        for (Natural channel = 0;  channel < _channelCount;
             channel++) {
            AudioSampleRingBuffer& inputSampleRingBuffer =
                sampleRingBufferVector.at(channel, 0);
            AudioSampleRingBuffer& outputSampleRingBuffer =
                sampleRingBufferVector.at(channel, 1);
            const AudioSampleList& inputList = buffer[channel];
            AudioSampleList& outputList = buffer[channel];

            for (Natural i = 0;  i < sampleCount;  i++) {
                const AudioSample inputSample = inputList[i];
                AudioSample& outputSample     = outputList[i];
                inputSampleRingBuffer.shiftRight(inputSample);
                outputSampleRingBuffer.shiftRight(0.0);
                filter.apply(inputSampleRingBuffer, outputSampleRingBuffer);
                outputSample = outputSampleRingBuffer.first();
            }
        }
    }

    Logging_trace("<<");
}
