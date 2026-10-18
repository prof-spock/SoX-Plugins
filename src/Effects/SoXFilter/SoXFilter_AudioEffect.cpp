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

#include "AudioDataPointRingBufferVector.h"
#include "Dictionary.h"
#include "FilterBandwidthUnit.h"
#include "IIRFilter.h"
#include "Logging.h"
#include "SoXAudioHelper.h"

/*--------------------*/

using Audio::AudioDataPointRingBufferVector;
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
/* EFFECT KINDS       */
/*....................*/

/** the name of the allpass filter */
static const String _EffectKind_allpass    = "Allpass";

/** the name of the band filter */
static const String _EffectKind_band       = "Band";

/** the name of the bass filter */
static const String _EffectKind_bass       = "Bass";

/** the name of the bandpass filter */
static const String _EffectKind_bandpass   = "BandPass";

/** the name of the bandreject filter */
static const String _EffectKind_bandreject = "BandReject";

/** the name of the biquad filter */
static const String _EffectKind_biquad     = "Biquad";

/** the name of the equalizer filter */
static const String _EffectKind_equalizer  = "Equalizer";

/** the name of the highpass filter */
static const String _EffectKind_highpass   = "HighPass";

/** the name of the lowpass filter */
static const String _EffectKind_lowpass    = "LowPass";

/** the name of the treble filter */
static const String _EffectKind_treble     = "Treble";

/** the list of all filter kinds as a string (in English language) */
static const StringList _effectKindList =
    StringList::fromList({_EffectKind_allpass, _EffectKind_band,
                          _EffectKind_bandpass, _EffectKind_bandreject,
                          _EffectKind_bass, _EffectKind_biquad,
                          _EffectKind_equalizer, _EffectKind_highpass,
                          _EffectKind_lowpass, _EffectKind_treble});

/*----------------------*/
/* FORWARD DECLARATIONS */
/*----------------------*/

static Real _alphaForBandwidth (IN Real dataPointRate,
                                IN Real bandwidth,
                                IN FilterBandwidthUnit unit,
                                IN Real frequency,
                                IN Real gain);

static String _bwUnitToString (IN FilterBandwidthUnit value);

static StringList _unitCodeListForKind (IN String& effectKind);

/*============================================================*/

namespace SoXPlugins::Effects::SoXFilter {

    /**
     * An <C>_EffectDescriptor_FLTR</C> object is the internal
     * implementation of a biquad filter effect descriptor type where
     * all data point input is routed to and data point output is
     * routed from.
     */
    struct _EffectDescriptor_FLTR {

        /** the associated filter effect */
        SoXFilter_AudioEffect& effect;

        /** the effect kind (as a string) */
        String kind;

        Real b0; /**< IIR filter coefficient b0 */
        Real b1; /**< IIR filter coefficient b1 */
        Real b2; /**< IIR filter coefficient b2 */
        Real a0; /**< IIR filter coefficient a0 */
        Real a1; /**< IIR filter coefficient a1 */
        Real a2; /**< IIR filter coefficient a2 */

        /** a vector of data point ring buffers for the IIR filter */
        AudioDataPointRingBufferVector dataPointRingBufferVector;

        /** the underlying IIR filter */
        IIRFilter filter;

        /** the characteristic frequency of the filter */
        Real frequency;

        /** the nominal value of the bandwidth of the filter */
        Real bandwidth;

        /** the unit of the bandwidth of the filter */
        FilterBandwidthUnit bandwidthUnit;

        /** the filter gain */
        Real gain;

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
         * and <C>dataPointRate</C> and updates IIR filter
         * accordingly.
         *
         * @param[in] dataPointRate  new data point rate
         */
        void updateSettings (IN Real dataPointRate);

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

/*--------------------*/
/* support functions  */
/*--------------------*/

/**
 * Updates coefficients for an allpass filter in <C>descriptor</C>
 * using parameters <C>alpha</C> and <C>cw0</C>.
 *
 * @param [inout]  descriptor  filter descriptor to be updated
 * @param [in]     alpha       alpha parameter
 * @param [in]     cw0         cosine of wave length
 */
static void
_updateSettingsForAllpassFilter (INOUT _EffectDescriptor_FLTR& descriptor,
                                 IN Real alpha,
                                 IN Real cw0)
{
    Logging_trace3(">>: descriptor = %1, alpha = %2, cw0 = %3",
                   descriptor.toString(), TOSTRING(alpha), TOSTRING(cw0));

    const Real b0 =  Real::one - alpha;
    const Real b1 = -Real::two * cw0;
    const Real b2 =  Real::one + alpha;

    descriptor.b0 =  b0;
    descriptor.b1 =  b1;
    descriptor.b2 =  b2;
    descriptor.a0 =  b2;
    descriptor.a1 =  b1;
    descriptor.a2 =  b0;

    Logging_trace1("<<: descriptor = %1", descriptor.toString());
}

/*--------------------*/

/**
 * Updates coefficients for a band filter in <C>descriptor</C> using
 * parameters <C>dataPointRate</C> and <C>cw0</C>.
 *
 * @param [inout]  descriptor     filter descriptor to be updated
 * @param [in]     dataPointRate  data point rate
 * @param [in]     cw0            cosine of wave length
 */
static void
_updateSettingsForBandFilter (INOUT _EffectDescriptor_FLTR& descriptor,
                              IN Real cw0,
                              IN Real dataPointRate)
{
    Logging_trace3(">>: descriptor = %1, cw0 = %2, dataPointRate = %3Hz",
                   descriptor.toString(), TOSTRING(cw0),
                   TOSTRING(dataPointRate));

    const Real four{4.0};
    const Real frequency = descriptor.frequency;
    const Real bandwidth = descriptor.bandwidth;
    const FilterBandwidthUnit bandwidthUnit = descriptor.bandwidthUnit;

    const Real bandwidthAsFrequency =
             (bandwidthUnit == FilterBandwidthUnit::quality
              ? frequency / bandwidth
              : (bandwidthUnit == FilterBandwidthUnit::octaves
                 ? Real{(frequency * Real::two.power(bandwidth - Real::one)
                         * Real::two.power(-bandwidth / Real::two))}
                 : bandwidth));

    const Real a0 = Real::one;
    const Real a2 = (-Real::twoPi * bandwidthAsFrequency / dataPointRate).exp();
    const Real a1 = -four * a2 / (Real::one + a2) * cw0;
    Real b0 = (Real::sqrt(Real::one - a1.sqr() / (four * a2))
               * (Real::one - a2));
    const Real b1 = Real::zero;
    const Real b2 = Real::zero;

    if (descriptor.usesUnpitchedAudioMode) {
        const Real factor =
            Real::sqrt(((Real::one + a2).sqr() - a1.sqr())
                       * (Real::one - a2) / (Real::one + a2))
            / b0;
        b0 *= factor;
    }

    descriptor.b0 =  b0;
    descriptor.b1 =  b1;
    descriptor.b2 =  b2;
    descriptor.a0 =  a0;
    descriptor.a1 =  a1;
    descriptor.a2 =  a2;

    Logging_trace1("<<: descriptor = %1", descriptor.toString());
}

/*--------------------*/

/**
 * Updates coefficients for a band pass or band reject filter in
 * <C>descriptor</C> using parameters <C>alpha</C>, <C>cw0</C> and
 * <C>sw0</C>.
 *
 * @param [inout]  descriptor     filter descriptor to be updated
 * @param [in]     alpha          alpha parameter
 * @param [in]     cw0            cosine of wave length
 * @param [in]     sw0            sine of wave length
 */
static void
_updateSettingsForBpBrFilter (INOUT _EffectDescriptor_FLTR& descriptor,
                              IN Real alpha,
                              IN Real cw0,
                              IN Real sw0)
{
    Logging_trace3(">>: descriptor = %1, cw0 = %2, dataPointRate = %3Hz",
                   descriptor.toString(), TOSTRING(cw0),
                   TOSTRING(dataPointRate));

    Real b0, b1, b2;

    if (descriptor.kind == _EffectKind_bandreject) {
        b0 =  Real::one;
        b1 = -cw0 * Real::two;
        b2 =  Real::one;
    } else {
        b0 =  (descriptor.usesConstantSkirtGain ? sw0 / Real::two : alpha);
        b1 =  Real::zero;
        b2 = -b0;
    }

    descriptor.b0 =  b0;
    descriptor.b1 =  b1;
    descriptor.b2 =  b2;
    descriptor.a0 =  alpha + Real::one;
    descriptor.a1 = -cw0 * Real::two;
    descriptor.a2 = -alpha + Real::one;

    Logging_trace1("<<: descriptor = %1", descriptor.toString());
}

/*--------------------*/

/**
 * Updates coefficients for a bass or treble filter in
 * <C>descriptor</C> using parameters <C>alpha</C> and <C>cw0</C>.
 *
 * @param [inout]  descriptor  filter descriptor to be updated
 * @param [in]     alpha       alpha parameter
 * @param [in]     cw0         cosine of wave length
 */
static void
_updateSettingsForBsTrbFilter (INOUT _EffectDescriptor_FLTR& descriptor,
                               IN Real alpha,
                               IN Real cw0)
{
    Logging_trace3(">>: descriptor = %1, alpha = %2, cw0 = %3",
                   descriptor.toString(), TOSTRING(alpha), TOSTRING(cw0));

    const Real a = SoXAudioHelper::dBToLinear(descriptor.gain, 40.0);
    const Real aP1 = a + Real::one;
    const Real aM1 = a - Real::one;
    const Real f = (descriptor.kind == _EffectKind_bass
                    ? Real::one : -Real::one);
    const Real sqrtAlphaA = Real::two * Real::sqrt(a) * alpha;
    const Real twoF = Real::two * f;

    descriptor.b0 =         a * ( aP1 - f * aM1 * cw0 + sqrtAlphaA );
    descriptor.b1 =  twoF * a * ( aM1 - f * aP1 * cw0              );
    descriptor.b2 =         a * ( aP1 - f * aM1 * cw0 - sqrtAlphaA );
    descriptor.a0 =               aP1 + f * aM1 * cw0 + sqrtAlphaA;
    descriptor.a1 = -twoF *     ( aM1 + f * aP1 * cw0              );
    descriptor.a2 =               aP1 + f * aM1 * cw0 - sqrtAlphaA;

    Logging_trace1("<<: descriptor = %1", descriptor.toString());
}

/*--------------------*/

/**
 * Updates coefficients for an equalizer filter in
 * <C>descriptor</C> using parameters <C>alpha</C> and <C>cw0</C>.
 *
 * @param [inout]  descriptor  filter descriptor to be updated
 * @param [in]     alpha       alpha parameter
 * @param [in]     cw0         cosine of wave length
 */
static void
_updateSettingsForEqualizerFilter (INOUT _EffectDescriptor_FLTR& descriptor,
                                   IN Real alpha,
                                   IN Real cw0)
{
    Logging_trace3(">>: descriptor = %1, alpha = %2, cw0 = %3",
                   descriptor.toString(), TOSTRING(alpha), TOSTRING(cw0));

    const Real ten{10.0};
    const Real filterGain = ten.power(descriptor.gain / 40.0);
    const Real b1 = -Real::two * cw0;

    descriptor.b0 = Real::one + alpha * filterGain;
    descriptor.b1 = b1;
    descriptor.b2 = Real::one - alpha * filterGain;
    descriptor.a0 = Real::one + alpha / filterGain;
    descriptor.a1 = b1;
    descriptor.a2 = Real::one - alpha / filterGain;

    Logging_trace1("<<: descriptor = %1", descriptor.toString());
}

/*--------------------*/

/**
 * Updates coefficients for an equalizer filter in
 * <C>descriptor</C> using parameters <C>alpha</C> and <C>cw0</C>.
 *
 * @param [inout]  descriptor  filter descriptor to be updated
 * @param [in]     alpha       alpha parameter
 * @param [in]     cw0         cosine of wave length
 */
static void
_updateSettingsForHpLpFilter (INOUT _EffectDescriptor_FLTR& descriptor,
                              IN Real alpha,
                              IN Real cw0,
                              IN Real w0)
{
    Logging_trace4(">>: descriptor = %1, alpha = %2, cw0 = %3, w0 = %4",
                   descriptor.toString(), TOSTRING(alpha),
                   TOSTRING(cw0), TOSTRING(w0));

    Real factorA, factorB, factorC;

    if (descriptor.isSinglePole) {
        if (descriptor.kind == _EffectKind_highpass) {
            factorA = -Real::one;
            factorB = 0.5;
            factorC = -Real::one;
        } else {
            factorA = Real::one;
            factorB = Real::one;
            factorC = Real::zero;
        }

        const Real a1 = -Real::exp(-w0);
        const Real b0 = (Real::one + factorA * a1) * factorB;

        descriptor.a0 = Real::one;
        descriptor.a1 = a1;
        descriptor.a2 = Real::zero;
        descriptor.b0 = b0;
        descriptor.b1 = factorC * b0;
        descriptor.b2 = Real::zero;
    } else {
        if (descriptor.kind == _EffectKind_highpass) {
            factorA = (Real::one + cw0);
            factorB = -Real::one;
        } else {
            factorA = (Real::one - cw0);
            factorB = Real::one;
        }

        const Real b0 = factorA / Real::two;

        descriptor.b0 = b0;
        descriptor.b1 = factorB * factorA;
        descriptor.b2 = b0;
        descriptor.a0 = Real::one + alpha;
        descriptor.a1 = -Real::two * cw0;
        descriptor.a2 = Real::one - alpha;
    }

    Logging_trace1("<<: descriptor = %1", descriptor.toString());
}

/*------------------------*/
/* _EffectDescriptor_FLTR */
/*------------------------*/

_EffectDescriptor_FLTR::
_EffectDescriptor_FLTR (INOUT SoXFilter_AudioEffect& effect)
    : effect(effect),
      kind{_EffectKind_biquad},
      b0{0.0}, b1{0.0}, b2{0.0}, a0{0.0}, a1{0.0}, a2{0.0},
      dataPointRingBufferVector{2, true, _biquadFilterOrder},
      filter{_biquadFilterOrder},
      frequency{1000.0},
      bandwidth{1.5},
      bandwidthUnit{FilterBandwidthUnit::slope},
      gain{0.0},
      usesUnpitchedAudioMode{false},
      usesConstantSkirtGain{false},
      isSinglePole{true}
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

void _EffectDescriptor_FLTR::updateSettings (IN Real dataPointRate)
{
    Logging_trace2(">>: kind = %1, dataPointRate = %2",
                   kind, TOSTRING(dataPointRate));

    effect.setParameterValidity(false);

    if (kind == _EffectKind_biquad) {
        /* we can just keep the direct coefficients, no
           calculation is necessary */
    } else {
        const Real w0 = Real::twoPi * frequency / dataPointRate;
        const Real cw0 = Real::cos(w0);
        const Real sw0 = Real::sin(w0);
        const Real alpha = _alphaForBandwidth(dataPointRate,
                                              bandwidth,
                                              bandwidthUnit,
                                              frequency,
                                              gain);

        if (kind == _EffectKind_allpass) {
            _updateSettingsForAllpassFilter(*this, alpha, cw0);
        } else if (kind == _EffectKind_band) {
            _updateSettingsForBandFilter(*this, cw0, dataPointRate);
        } else if (kind == _EffectKind_bandpass
                   || kind == _EffectKind_bandreject) {
            _updateSettingsForBpBrFilter(*this, alpha, cw0, sw0);
        } else if (kind == _EffectKind_bass || kind == _EffectKind_treble) {
            _updateSettingsForBsTrbFilter(*this, alpha, cw0);
        } else if (kind == _EffectKind_equalizer) {
            _updateSettingsForEqualizerFilter(*this, alpha, cw0);
        } else if (kind == _EffectKind_highpass
                   || kind == _EffectKind_lowpass) {
            _updateSettingsForHpLpFilter(*this, alpha, cw0, w0);
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
                    " gain = %5dB,"
                    " usesUnpitchedAudioMode = %6,"
                    " usesConstantSkirtGain = %7,"
                    " isSinglePole = %8",
                    kind, TOSTRING(frequency),
                    TOSTRING(bandwidth), _bwUnitToString(bandwidthUnit),
                    TOSTRING(gain),
                    TOSTRING(usesUnpitchedAudioMode),
                    TOSTRING(usesConstantSkirtGain),
                    TOSTRING(isSinglePole));

    String st2 =
        STR::expand("b0 = %1, b1 = %2, b2 = %3,"
                    " a0 = %4, a1 = %5, a2 = %6,"
                    " filter = %7, dataPointRingBufferVector = %8",
                    TOSTRING(b0), TOSTRING(b1), TOSTRING(b2),
                    TOSTRING(a0), TOSTRING(a1), TOSTRING(a2),
                    filter.toString(),
                    dataPointRingBufferVector.toString());

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
static const String parameterName_bandwidth      = "Bandwidth";

/** the parameter name of the bandwidthUnit parameter */
static const String parameterName_bandwidthUnit  = "Bandwidth Unit";

/** the parameter names of a biquad filter */
static const StringList _biquadFilterParameterNameList =
    StringList::fromList({"b0", "b1", "b2", "a0", "a1", "a2"});

/** the parameter name of the cstSkirtGain parameter */
static const String parameterName_cstSkirtGain   = "Cst. Skirt Gain?";

/** the parameter name of the effect kind parameter */
static const String parameterName_effectKind     = "Effect Kind";

/** the alias parameter name of the effect kind parameter */
static const String parameterName__EffectKind_OLD = "Filter Kind";

/** the alias parameter name of the eqGain parameter */
static const String parameterName_eqGain_OLD     = "Eq. Gain [dB]";

/** the parameter name of the frequency parameter */
static const String parameterName_frequency      = "Frequency [Hz]";

/** the parameter name of the gain parameter */
static const String parameterName_gain           = "Gain [dB]";

/** the parameter name of the poleCount parameter */
static const String parameterName_poleCount      = "Number of Poles";

/** the parameter name of the unpitchedMode parameter */
static const String parameterName_unpitchedMode  = "Unpitched Mode?";

/*--------------------*/

/** the list of all parameter names */
static StringList _allParameterNameList =
    StringList::fromList(
        {parameterName_bandwidth,
         parameterName_bandwidthUnit,
         parameterName_cstSkirtGain,
         parameterName_frequency,
         parameterName_gain,
         parameterName_effectKind,
         parameterName_poleCount,
         parameterName_unpitchedMode,
         "b0", "b1", "b2", "a0", "a1", "a2"});

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

/** the character flag for the gain parameter set in a dialog */
static const String paramFlag_gain        = "D";

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
static const Dictionary _effectKindToWidgetDataMap =
    Dictionary::fromList(
        StringList::makeBySplit(
            _EffectKind_allpass    + comma + "F/B"   + comma +
            _EffectKind_band       + comma + "U/F/B" + comma +
            _EffectKind_bandpass   + comma + "C/F/B" + comma +
            _EffectKind_bandreject + comma + "F/B"   + comma +
            _EffectKind_bass       + comma + "D/F/B" + comma +
            _EffectKind_biquad     + comma + "Q"     + comma +
            _EffectKind_equalizer  + comma + "F/B/D" + comma +
            _EffectKind_highpass   + comma + "P/F/B" + comma +
            _EffectKind_lowpass    + comma + "P/F/B" + comma +
            _EffectKind_treble     + comma + "D/F/B",
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
static const Dictionary _effectKindToUnitMap =
    Dictionary::fromList(
        StringList::makeBySplit(
            _EffectKind_allpass    + comma + "f/o/q/b"   + comma +
            _EffectKind_band       + comma + "f/o/q/b"   + comma +
            _EffectKind_bandpass   + comma + "f/o/q/b"   + comma +
            _EffectKind_bandreject + comma + "f/o/q/b"   + comma +
            _EffectKind_bass       + comma + "f/o/q/b/s" + comma +
            _EffectKind_equalizer  + comma + "f/o/q/b"   + comma +
            _EffectKind_highpass   + comma + "f/o/q/b"   + comma +
            _EffectKind_lowpass    + comma + "f/o/q/b"   + comma +
            _EffectKind_treble     + comma + "f/o/q/b/s",
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
 * <C>unit</C> with <C>frequency</C> and <C>gain</C>;
 * <C>dataPointRate</C> gives the current data point rate for the
 * effect.
 *
 * @param[in] dataPointRate  current effect data point rate
 * @param[in] bandWidth      the nominal of the filter band width
 * @param[in] unit           the unit of the filter band width
 * @param[in] frequency      the characteristic frequency of the filter
 * @param[in] gain           the filter gain (in decibels)
 * @return  alpha parameter of filter
 */
static Real _alphaForBandwidth (IN Real dataPointRate,
                                IN Real bandwidth,
                                IN FilterBandwidthUnit unit,
                                IN Real frequency,
                                IN Real gain)
{
    Logging_trace5(">>: dataPointRate = %1, bandwidth = %2%3,"
                   " frequency = %4, gain = %5",
                   TOSTRING(dataPointRate), TOSTRING(bandwidth),
                   _bwUnitToString(unit), TOSTRING(frequency),
                   TOSTRING(gain));

    const Real w0 = Real::twoPi * frequency / dataPointRate;
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
            const Real a = SoXAudioHelper::dBToLinear(gain, 40.0);
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
 * filter kind given as <C>effectKind</C>.
 *
 * @param[inout] parameterMap  effect parameter map of filter effect
 *                             to be updated for bandwidth unit parameter
 * @param[in] effectKind       kind of filter
 */
static void
_setBandwidthUnitParameter(INOUT SoXEffectParameterMap& parameterMap,
                           IN String& effectKind)
{
    Logging_trace1(">>: %1", effectKind);

    const StringList unitCodeList = _unitCodeListForKind(effectKind);

    if (!unitCodeList.empty()) {
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
 * <C>effectKind</C>.
 *
 * @param[in] effectKind  kind of filter to be used
 * @return  list of bandwidth unit codes for filter
 */
static StringList _unitCodeListForKind (IN String& effectKind)
{
    Logging_trace1(">>: %1", effectKind);
    StringList result;
    /* WORKAROUND: use longest unit code list for the value list,
     * because the values cannot be changed later */
    const String unitCodeListAsString =
        // _effectKindToUnitMap.atWithDefault(effectKind, "");
        "f/o/q/b/s";    

    if (STR::contains(unitCodeListAsString, separator)) {
        result = StringList::makeBySplit(unitCodeListAsString, separator);
    }

    Logging_trace1("<<: %1", result.toString());
    return result;
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
    const String effectKind = _effectKindList[0];
    const StringList unitCodeList = _unitCodeListForKind(effectKind);
    StringList bwUnitTextList;

    if (unitCodeList.empty()) {
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

    /* define kind of filter and all parameter names in the
       presentation order */
    _effectParameterMap.setKindAndValueEnum(parameterName_effectKind,
                                            _effectKindList,
                                            _EffectKind_allpass);

    _effectParameterMap.setKindEnum(parameterName_cstSkirtGain,
                                    _yesNoList);
    _effectParameterMap.setKindEnum(parameterName_unpitchedMode,
                                    _yesNoList);

    _effectParameterMap.setKindReal(parameterName_gain,
                                    -25.0, 25.0, 0.01);
    _effectParameterMap.setKindReal(parameterName_poleCount,
                                    1.0, 2.0, 1.0);
    _effectParameterMap.setKindReal(parameterName_frequency,
                                    10.0, 20000.0, 0.01);
    _effectParameterMap.setKindReal(parameterName_bandwidth,
                                    0.001, 20000.0, 0.001);
    _effectParameterMap.setKindEnum(parameterName_bandwidthUnit,
                                    bwUnitTextList);

    for (const String& parameterName : _biquadFilterParameterNameList) {
        _effectParameterMap.setKindReal(parameterName, -10.0, 10.0, 1e-6);
    }

    /* add alias parameter name */
    _effectParameterMap.setAsAlias(parameterName__EffectKind_OLD,
                                   parameterName_effectKind);

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

String SoXFilter_AudioEffect::_effectDescriptorToString () const
{
    String result;

    if (_effectDescriptor == nullptr) {
        result = "NULL";
    } else {
        const _EffectDescriptor_FLTR& effectDescriptor =
            TOREFERENCE<_EffectDescriptor_FLTR>(_effectDescriptor);
        result = effectDescriptor.toString();
    }

    return result;
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

SoXValueChangeKind
SoXFilter_AudioEffect::_setValueInternal
                                (IN String& parameterName,
                                 IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    _EffectDescriptor_FLTR& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FLTR>(_effectDescriptor);
    SoXValueChangeKind result = SoXValueChangeKind::parameterChange;

    if (parameterName == parameterName_effectKind) {
        _updateEffectForKind(value);
        effectDescriptor.kind = _effectParameterMap.value(parameterName);
        setDefaultValues();
        result = SoXValueChangeKind::globalChange;
    } else if (!_effectParameterMap.isActive(parameterName)) {
        Logging_trace("--: parameter is inactive -> ignored");
    } else {
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
        } else if (parameterName == parameterName_gain) {
            effectDescriptor.gain = STR::toReal(value);
        } else if (parameterName == parameterName_frequency) {
            effectDescriptor.frequency = STR::toReal(value);
        } else if (parameterName == parameterName_poleCount) {
            effectDescriptor.isSinglePole = (STR::toReal(value) == 1.0);
        } else if (parameterName == parameterName_unpitchedMode) {
            effectDescriptor.usesUnpitchedAudioMode = (value == "Yes");
        }
    }

    Logging_trace1("<<: %1", SoXValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXFilter_AudioEffect::setDefaultValues () {
    Logging_trace(">>");

    const String effectKind =
        _effectParameterMap.value(parameterName_effectKind);
    Dictionary dictionary;

    dictionary.set(parameterName_effectKind, effectKind);

    dictionary.set(parameterName_bandwidth,     "1.0");
    dictionary.set(parameterName_bandwidthUnit, _bwUnitText_quality);
    dictionary.set(parameterName_cstSkirtGain,  "No");
    dictionary.set(parameterName_gain,          "0.0");
    dictionary.set(parameterName_frequency,     "1000.0");
    dictionary.set(parameterName_poleCount,     "1.0");
    dictionary.set(parameterName_unpitchedMode, "No");

    if (effectKind == _EffectKind_bass
        || effectKind == _EffectKind_treble) {
        dictionary.set(parameterName_frequency,
                       (effectKind == _EffectKind_bass ? "100" : "3000"));
        dictionary.set(parameterName_bandwidth, "0.5");
        dictionary.set(parameterName_bandwidthUnit, _bwUnitText_slope);
    } else if (effectKind == _EffectKind_biquad) {
        for (const String& parameterName : _biquadFilterParameterNameList) {
            dictionary.set(parameterName, "0.0");
        }

        dictionary.set("a0", "1.0");
    } else if (effectKind == _EffectKind_highpass
               || effectKind == _EffectKind_lowpass) {
        dictionary.set(parameterName_bandwidth, "0.707107");
        dictionary.set(parameterName_bandwidthUnit, _bwUnitText_quality);
    }

    _updateEffectForKind(effectKind);
    setValues(dictionary);
    recalculate();

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXFilter_AudioEffect::
_applyForFrame (IN AudioDataPointList& inputDataPointList,
                OUT AudioDataPointList& outputDataPointList)
{
    Logging_trace(">>");

    _EffectDescriptor_FLTR& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FLTR>(_effectDescriptor);
    const IIRFilter& filter{effectDescriptor.filter};
    AudioDataPointRingBufferVector& dataPointRingBufferVector =
        effectDescriptor.dataPointRingBufferVector;

    for (Natural channel = 0;  channel < _channelCount; channel++) {
        AudioDataPointRingBuffer& inputDataPointRingBuffer =
            dataPointRingBufferVector.at(channel, 0);
        AudioDataPointRingBuffer& outputDataPointRingBuffer =
            dataPointRingBufferVector.at(channel, 1);
        const AudioDataPoint inputDataPoint = inputDataPointList[channel];
        AudioDataPoint& outputDataPoint     = outputDataPointList[channel];
        inputDataPointRingBuffer.shiftRight(inputDataPoint);
        outputDataPointRingBuffer.shiftRight(0.0);
        filter.apply(inputDataPointRingBuffer, outputDataPointRingBuffer);
        outputDataPoint = outputDataPointRingBuffer.first();
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXFilter_AudioEffect::
_handleDataPointRateChange (IN Real dataPointRate)
{
    Logging_trace1(">>: %1", TOSTRING(dataPointRate));
    recalculate();
    Logging_trace("<<");
}

/*--------------------*/

void SoXFilter_AudioEffect::recalculate ()
{
    Logging_trace(">>");

    _EffectDescriptor_FLTR& effectDescriptor =
        TOREFERENCE<_EffectDescriptor_FLTR>(_effectDescriptor);
    effectDescriptor.updateSettings(_dataPointRate);

    Logging_trace("<<");
}

/*--------------------*/

void SoXFilter_AudioEffect::_updateEffectForKind (IN String& effectKind)
{
    Logging_trace1(">>: %1", effectKind);
    Assertion_pre(_effectKindList.contains(effectKind),
                  "effect kind must be known");

    SoXEffectParameterMap& parameterMap = _effectParameterMap;

    _setBandwidthUnitParameter(parameterMap, effectKind);

    /* activate or deactivate parameter names */
    const String widgetDataString =
        _effectKindToWidgetDataMap.at(effectKind);
    const StringList widgetCodeList =
        StringList::makeBySplit(widgetDataString, separator);

    StringList activeParameterNameList;
    activeParameterNameList.append(parameterName_effectKind);

    for (String widgetCode : widgetCodeList) {
        if (widgetCode == paramFlag_biquad) {
            activeParameterNameList.append(_biquadFilterParameterNameList);
        } else if (widgetCode == paramFlag_poleCount) {
            activeParameterNameList.append(parameterName_poleCount);
        } else if (widgetCode == paramFlag_gain) {
            activeParameterNameList.append(parameterName_gain);
        } else if (widgetCode == paramFlag_unpitchedMode) {
            activeParameterNameList.append(parameterName_unpitchedMode);
        } else if (widgetCode == paramFlag_cstSkirtGain) {
            activeParameterNameList.append(parameterName_cstSkirtGain);
        } else if (widgetCode == paramFlag_frequency) {
            activeParameterNameList.append(parameterName_frequency);
        } else if (widgetCode == paramFlag_bandwidth) {
            activeParameterNameList.append(parameterName_bandwidth);
            activeParameterNameList.append(parameterName_bandwidthUnit);
        }
    }

    /* set the parameter activeness by effect kind and update
       ranges of parameters */
    parameterMap.setActivenessForNameList(_allParameterNameList, false);
    parameterMap.setActivenessForNameList(activeParameterNameList, true);
    parameterMap.invalidateValues(activeParameterNameList);
    parameterMap.setValue(parameterName_effectKind, effectKind);
    parameterMap.reorderByNameList(activeParameterNameList);

    if (effectKind == _EffectKind_equalizer) {
        /* make old eq gain an alias for gain parameter */
        parameterMap.setAsAlias(parameterName_eqGain_OLD,
                                parameterName_gain);
    }

    Logging_trace1("<<: parameterMap = %1", parameterMap.toString());
}
