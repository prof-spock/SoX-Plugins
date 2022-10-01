/**
 * @file
 * The <C>SoXReverbSupport</C> body implements support classes for the
 * freeverb algorithm and provides a single <B>_SoXReverb</B> class as
 * its interface.
 *
 * The original algorithms can be found in the <A
 * HREF="https://sourceforge.net/p/sox/code">SoX repository</A>
 * in the file
 *   - "reverb.c" by Rob Sewell with the code for the reverb effect
 *     based on freeverb by Jezar at Dreampoint.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXReverbSupport.h"

#include "GenericTuple.h"
#include "NaturalList.h"
#include "StringUtil.h"
#include "SoXAudioHelper.h"

/*--------------------*/


using Audio::AudioSample;
using BaseModules::StringUtil;
using BaseTypes::Containers::GenericTuple;
using BaseTypes::Containers::NaturalList;
using SoXPlugins::Effects::SoXReverb::_SoXReverb;
using SoXPlugins::Helpers::SoXAudioHelper;

/*============================================================*/

namespace SoXPlugins::Effects::SoXReverb {

    /*--------------------*/
    /* CONSTANTS          */
    /*--------------------*/

    /** the maximum number of supported channels */
    static constexpr size_t _maxChannelCount = 100;

    /* Freeverb construction parameters */

    /** the number of freeverb comb filters */
    static constexpr size_t _lineCombFilterCount    = 8;

    /** the number of freeverb allpass filters */
    static constexpr size_t _lineAllpassFilterCount = 4;

    /** the stereo spread in samples in freeverb */
    static const Real _stereoSpread = 12.0;

    /** the allpass factor in freeverb */
    static const Real _allpassFactor = 0.5;

    /*--------------------*/
    /* TYPE DEFINITIONS   */
    /*--------------------*/

    /**
     * A <C>_SamplePair</C> is a pair of audio samples.
     */
    using _SamplePair = GenericTuple<AudioSample, 2>;

    /*====================*/
    /* Allpass Filter     */
    /*====================*/

    /**
     * An <C>_AllpassFilter</C> object is a simple allpass filter with
     * an associated sample ring buffer as a delay line
     */
    struct _AllpassFilter {

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        /**
         * Makes allpass filter with embedded sample ring buffer.
         */
        _AllpassFilter ();

        /*--------------------*/

        /**
         * Gets length of sample ring buffer in filter.
         *
         * @return length of sample ring buffer
         */
        Natural ringBufferLength () const;

        /*--------------------*/

        /**
         * Sets length of sample ring buffer in filter to
         * <C>length</C>.
         *
         * @param[in] length  new length of sample ring buffer
         */
        void setRingBufferLength (IN Natural length);

        /*--------------------*/
        /* filter application */
        /*--------------------*/

        /**
         * Applies allpass filter to single <C>inputSample</C> and
         * returns resulting sample.
         *
         * @param[in] inputSample  single input sample
         * @return  output sample of allpass
         */
        AudioSample apply (IN AudioSample inputSample);

        /*--------------------*/
        /*--------------------*/

        protected:

            /** internal sample ring buffer of allpass */
            AudioSampleRingBuffer _sampleRingBuffer;

    };

    /*====================*/
    /* Comb Filter        */
    /*====================*/

    /**
     * A <C>_CombFilter</C> object is a simple Schröder-Moorer comb
     * filter with an associated sample ring buffer as a delay line
     * and a single stored sample.
     */
    struct _CombFilter {

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        /**
         * Makes comb filter with embedded sample ring buffer.
         */
        _CombFilter ();

        /*--------------------*/

        /**
         * Gets length of sample ring buffer in filter
         *
         * @return length of sample ring buffer
         */
        Natural ringBufferLength () const;

        /*--------------------*/

        /**
         * Sets length of sample ring buffer in filter
         *
         * @param[in] length  new length of sample ring buffer
         */
        void setRingBufferLength (IN Natural length);

        /*--------------------*/
        /* filter application */
        /*--------------------*/

        /**
         * Applies comb filter to single <C>inputSample</C> with
         * parameters <C>feedback</C> and <C>hfDamping</C>.
         *
         * @param[in] inputSample  single input sample
         * @param[in] feedback     feedback of comb filter
         * @param[in] hfDamping    hfDamping of comb filter
         * @return  output sample of comb filter
         */
        AudioSample apply (IN AudioSample inputSample,
                              IN Real feedback, IN Real hfDamping);

        /*--------------------*/
        /*--------------------*/

        protected:

            /** internal sample ring buffer of comb filter */
            AudioSampleRingBuffer _sampleRingBuffer;

            /** the single state sample of comb filter */
            AudioSample _storedSample;

    };

    /*====================*/
    /* ReverbLine         */
    /*====================*/

    /**
     * A <C>_ReverbLine</C> object is a list of filters with delay
     * lines forming a single reverb channel in Freeverb.
     */
    struct _ReverbLine {

        /**
         * Constructs a complete reverb line and all filters; creates
         * all sample ring buffers with arbitrary length to be adapted
         * by later adjustments.
         */
        _ReverbLine ();

        /*--------------------*/

        /**
         * Destroys complete reverb line and all filters.
         */
        ~_ReverbLine ();

        /*--------------------*/

        /**
         * Returns a string representation of reverb line.
         *
         * @return  string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Adjusts lengths of sample ring buffers for reverb line
         * based on <C>sampleRate</C>, <C>roomScale</C> and
         * <C>stereoDepth</C>.
         *
         * @param[in] sampleRate   new sample rate of reverb line 
         * @param[in] roomScale    new room scale of reverb line 
         * @param[in] stereoDepth  new stereo depth of reverb line
         */
        void adjustRingBufferLengths (IN Real sampleRate,
                                 IN Real roomScale,
                                 IN Real stereoDepth);

        /*--------------------*/

        /**
         * Applies reverb line to single <C>inputSample</C> and
         * returns output sample with parameters <C>feedback</C>,
         * <C>hfDamping</C> and <C>gain</C>; the comb filters are
         * applied to the input in parallel, the allpass filters in
         * series.
         *
         * @param[in] inputSample  single input sample
         * @param[in] feedback     feedback parameter for comb filters
         * @param[in] hfDamping    hf damping parameter for comb filters
         * @param[in] gain         the gain of the reverb line
         * @return  output sample of reverb line
         */
        AudioSample apply (IN AudioSample inputSample,
                              IN Real feedback,
                              IN Real hfDamping,
                              IN Real gain);

        /*--------------------*/
        /*--------------------*/

        protected:

            /** the list of all allpass filters in reverb line */
            GenericTuple<_AllpassFilter*,
                         _lineAllpassFilterCount> _allpassFilterList;

            /** the list of all comb filters in reverb line */
            GenericTuple<_CombFilter*,
                         _lineCombFilterCount>    _combFilterList;

    };

    /*====================*/
    /* Reverb Channel     */
    /*====================*/

    /**
     * A <C>_ReverbChannel</C> object processes a single stereo
     * channel via either one or two reverb lines (depending on
     * whether stereo depth is greater than 0 or not); also there is a
     * delay of the input sample via a channel delay line (when
     * predelay is non-zero)
     */
    struct _ReverbChannel {

        /**
         * Constructs a complete reverb line with predelay line and
         * all filters for one channel; creates all sample ring
         * buffers with arbitrary length to be adapted by later
         * adjustments.
         */
        _ReverbChannel ();

        /*--------------------*/

        /**
         * Destroys a complete reverb line with predelay line and all
         * filters for one channel.
         */
        ~_ReverbChannel ();

        /*--------------------*/

        /**
         * Returns a string representation of reverb channel.
         *
         * @return  string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Adjusts lengths of all filter ring buffers in reverb
         * channel according to parameters <C>sampleRate</C>,
         * <C>roomScale</C> and <C>stereoDepth</C> to their effective
         * length; adjusts input ring buffer to length
         * <C>predelay</C>.
         *
         * @param[in] sampleRate   new sample rate for reverb channel
         * @param[in] predelay     new predelay for reverb channel
         * @param[in] roomScale    new room scale for reverb channel
         * @param[in] stereoDepth  new stereo depth for reverb channel
         */
        void adjustRingBufferLengths (IN Real sampleRate,
                                 IN Real predelay,
                                 IN Real roomScale,
                                 IN Real stereoDepth);

        /*--------------------*/

        /**
         * Applies current reverb channel with parameters
         * <C>feedback</C>, <C>hfDamping</C> and <C>gain</C> to single
         * <C>inputSample</C> and returns output samples in
         * <C>wetSamplePair</C>.
         *
         * @param[in]  inputSample    single input sample
         * @param[in]  feedback       feedback of comb filters
         * @param[in]  hfDamping      hfDamping of comb filters
         * @param[in]  gain           the gain of the reverb lines
         * @param[out] wetSamplePair  pair of output samples
         */
        void apply (IN AudioSample inputSample,
                    IN Real feedback,
                    IN Real hfDamping,
                    IN Real gain,
                    OUT _SamplePair& wetSamplePair);

        /*--------------------*/
        /*--------------------*/

        protected:

            /** the ring buffer of input samples in this reverb channel */
            AudioSampleRingBuffer _inputSampleRingBuffer;

            /** the number of associated reverb lines (typically 1 or 2) */
            Natural _reverbLineCount;

            /** the list of associated reverb lines */
            GenericSequence<_ReverbLine*> _reverbLineList;

    };

    /*--------------------*/

    /** a list of reverb channels */
    using _ReverbChannelList = GenericSequence<_ReverbChannel*>;

    /*====================*/
    /* Reverb             */
    /*====================*/

    /**
     * A <C>_ReverbEffectParameterData</C> object encapsulates the
     * freeverb parameters like e.g.\ feedback, stereo depth and room
     * scale.
     */
    struct _ReverbEffectParameterData {

        /** information whether direct signal should be suppressed in
         * output */
        Boolean isWetOnly;

        /** the feedback as a factor */
        Real feedback;

        /** the high frequency damping as a factor */
        Real hfDamping;

        /** the stereo depth as a factor */
        Real stereoDepth;

        /** the wet gain as a factor */
        Real wetGain;

        /** the predelay in seconds */
        Real predelay;

        /** the room scale as a factor */
        Real roomScale;

        /** the number of channels in this reverb */
        Natural channelCount;

        /** the list of reverb channels */
        _ReverbChannelList reverbChannelList{};
    };

    /*============================================================*/

    /* the maximum value for the room scale */
    static const Real _maximumRoomScale   = 1.0;

    /* the maximum value for the stereo depth */
    static const Real _maximumStereoDepth = 1.0;

    /* the maximum value for the predelay in seconds */
    static const Real _maximumPredelay    = 0.5;

    /** arbitrary default used for construction of delay lines */
    static const Real _defaultSampleRate = 100.0;

    /** reference sample rate for reverb line delays */
    static const Real _referenceSampleRate = 44100.0;

    /** list of sample ring buffer lengths for comb filters */
    static const NaturalList _combFilterLengthList =
        NaturalList::fromList({Natural{1116}, Natural{1188},
                               Natural{1277}, Natural{1356},
                               Natural{1422}, Natural{1491},
                               Natural{1557}, Natural{1617}});

    /** list of sample ring buffer lengths for allpass filters */
    static const NaturalList _allpassFilterLengthList =
        NaturalList::fromList({Natural{225}, Natural{341},
                               Natural{441}, Natural{556}});

    /*============================================================*/

    _AllpassFilter::_AllpassFilter ()
        : _sampleRingBuffer{}
    {
    }

    /*--------------------*/

    Natural _AllpassFilter::ringBufferLength () const
    {
        return _sampleRingBuffer.length();
    }

    /*--------------------*/

    void _AllpassFilter::setRingBufferLength (IN Natural length)
    {
        _sampleRingBuffer.setLength(length);
    }

    /*--------------------*/

    AudioSample _AllpassFilter::apply (IN AudioSample inputSample)
    {
        const AudioSample outputSample = _sampleRingBuffer.first();
        const AudioSample newSample =
            inputSample + outputSample * _allpassFactor;
        _sampleRingBuffer.shiftLeft(newSample);
        return outputSample - inputSample;
    }

    /*--------------------*/

    _CombFilter::_CombFilter ()
        : _sampleRingBuffer{},
          _storedSample{0.0}
    {
    }

    /*--------------------*/

    Natural _CombFilter::ringBufferLength () const
    {
        return _sampleRingBuffer.length();
    }

    /*--------------------*/

    void _CombFilter::setRingBufferLength (IN Natural length)
    {
        _sampleRingBuffer.setLength(length);
    }

    /*--------------------*/

    AudioSample _CombFilter::apply (IN AudioSample inputSample,
                                       IN Real feedback,
                                       IN Real hfDamping)
    {
        const AudioSample outputSample = _sampleRingBuffer.first();
        _storedSample = (outputSample
                         + (_storedSample - outputSample) * hfDamping);
        _sampleRingBuffer.shiftLeft(inputSample + _storedSample * feedback);
        return outputSample;
    }

    /*============================================================*/

    /**
     * Returns length of filter delay line for given parameters;
     * if <C>isCreation</C> is set, the maximum length for those
     * parameters is returned.
     *
     * @param[in] isCreation    tells whether reverb line effective
     *                          length or maximum length should be
     *                          calculated
     * @param[in] isCombFilter  tells whether length for comb filter
     *                          or allpass
     *                          filter should be calculated
     * @param[in] index         the index of the filter (starting at zero)
     * @param[in] sampleRate    the sample rate of reverb
     * @param[in] cRoomScale    the room scale of reverb
     * @param[in] stereoDepth   the desired stereo depth of reverb
     * @return  length of filter sample ring buffer
     */
    static
    Natural _reverbLineDelayLength (IN Boolean isCreation,
                                    IN Boolean isCombFilter,
                                    IN Natural index,
                                    IN Real sampleRate,
                                    IN Real cRoomScale,
                                    IN Real stereoDepth)
    {
        const Real roomScale = (isCreation ? _maximumRoomScale
                                : (isCombFilter ? cRoomScale : 1.0));
        const Real factor = sampleRate / _referenceSampleRate * roomScale;
        const Integer sign = (index % 2 == 0 ? 1 : -1);
        const Real offset = (isCreation ? _maximumStereoDepth
                             : stereoDepth * sign);
        const NaturalList& lengthList = (isCombFilter
                                         ? _combFilterLengthList
                                         : _allpassFilterLengthList);
        const Natural length = lengthList[index];
        const Real adjustment = _stereoSpread;
        const Natural result =
            Natural{Real::round(factor
                                * (Real{length} + adjustment * offset))};
        return result;
    }

    /*--------------------*/

    /**
     * Returns initial filter delay line maximum length for the
     * sampleRingBuffer for given parameters.
     *
     * @param[in] isCombFilter  tells to adapt comb filter or allpass
     *                          filter
     * @param[in] index         the index of the filter (starting at zero)
     * @param[in] sampleRate    the sample rate of reverb
     * @return maximum ring buffer length (for later setup)
     */
    static
    Natural _initialReverbLineDelayLength (IN Boolean isCombFilter,
                                           IN Natural index,
                                           IN Real sampleRate)
    {
        // calculate maximum ring buffer length with arbitrary values for
        // <channel>, <roomScale> and <stereoDepth>
        return _reverbLineDelayLength(true, isCombFilter,
                                      index, sampleRate, 0.0, 0.0);
    }

    /*--------------------*/

    /**
     * Returns adapted filter delay line length for the sample ring
     * buffer for given parameters <C>isCombFilter</C>,
     * <C>sampleRate</C>, <C>roomScale</C> and <C>stereoDepth</C> to
     * their effective length.
     *
     * @param[in] isCombFilter  tells whether length for comb filter or
     *                          allpass filter should be calculated
     * @param[in] index         the index of the filter (starting at zero)
     * @param[in] sampleRate    the sample rate of reverb
     * @param[in] roomScale     the room scale of reverb
     * @param[in] stereoDepth   the desired stereo depth of reverb
     * @return  adapted length of filter sample ring buffer
     */
    static
    Natural _adjustedReverbLineDelayLength (IN Boolean isCombFilter,
                                            IN Natural index,
                                            IN Real sampleRate,
                                            IN Real roomScale,
                                            IN Real stereoDepth)
    {
        return _reverbLineDelayLength(false, isCombFilter, index,
                                      sampleRate, roomScale, stereoDepth);
    }

    /*============================================================*/

    _ReverbLine::_ReverbLine ()
        : _allpassFilterList{},
          _combFilterList{}
    {
        // set allpass filters delay line lengths
        for (Natural i = 0;  i < _lineAllpassFilterCount;  i++) {
            _AllpassFilter* allpassFilter = new _AllpassFilter();
            const Natural length =
                _initialReverbLineDelayLength(false, i, _defaultSampleRate);
            allpassFilter->setRingBufferLength(length);
            _allpassFilterList[i] = allpassFilter;
        }

        // set comb filters delay line lengths
        for (Natural i = 0;  i < _lineCombFilterCount;  i++) {
            _CombFilter* combFilter = new _CombFilter();
            const Natural length =
                _initialReverbLineDelayLength(true, i, _defaultSampleRate);
            combFilter->setRingBufferLength(length);
            _combFilterList[i] = combFilter;
        }
    }

    /*--------------------*/

    _ReverbLine::~_ReverbLine ()
    {
        for (_AllpassFilter* filter : _allpassFilterList) {
            delete filter;
        }

        for (_CombFilter* filter : _combFilterList) {
            delete filter;
        }
    }

    /*--------------------*/

    String _ReverbLine::toString () const {
        String st;

        st = "ReverbLine(";

        for (Natural i = 0;  i < _lineAllpassFilterCount;  i++) {
            const _AllpassFilter* filter = _allpassFilterList[i];
            const Natural ringBufferLength = filter->ringBufferLength();
            st += ((i > 0 ? ", af(" : "af(")
                   + TOSTRING(i) + ")="
                   + TOSTRING(ringBufferLength));
        }

        st += ", ";

        for (Natural i = 0;  i < _lineCombFilterCount;  i++) {
            const _CombFilter* filter = _combFilterList[i];
            const Natural ringBufferLength = filter->ringBufferLength();
            st += ((i > 0 ? ", cf(" : "cf(")
                   + TOSTRING(i) + ")="
                   + TOSTRING(ringBufferLength));
        }

        st += ")";
        return st;
    }

    /*--------------------*/

    void _ReverbLine::adjustRingBufferLengths (IN Real sampleRate,
                                          IN Real roomScale,
                                          IN Real stereoDepth)
    {
        // adjust allpass filter delay lines
        for (Natural i = 0;  i < _lineAllpassFilterCount;  i++) {
            _AllpassFilter* allpassFilter = _allpassFilterList[i];
            const Natural length =
                _adjustedReverbLineDelayLength(false, i, sampleRate,
                                               roomScale, stereoDepth);
            allpassFilter->setRingBufferLength(length);
        }

        // adjust comb filter delay lines
        for (Natural i = 0;  i < _lineCombFilterCount;  i++) {
            _CombFilter* combFilter = _combFilterList[i];
            const Natural length =
                _adjustedReverbLineDelayLength(true, i, sampleRate,
                                               roomScale, stereoDepth);
            combFilter->setRingBufferLength(length);
        }
    }

    /*--------------------*/

    AudioSample _ReverbLine::apply (IN AudioSample inputSample,
                                       IN Real feedback,
                                       IN Real hfDamping,
                                       IN Real gain)
    {
        // route input sample through the filters
        AudioSample outputSample = 0.0;

        // process comb filters in parallel
        for (_CombFilter* filter : _combFilterList) {
            outputSample += filter->apply(inputSample, feedback, hfDamping);
        }

        // process allpass filters in series
        for (_AllpassFilter* filter : _allpassFilterList) {
            outputSample = filter->apply(outputSample);
        }

        outputSample *= gain;
        return outputSample;
    }

    /*============================================================*/

    _ReverbChannel::_ReverbChannel ()
        : _inputSampleRingBuffer{0},
          _reverbLineCount{2},
          _reverbLineList{2}
    {
        for (_ReverbLine*& reverbLine : _reverbLineList) {
            reverbLine = new _ReverbLine();
        }
    }

    /*--------------------*/

    _ReverbChannel::~_ReverbChannel ()
    {
        for (_ReverbLine* reverbLine : _reverbLineList) {
            delete reverbLine;
        }
    }

    /*--------------------*/

    String _ReverbChannel::toString () const
    {
        String st = ("ReverbChannel("
                     "predelay = "
                     + TOSTRING(_inputSampleRingBuffer.length()));

        // add information about reverb lines
        for (const _ReverbLine* reverbLine : _reverbLineList) {
            st += ", " + reverbLine->toString();
        }

        st += ")";
        return st;
    }

    /*--------------------*/

    void _ReverbChannel::adjustRingBufferLengths (IN Real sampleRate,
                                             IN Real predelay,
                                             IN Real roomScale,
                                             IN Real stereoDepth)
    {
        const Natural ringBufferLength =
            Natural{Real::round(predelay * sampleRate)};
        _inputSampleRingBuffer.setLength(ringBufferLength);

        // adapt lengths of reverb lines; when stereo depth is zero,
        // only a single reverb line is used per channel
        _reverbLineCount = (stereoDepth == 0.0 ? 1 : 2);
        Real effectiveStereoDepth = 0.0;

        for (_ReverbLine* reverbLine : _reverbLineList) {
            reverbLine->adjustRingBufferLengths(sampleRate, roomScale,
                                           effectiveStereoDepth);
            effectiveStereoDepth = stereoDepth;
        }
    }

    /*--------------------*/

    void _ReverbChannel::apply (IN AudioSample cInputSample,
                                IN Real feedback,
                                IN Real hfDamping,
                                IN Real gain,
                                OUT _SamplePair& wetSamplePair)
    {
        AudioSample inputSample = cInputSample;

        // check and process predelay
        if (_inputSampleRingBuffer.length() > 0) {
            const AudioSample firstSample = _inputSampleRingBuffer.first();
            _inputSampleRingBuffer.shiftLeft(inputSample);
            inputSample = firstSample;
        }

        // process all reverb lines for this channel and store their
        // results in output sample list
        for (Natural i = 0;  i < _reverbLineCount;  i++) {
            _ReverbLine* reverbLine = _reverbLineList[i];
            const AudioSample outputSample =
                reverbLine->apply(inputSample, feedback, hfDamping, gain);
            wetSamplePair[i] = outputSample;
        }
    }

}

/*============================================================*/

_SoXReverb::_SoXReverb ()
{
    _ReverbEffectParameterData* effectParameterData =
        new _ReverbEffectParameterData();
    _effectParameterData = effectParameterData;

    effectParameterData->isWetOnly    = false;
    effectParameterData->feedback     = 0.0;
    effectParameterData->hfDamping    = 0.0;
    effectParameterData->stereoDepth  = 0.0;
    effectParameterData->wetGain      = 0.0;
    effectParameterData->predelay     = 0.0;
    effectParameterData->roomScale    = 10.0;
    effectParameterData->channelCount = 0;
    effectParameterData->reverbChannelList.clear();
}

/*--------------------*/

_SoXReverb::~_SoXReverb ()
{
    // get rid of all embedded reverb channels by resizing to zero
    // length
    resize(_defaultSampleRate, 0);
}

/*--------------------*/

void _SoXReverb::setParameters (IN Boolean isWetOnly,
                                IN Percentage cReverberance,
                                IN Percentage cHfDamping,
                                IN Percentage cRoomScale,
                                IN Percentage cStereoDepth,
                                IN Real cPredelay,
                                IN Real cWetDbGain)
{
    _ReverbEffectParameterData* effectParameterData =
        static_cast<_ReverbEffectParameterData*>(_effectParameterData);

    // adjust parameter value ranges
    const Percentage reverberance = cReverberance.forceToPercentage();
    const Percentage hfDamping = cHfDamping.forceToPercentage();
    const Percentage roomScale = cRoomScale.forceToPercentage();
    const Percentage stereoDepth = cStereoDepth.forceToPercentage();
    const Real predelay = cPredelay.forceToRange(0.0, _maximumPredelay);
    const Real wetDbGain = cWetDbGain.forceToRange(-10.0, 10.0);

    // calculate technical parameters
    const Real minimumFeedback =  -1 / log(1 - 0.3);
    const Real maximumFeedback =
        (Real{100.0} / (Real{1 - 0.98}.log() * minimumFeedback + 1.0));

    // set reverb parameters
    effectParameterData->isWetOnly   = isWetOnly;
    effectParameterData->feedback    =
        Real{1.0} - Real::exp((Real{reverberance} - maximumFeedback)
                            / (minimumFeedback * maximumFeedback));
    effectParameterData->hfDamping   = hfDamping / 100.0 * 0.3 + 0.2;
    effectParameterData->predelay    = predelay;
    effectParameterData->stereoDepth = stereoDepth / 100.0;
    effectParameterData->roomScale   = roomScale / 100.0 * 0.9 + 0.1;
    effectParameterData->wetGain     =
        SoXAudioHelper::dBToLinear(wetDbGain) * 0.015;
}

/*--------------------*/

void _SoXReverb::resize (IN Real sampleRate,
                         IN Natural channelCount)
{
    _ReverbEffectParameterData* effectParameterData =
        static_cast<_ReverbEffectParameterData*>(_effectParameterData);

    effectParameterData->channelCount = channelCount;
    _ReverbChannelList& reverbChannelList =
        effectParameterData->reverbChannelList;

    const Natural oldChannelCount = reverbChannelList.size();

    // get rid of extraneous channels
    for (Natural channel = channelCount;
         channel < oldChannelCount;  channel++) {
        delete reverbChannelList[channel];
    }

    reverbChannelList.setLength(channelCount);

    // create new channels
    for (Natural channel = oldChannelCount;
         channel < channelCount;  channel++) {
        reverbChannelList[channel] = new _ReverbChannel();
    }

    for (_ReverbChannel* reverbChannel : reverbChannelList) {
        reverbChannel->adjustRingBufferLengths
                           (sampleRate,
                            effectParameterData->predelay,
                            effectParameterData->roomScale,
                            effectParameterData->stereoDepth);
    }
}

/*--------------------*/

void _SoXReverb::apply (IN AudioSampleRingBuffer& inputSampleList,
                        OUT AudioSampleRingBuffer& outputSampleList)
{
    _ReverbEffectParameterData* effectParameterData =
        static_cast<_ReverbEffectParameterData*>(_effectParameterData);

    AudioSample inputSample;
    _ReverbChannelList& reverbChannelList =
        effectParameterData->reverbChannelList;
    const Natural channelCount = effectParameterData->channelCount;
    GenericTuple<_SamplePair, _maxChannelCount> wetSamplePairList;

    // when stereo depth is non-zero for a reverb, each reverb channel
    // produces a pair of output samples to be stored in a list of wet
    // result pairs
    for (Natural channel = 0;  channel < channelCount;  channel++) {
        // collect all wet sample pairs
        _SamplePair& wetSamplePair = wetSamplePairList[channel];
        _ReverbChannel* reverbChannel = reverbChannelList[channel];
        inputSample = inputSampleList[channel];
        reverbChannel->apply(inputSample,
                             effectParameterData->feedback,
                             effectParameterData->hfDamping,
                             effectParameterData->wetGain,
                             wetSamplePair);
    }

    // combine wet sample pairs with input samples
    const Boolean hasMultipleLines =
        (effectParameterData->stereoDepth > 0.0 && channelCount == 2);

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        _SamplePair& wetSamplePair = wetSamplePairList[channel];
        AudioSample outputSample;

        if (!hasMultipleLines) {
            outputSample = wetSamplePair[0];
        } else {
            const _SamplePair& otherWetSamplePair =
                wetSamplePairList[Natural{1} - channel];
            outputSample = (wetSamplePair[channel]
                            + otherWetSamplePair[channel]) / 2.0;
        }

        inputSample  = inputSampleList[channel];
        outputSample += (effectParameterData->isWetOnly ? 0.0
                         : inputSample);
        outputSampleList[channel] = outputSample;
    }
}

/*--------------------*/

String _SoXReverb::toString () const
{
    _ReverbEffectParameterData* effectParameterData =
        static_cast<_ReverbEffectParameterData*>(_effectParameterData);

    const Natural channelCount =
        effectParameterData->channelCount;
    String st =
        ("Reverb("
         "isWetOnly = " + TOSTRING(effectParameterData->isWetOnly)
         + ", feedback = " + TOSTRING(effectParameterData->feedback)
         + ", hfDamping = " + TOSTRING(effectParameterData->hfDamping)
         + ", predelay = " + TOSTRING(effectParameterData->predelay)
         + ", stereoDepth = " + TOSTRING(effectParameterData->stereoDepth)
         + ", wetGain = " + TOSTRING(effectParameterData->wetGain)
         + ", roomScale = " + TOSTRING(effectParameterData->roomScale)
         + ", channelCount = " + TOSTRING(channelCount)
         + ", channels = (");

    // add information about reverb channels
    for (Natural channel = 0;   channel < channelCount;
         channel++) {
        _ReverbChannel* reverbChannel =
            effectParameterData->reverbChannelList[channel];
        st += (channel == 0 ? "" : ", ") + reverbChannel->toString();
    }

    st += "))";
    return st;
}
