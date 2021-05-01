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

/*====================*/

#include "SoXFilter_AudioEffect.h"

#include <cmath>
#include "Dictionary.h"
#include "Natural.h"
#include "StringUtil.h"
#include "SoXAudioHelper.h"
#include "SoXAudioSampleMatrix.h"
#include "SoXAudioValueChangeKind.h"
#include "SoXFilterBandwidthUnit.h"
#include "SoXIIRFilter.h"

/*====================*/

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
namespace SoXAudioHelper = SoXPlugins::CommonAudio::SoXAudioHelper;

using std::map;

using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Containers::Dictionary;
using SoXPlugins::CommonAudio::SoXAudioSampleMatrix;
using SoXPlugins::CommonAudio::SoXAudioSampleQueue;
using SoXPlugins::CommonAudio::SoXFilterBandwidthUnit;
using SoXPlugins::CommonAudio::SoXIIRFilter;
using SoXPlugins::Effects::SoXFilter::SoXFilter_AudioEffect;

/*============================================================*/

namespace SoXPlugins::Effects::SoXFilter {

    /**
     * An <C>_EffectDescriptor_FLTR</C> object is the internal
     * implementation of a biquad filter effect descriptor type where all
     * sample input is routed to and sample output is routed from.
     */
    struct _EffectDescriptor_FLTR {

        /** the filter kind (as a string) */
        String kind;

        Real b0; /**< IIR filter coefficient b0 */
        Real b1; /**< IIR filter coefficient b1 */
        Real b2; /**< IIR filter coefficient b2 */
        Real a0; /**< IIR filter coefficient a0 */
        Real a1; /**< IIR filter coefficient a1 */
        Real a2; /**< IIR filter coefficient a2 */

        /** a matrix of sample queues for the IIR filter */
        SoXAudioSampleMatrix sampleQueueList;

        /** the underlying IIR filter */
        SoXIIRFilter filter;

        /** the characteristic frequency of the filter */
        Real frequency;

        /** the nominal value of the bandwidth of the filter */
        Real bandwidth;

        /** the unit of the bandwidth of the filter */
        SoXFilterBandwidthUnit bandwidthUnit;

        /** the filter gain */
        Real dBGain;

        /** the gain for an equalizer */
        Real equGain;

        /** tells whether filter uses mode for unpitched audio (for a band
         *  filter) */
        bool usesUnpitchedAudioMode;

        /** tells whether filter uses constant skirt gain (for a bandpass) */
        bool usesConstantSkirtGain;

        /** tells whether filter is a 1-pole filter */
        bool isSinglePole;
    };

    /*--------------------*/
    /* internal features  */
    /*--------------------*/

    /** the filter order of a biquad filter */
    static const Natural _biquadFilterOrder = 3;

    /*====================*/

    /** the universal separator character */
    static const String separator = "/";

    /** the comma character */
    static const String comma = ",";

    /** the list of answers in a yes-no-combobox (in English language) */
    static const StringList _yesNoList =
        StringList::makeBySplit("Yes/No", separator);

    /** the list of all filter kinds as a string (in English language) */
    static const StringList _kindList =
        StringList::makeBySplit(
            "Allpass/Band/BandPass/BandReject/Bass/Biquad"
            "/Equalizer/HighPass/LowPass/Treble",
            separator);

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
    static const String enumValue_butterworth = "Butterworth";

    /** the name of the frequency bandwidth unit */
    static const String enumValue_frequency   = "Frequency";

    /** the name of the octave bandwidth unit */
    static const String enumValue_octave      = "Octave(s)";

    /** the name of the quality bandwidth unit */
    static const String enumValue_quality     = "Quality";

    /** the name of the slope bandwidth unit */
    static const String enumValue_slope       = "Slope";

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
        Dictionary::makeFromList(
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
        Dictionary::makeFromList(
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
        Dictionary::makeFromList(
            StringList::makeBySplit(
                "b" + comma + enumValue_butterworth + comma +
                "f" + comma + enumValue_frequency   + comma +
                "o" + comma + enumValue_octave      + comma +
                "q" + comma + enumValue_quality     + comma +
                "s" + comma + enumValue_slope,
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
                                    IN SoXFilterBandwidthUnit unit,
                                    IN Real frequency,
                                    IN Real dBGain)
    {
        const Real w0 = Real::twoPi * frequency / sampleRate;
        const Real sinW0 = sin((double) w0);
        Real alpha = 0.0;

        switch(unit) {
            case SoXFilterBandwidthUnit::quality:
                alpha = sinW0 / (Real{2} * bandwidth);
                break;

            case SoXFilterBandwidthUnit::octaves:
                alpha = sinW0 * sinh(log(2.0)/2
                                       * (double) (bandwidth * w0 / sinW0));
                break;

            case SoXFilterBandwidthUnit::butterworth:
                alpha = sinW0 / Real{2 * sqrt(0.5)};
                break;

            case SoXFilterBandwidthUnit::frequency:
                alpha = sinW0 / (Real{2} * frequency / bandwidth);
                break;

            case SoXFilterBandwidthUnit::slope:
                const Real a = SoXAudioHelper::dBToLinear(dBGain, 40);
                alpha = (sinW0 / Real{2}
                         * Real::sqrt((a + Real{1}/a)
                                      * (Real{1}/bandwidth - Real{1})
                                      + Real{2}));
        }

        return alpha;
    }

    /*--------------------*/

    /**
     * Returns string representation of band width unit <C>value</C>.
     *
     * @param[in] value  band width unit
     * @return  string representation
     */
    static String _bwUnitToString (IN SoXFilterBandwidthUnit value)
    {
        String result;

        switch (value) {
            case SoXFilterBandwidthUnit::frequency:
                result = enumValue_frequency;
                break;
            case SoXFilterBandwidthUnit::octaves:
                result = enumValue_octave;
                break;
            case SoXFilterBandwidthUnit::quality:
                result = enumValue_quality;
                break;
            case SoXFilterBandwidthUnit::slope:
                result = enumValue_slope;
                break;
            case SoXFilterBandwidthUnit::butterworth:
                result = enumValue_butterworth;
                break;
        }

        return result;
    }

    /*--------------------*/

    /**
     * Sets up a new filter effect descriptor and returns it.
     *
     * @return new filter effect descriptor
     */
    static _EffectDescriptor_FLTR* _createEffectDescriptor ()
    {
        _EffectDescriptor_FLTR* result =
            new _EffectDescriptor_FLTR{
                filterKind_biquad,              // kind
                0.0, 0.0, 0.0, 0.0, 0.0, 0.0,   // coefficients
                {2, true, _biquadFilterOrder},  // sampleQueueList
                {_biquadFilterOrder},           // filter
                1000.0,                         // frequency
                1.5,                            // bandwidth
                SoXFilterBandwidthUnit::slope,  // bandwidthUnit
                0.0,                            // dBGain
                0.0,                            // equGain
                false,                          // usesUnpitchedAudioMode
                false,                          // usesConstantSkirtGain
                true,                           // isSinglePole
            };

        return result;
    }

    /*--------------------*/

    /**
     * Sets up all audio editor parameters in <C>parameterMap</C> for
     * filter kind given as <C>filterKind</C>.
     *
     * @param[in] parameterMap  audio parameter map of filter effect to be
     *                          initialized
     * @param[in] filterKind    kind of filter to be set up
     */
    static void _initializeAllParameters (SoXAudioParameterMap& parameterMap,
                                          String filterKind)
    {
        if (!_kindList.contains(filterKind)) {
            filterKind = _kindList[0];
        }

        parameterMap.clear();

        // first of all define kind of filter
        parameterMap.setKindAndValueEnum(parameterName_kind, _kindList,
                                         filterKind);

        const String widgetDataString =
            _filterKindToWidgetDataMap.at(filterKind);
        const StringList widgetCodeList =
                       StringList::makeBySplit(widgetDataString, separator);

        if (widgetCodeList.contains(paramFlag_biquad)) {
            for (String prefix : StringList::makeBySplit("b/a", separator)) {
                for (Natural i = 0;  i < _biquadFilterOrder;  i++) {
                    const String parameterName = prefix + TOSTRING(i);
                    parameterMap.setKindAndValueReal(parameterName,
                                                     -10, 10, 1e-6, 0.0);
                }
            }
        }

        if (widgetCodeList.contains(paramFlag_poleCount)) {
            parameterMap.setKindAndValueReal(parameterName_poleCount,
                                             1, 2, 1, 1);
        }

        if (widgetCodeList.contains(paramFlag_dBGain)) {
            parameterMap.setKindAndValueReal(parameterName_dBGain,
                                             -25, 25, 0.01, 0);
        }

        if (widgetCodeList.contains(paramFlag_unpitchedMode)) {
            parameterMap.setKindAndValueEnum(parameterName_unpitchedMode,
                                             _yesNoList, "No");
        }

        if (widgetCodeList.contains(paramFlag_cstSkirtGain)) {
            parameterMap.setKindAndValueEnum(parameterName_cstSkirtGain,
                                             _yesNoList, "No");
        }

        if (widgetCodeList.contains(paramFlag_frequency)) {
            parameterMap.setKindAndValueReal(parameterName_frequency,
                                             10, 20000, 0.01, 1000);
        }

        if (widgetCodeList.contains(paramFlag_bandwidth)) {
            const String unitCodeListAsString =
                _filterKindToUnitMap.at(filterKind);
            const StringList unitCodeList =
                StringList::makeBySplit(unitCodeListAsString, separator);
            StringList enumerationValueList;

            for (Natural i = 0;  i < unitCodeList.size();  i++) {
                const String enumValue =
                    _unitCodeToTextMap.at(unitCodeList[i]);
                enumerationValueList.append(enumValue);
            }

            parameterMap.setKindAndValueReal(parameterName_bandwidth,
                                             0.001, 20000, 0.001, 1.0);
            parameterMap.setKindAndValueEnum(parameterName_bandwidthUnit,
                                             enumerationValueList,
                                             enumValue_quality);
        }

        if (widgetCodeList.contains(paramFlag_equGain)) {
            parameterMap.setKindAndValueReal(parameterName_equGain,
                                             -25, 25, 0.01, 0);
        }
    }

    /*--------------------*/

    /**
     * Sets up filter kind selection parameter in <C>parameterMap</C> to
     * first filter kind
     *
     * @param[in] parameterMap  audio parameter map of filter effect to be
     *                          initialized
     */
    static void _initializeKindParameter (SoXAudioParameterMap& parameterMap) {
        const String filterKind = _kindList[0];
        parameterMap.clear();

        // first of all define kind of filter
        parameterMap.setKindAndValueEnum(parameterName_kind, _kindList,
                                         filterKind);
    }

    /*--------------------*/

    /**
     * Converts string <C>value</C> to bandwidth unit.
     *
     * @param[in] value  string representation of bandwidth unit
     * @return  associated bandwidth unit
     */
    static SoXFilterBandwidthUnit _toBWUnit (IN String& value)
    {
        SoXFilterBandwidthUnit result;

        if (value == enumValue_frequency) {
            result = SoXFilterBandwidthUnit::frequency;
        } else if (value == enumValue_octave) {
            result = SoXFilterBandwidthUnit::octaves;
        } else if (value == enumValue_quality) {
            result = SoXFilterBandwidthUnit::quality;
        } else if (value == enumValue_slope) {
            result = SoXFilterBandwidthUnit::slope;
        } else {
            result = SoXFilterBandwidthUnit::butterworth;
        }

        return result;
    }

    /*--------------------*/

    static Real _toReal (IN String& value)
    {
        return StringUtil::toReal(value);
    }

    /*--------------------*/

    /**
     * Recalculates IIR filter coefficients from other parameters and
     * <C>sampleRate</C> and updates IIR filter <C>effectDescriptor</C>
     * accordingly.
     *
     * @param[inout] effectDescriptor  effect descriptor of filter
     * @param[in]    sampleRate        new sample rate
     */
    static void _updateFilterCoefficients
                    (INOUT _EffectDescriptor_FLTR* effectDescriptor,
                     IN Real sampleRate)
    {
        Real b0 = 0.0;
        Real b1 = 0.0;
        Real b2 = 0.0;
        Real a0 = 0.0;
        Real a1 = 0.0;
        Real a2 = 0.0;

        if (effectDescriptor->kind == filterKind_biquad) {
            // we can just keep the direct coefficients, no calculation
            // is necessary
            b0 = effectDescriptor->b0;
            b1 = effectDescriptor->b1;
            b2 = effectDescriptor->b2;
            a0 = effectDescriptor->a0;
            a1 = effectDescriptor->a1;
            a2 = effectDescriptor->a2;
        } else {
            const String kind    = effectDescriptor->kind;
            const Real frequency = effectDescriptor->frequency;
            const Real bandwidth = effectDescriptor->bandwidth;
            const SoXFilterBandwidthUnit bandwidthUnit =
                effectDescriptor->bandwidthUnit;

            const Real w0 = Real::twoPi * frequency / sampleRate;
            const Real cw0 = Real::cos(w0);
            const Real sw0 = Real::sin(w0);
            const Real alpha = _alphaForBandwidth(sampleRate,
                                                  bandwidth,
                                                  bandwidthUnit,
                                                  frequency,
                                                  effectDescriptor->dBGain);
            const Real a =
                SoXAudioHelper::dBToLinear(effectDescriptor->dBGain, 40);

            if (kind == filterKind_allpass) {
                b0 =  Real{1} - alpha;
                b1 = -Real{2} * cw0;
                b2 =  Real{1} + alpha;
                a0 =  b2;
                a1 =  b1;
                a2 =  b0;
            } else if (kind == filterKind_band) {
                const Real bandwidthAsFrequency =
                         (bandwidthUnit == SoXFilterBandwidthUnit::quality
                          ? frequency / bandwidth
                          : (bandwidthUnit == SoXFilterBandwidthUnit::octaves
                             ? Real{(frequency * Real::power(2.0, bandwidth - 1)
                                     * Real::power(2.0, -bandwidth / 2))}
                             : bandwidth));
                a2 = (-Real::twoPi * bandwidthAsFrequency / sampleRate).exp();
                a1 = -Real{4} * a2 / (Real{1} + a2) * cw0;
                a0 = 1;
                b2 = 0;
                b1 = 0;
                b0 = Real::sqrt(Real{1} - a1.sqr()
                                / (Real{4} * a2)) * (Real{1} - a2);

                if (effectDescriptor->usesUnpitchedAudioMode) {
                    const Real factor =
                        Real::sqrt(((Real{1} + a2).sqr() - a1.sqr())
                                   * (Real{1} - a2) / (Real{1} + a2))
                        / b0;
                    b0 *= factor;
                }
            } else if (kind == filterKind_bandpass
                       || kind == filterKind_bandreject) {
                if (kind == filterKind_bandreject) {
                    b0 =  1;
                    b1 = -cw0 * 2;
                    b2 =  1;
                } else {
                    b0 =  (effectDescriptor->usesConstantSkirtGain
                           ? sw0 / 2 : alpha);
                    b1 =  0;
                    b2 = -b0;
                }

                a0 =  alpha + 1;
                a1 = -cw0 * 2;
                a2 = -alpha + 1;
            } else if (kind == filterKind_bass || kind == filterKind_treble) {
                const Real f = (kind == filterKind_bass ? 1 : -1);
                const Real sqrtAlphaA = Real{2} * Real::sqrt(a) * alpha;
                b0 =          a * ( (a+1) - f * (a-1) * cw0 + sqrtAlphaA );
                b1 =  f * 2 * a * ( (a-1) - f * (a+1) * cw0              );
                b2 =          a * ( (a+1) - f * (a-1) * cw0 - sqrtAlphaA );
                a0 =                (a+1) + f * (a-1) * cw0 + sqrtAlphaA;
                a1 = -f * 2 *     ( (a-1) + f * (a+1) * cw0              );
                a2 =                (a+1) + f * (a-1) * cw0 - sqrtAlphaA;
            } else if (kind == filterKind_equalizer) {
                const Real filterGain =
                    Real::power(10, (double) effectDescriptor->equGain / 40);
                b0  = Real{1} + alpha * filterGain;
                b1  = -Real{2} * cw0;
                b2  = Real{1} - alpha * filterGain;
                a0 = Real{1} + alpha / filterGain;
                a1 = b1;
                a2 = Real{1} - alpha / filterGain;
            } else if (kind == filterKind_highpass
                       || kind == filterKind_lowpass) {
                Real factorA, factorB, factorC;

                if (effectDescriptor->isSinglePole) {
                    if (kind == filterKind_highpass) {
                        factorA = -1;
                        factorB = 0.5;
                        factorC = -1;
                    } else {
                        factorA = 1;
                        factorB = 1;
                        factorC = 0;
                    }

                    a0 = 1;
                    a1 = -Real::exp(-w0);
                    a2 = 0;
                    b0  = (Real{1} + factorA * a1) * factorB;
                    b1  = factorC * b0;
                    b2  = 0;
                } else {
                    if (kind == filterKind_highpass) {
                        factorA = (Real{1} + cw0);
                        factorB = -1;
                    } else {
                        factorA = (Real{1} - cw0);
                        factorB = 1;
                    }

                    b0  = factorA / 2.0;
                    b1  = factorB * factorA;
                    b2  = b0;
                    a0 = Real{1} + alpha;
                    a1 = -Real{2} * cw0;
                    a2 = Real{1} - alpha;
                }
            }

            // update the internal variables
            effectDescriptor->b0 = b0;
            effectDescriptor->b1 = b1;
            effectDescriptor->b2 = b2;
            effectDescriptor->a0 = a0;
            effectDescriptor->a1 = a1;
            effectDescriptor->a2 = a2;
        }

        effectDescriptor->filter.set(b0, b1, b2, a0, a1, a2);
    }
};

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXFilter_AudioEffect::SoXFilter_AudioEffect ()
    : SoXAudioEffect{}
{
    _effectDescriptor = _createEffectDescriptor();
    _initializeKindParameter(_audioParameterMap);
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
    _EffectDescriptor_FLTR* effectDescriptor =
        static_cast<_EffectDescriptor_FLTR*>(_effectDescriptor);

    String st = "_EffectDescriptor_FLTR(";
    st += "kind = " + effectDescriptor->kind;
    st += ", frequency = " + TOSTRING(effectDescriptor->frequency) + "Hz";
    st += (", bandwidth = " + TOSTRING(effectDescriptor->bandwidth)
           + _bwUnitToString(effectDescriptor->bandwidthUnit));
    st += ", dBGain = " + TOSTRING(effectDescriptor->dBGain) + "dB";
    st += ", equGain = " + TOSTRING(effectDescriptor->equGain) + "dB";
    st += (", usesUnpitchedAudioMode = "
           + TOSTRING(effectDescriptor->usesUnpitchedAudioMode));
    st += (", usesConstantSkirtGain = "
           + TOSTRING(effectDescriptor->usesConstantSkirtGain));
    st += (", isSinglePole = "
           + TOSTRING(effectDescriptor->isSinglePole));
    st += ", b0 = " + TOSTRING(effectDescriptor->b0);
    st += ", b1 = " + TOSTRING(effectDescriptor->b1);
    st += ", b2 = " + TOSTRING(effectDescriptor->b2);
    st += ", a0 = " + TOSTRING(effectDescriptor->a0);
    st += ", a1 = " + TOSTRING(effectDescriptor->a1);
    st += ", a2 = " + TOSTRING(effectDescriptor->a2);
    st += ", filter = " + effectDescriptor->filter.toString();
    st += (", sampleQueueList = "
           + effectDescriptor->sampleQueueList.toString());
    st += ")";

    return st;
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXFilter_AudioEffect::name() const
{
    return "SoX Filter";
}

/*--------------------*/
/* parameter change   */
/*--------------------*/

SoXAudioValueChangeKind
SoXFilter_AudioEffect::_setValueInternal (IN String& parameterName,
                                          IN String& value,
                                          IN bool recalculationIsSuppressed)
{
    _EffectDescriptor_FLTR* effectDescriptor =
        static_cast<_EffectDescriptor_FLTR*>(_effectDescriptor);
    SoXAudioValueChangeKind result =
        SoXAudioValueChangeKind::parameterChange;

    if (parameterName == parameterName_kind) {
        _initializeAllParameters(_audioParameterMap, value);
        effectDescriptor->kind = _audioParameterMap.value(parameterName);
        result = SoXAudioValueChangeKind::globalChange;
    } else {
        if (parameterName == "a0") {
            effectDescriptor->a0 = _toReal(value);
        } else if (parameterName == "a1") {
            effectDescriptor->a1 = _toReal(value);
        } else if (parameterName == "a2") {
            effectDescriptor->a2 = _toReal(value);
        } else if (parameterName == "b0") {
            effectDescriptor->b0 = _toReal(value);
        } else if (parameterName == "b1") {
            effectDescriptor->b1 = _toReal(value);
        } else if (parameterName == "b2") {
            effectDescriptor->b2 = _toReal(value);
        } else if (parameterName == parameterName_bandwidth) {
            effectDescriptor->bandwidth = _toReal(value);
        } else if (parameterName == parameterName_bandwidthUnit) {
            effectDescriptor->bandwidthUnit = _toBWUnit(value);
        } else if (parameterName == parameterName_cstSkirtGain) {
            effectDescriptor->usesConstantSkirtGain = (value == "Yes");
        } else if (parameterName == parameterName_dBGain) {
            effectDescriptor->dBGain = _toReal(value);
        } else if (parameterName == parameterName_equGain) {
            effectDescriptor->equGain = _toReal(value);
        } else if (parameterName == parameterName_frequency) {
            effectDescriptor->frequency = _toReal(value);
        } else if (parameterName == parameterName_poleCount) {
            effectDescriptor->isSinglePole = (_toReal(value) == 1.0);
        } else if (parameterName == parameterName_unpitchedMode) {
            effectDescriptor->usesUnpitchedAudioMode = (value == "Yes");
        }

        if (!recalculationIsSuppressed) {
            _updateFilterCoefficients(effectDescriptor, _sampleRate);
        }
    }

    return result;
}

/*--------------------*/

void SoXFilter_AudioEffect::setDefaultValues () {
    const String filterKind =
        _audioParameterMap.value(parameterName_kind);
    _initializeAllParameters(_audioParameterMap, filterKind);
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXFilter_AudioEffect::prepareToPlay (IN Real sampleRate)
{
    if (sampleRate != _sampleRate) {
        _EffectDescriptor_FLTR* effectDescriptor =
            static_cast<_EffectDescriptor_FLTR*>(_effectDescriptor);
        _sampleRate = sampleRate;

        // filter has to be recalculated
        _updateFilterCoefficients(effectDescriptor, _sampleRate);
    }
}

/*--------------------*/

void
SoXFilter_AudioEffect::processBlock (IN Real timePosition,
                                     INOUT SoXAudioSampleBuffer& buffer)
{
    SoXAudioEffect::processBlock(timePosition, buffer);

    _EffectDescriptor_FLTR* effectDescriptor =
        static_cast<_EffectDescriptor_FLTR*>(_effectDescriptor);
    const Natural sampleCount = buffer[0].size();
     SoXIIRFilter& filter{effectDescriptor->filter};
    SoXAudioSampleMatrix& sampleQueueList =
        effectDescriptor->sampleQueueList;

    for (Natural channel = 0;  channel < _channelCount;
         channel++) {
        SoXAudioSampleQueue& inputSampleQueue{sampleQueueList
                                                .at(channel, 0)};
        SoXAudioSampleQueue& outputSampleQueue{sampleQueueList
                                                 .at(channel, 1)};
        const SoXAudioSampleList& inputList = buffer[channel];
        SoXAudioSampleList& outputList = buffer[channel];

        for (Natural i = 0;  i < sampleCount;  i++) {
            const SoXAudioSample inputSample = inputList[i];
            SoXAudioSample& outputSample     = outputList[i];
            inputSampleQueue.shiftRight(inputSample);
            outputSampleQueue.shiftRight(0.0);
            filter.apply(inputSampleQueue, outputSampleQueue);
            outputSample = outputSampleQueue.first();
        }
    }
}
