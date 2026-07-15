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

#include <algorithm>
#include <numeric>

#include "SoXReverbSupport.h"

#include "DelayLine.h"
#include "GenericTuple.h"
#include "Logging.h"
#include "SoXAudioHelper.h"

/*--------------------*/

using Audio::AudioDataPoint;
using Audio::DelayLine;
using BaseTypes::GenericTypes::GenericTuple;
using SoXPlugins::Effects::SoXReverb::_SoXReverb;
using SoXPlugins::Helpers::SoXAudioHelper;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/*============================================================*/

/*--------------------*/
/* CONSTANTS          */
/*--------------------*/

/** the maximum number of supported channels */
static constexpr size_t _maxChannelCount = 100;

/* Freeverb construction parameters */

/** the number of freeverb comb filters */
static constexpr size_t _lineCombFilterCount    = 8;

/** list of delay line lengths for comb filters in the
 * Freeverb algorithm */
static const RealList _combFilterLengthList =
    RealList::fromList({Real{1116.0}, Real{1188.0},
                        Real{1277.0}, Real{1356.0},
                        Real{1422.0}, Real{1491.0},
                        Real{1557.0}, Real{1617.0}});

/** the number of freeverb allpass filters */
static constexpr size_t _lineAllpassFilterCount = 4;

/** list of delay line lengths for allpass filters in the
 * Freeverb algorithm */
static const RealList _allpassFilterLengthList =
    RealList::fromList({Real{225.0}, Real{341.0},
                        Real{441.0}, Real{556.0}});

/** the stereo spread in data points in freeverb */
static const Real _stereoSpread = 12.0;

/** the allpass factor in freeverb */
static const Real _allpassFactor = 0.5;

/** the maximum value for the room scale */
static const Real _maximumRoomScale   = 1.0;

/** the maximum value for the stereo depth */
static const Real _maximumStereoDepth = 1.0;

/** the maximum value for the predelay in seconds */
static const Real _maximumPredelay    = 0.5;

/** arbitrary default used for construction of delay lines */
static const Real _defaultDataPointRate = 100.0;

/** reference data point rate for reverb line delays */
static const Real _referenceDataPointRate = 44100.0;

/*============================================================*/

namespace SoXPlugins::Effects::SoXReverb {

    /*--------------------*/
    /* TYPE DEFINITIONS   */
    /*--------------------*/

    /**
     * A <C>_DataPointPair</C> is a pair of audio data points.
     */
    using _DataPointPair = GenericTuple<AudioDataPoint, 2>;

    /*====================*/
    /* Allpass Filter     */
    /*====================*/

    /**
     * An <C>_AllpassFilter</C> object is a simple allpass filter with
     * an associated delay line
     */
    struct _AllpassFilter {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Makes allpass filter with embedded delay line.
         */
        _AllpassFilter ();

        /*--------------------*/
        /* property access    */
        /*--------------------*/

        /**
         * Gets length of delay line in filter.
         *
         * @return length of delay line
         */
        Real delayLineLength () const;

        /*--------------------*/
        /* property change    */
        /*--------------------*/

        /**
         * Sets length of delay line in filter to
         * <C>length</C>.
         *
         * @param[in] length  new length of delay line
         */
        void setDelayLineLength (IN Real length);

        /*--------------------*/
        /* complex change     */
        /*--------------------*/

        /**
         * Applies allpass filter to single <C>inputDataPoint</C> and
         * returns resulting data point.
         *
         * @param[in] inputDataPoint  single input data point
         * @return  output data point of allpass
         */
        AudioDataPoint apply (IN AudioDataPoint inputDataPoint);

        /*--------------------*/
        /*--------------------*/

        private:

            /** internal delay line of allpass */
            DelayLine _delayLine;

    };

    /*====================*/
    /* Comb Filter        */
    /*====================*/

    /**
     * A <C>_CombFilter</C> object is a simple Schröder-Moorer comb
     * filter with an associated delay line and a single stored data
     * point.
     */
    struct _CombFilter {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Makes comb filter with embedded delay line.
         */
        _CombFilter ();

        /*--------------------*/
        /* property access    */
        /*--------------------*/

        /**
         * Gets length of delay line in filter
         *
         * @return length of delay line
         */
        Real delayLineLength () const;

        /*--------------------*/
        /* property change    */
        /*--------------------*/

        /**
         * Sets length of delay line in filter
         *
         * @param[in] length  new length of delay line
         */
        void setDelayLineLength (IN Real length);

        /*--------------------*/
        /* complex change     */
        /*--------------------*/

        /**
         * Applies comb filter to single <C>inputDataPoint</C> with
         * parameters <C>feedback</C> and <C>hfDamping</C>.
         *
         * @param[in] inputDataPoint  single input data point
         * @param[in] feedback        feedback of comb filter
         * @param[in] hfDamping       hfDamping of comb filter
         * @return  output data point of comb filter
         */
        AudioDataPoint apply (IN AudioDataPoint inputDataPoint,
                              IN Real feedback, IN Real hfDamping);

        /*--------------------*/
        /*--------------------*/

        private:

            /** internal delay line of comb filter */
            DelayLine _delayLine;

            /** the single state data point of comb filter */
            AudioDataPoint _storedDataPoint;

    };

    /*====================*/
    /* ReverbLine         */
    /*====================*/

    /**
     * A <C>_ReverbLine</C> object is a list of filters with delay
     * lines forming a single reverb channel in Freeverb.
     */
    struct _ReverbLine {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Constructs a complete reverb line and all filters; creates
         * all delay lines with arbitrary length to be adapted
         * by later adjustments.
         */
        _ReverbLine ();

        /*--------------------*/

        /**
         * Destroys complete reverb line and all filters.
         */
        ~_ReverbLine ();

        /*--------------------*/
        /* type conversion    */
        /*--------------------*/

        /**
         * Returns a string representation of reverb line.
         *
         * @return  string representation
         */
        String toString () const;

        /*--------------------*/
        /* measurement        */
        /*--------------------*/

        /**
         * Returns length of the maximum delay path in data points.
         *
         * @return  maximum path length (in data points)
         */
        Real maximumDelayPathLength () const;

        /*--------------------*/
        /* property change    */
        /*--------------------*/

        /**
         * Adjusts lengths of delay lines for reverb line
         * based on <C>dataPointRate</C>, <C>roomScale</C> and
         * <C>stereoDepth</C>.
         *
         * @param[in] dataPointRate   new data point rate of reverb line 
         * @param[in] roomScale       new room scale of reverb line 
         * @param[in] stereoDepth     new stereo depth of reverb line
         */
        void adjustDelayLineLengths (IN Real dataPointRate,
                                      IN Real roomScale,
                                      IN Real stereoDepth);

        /*--------------------*/
        /* complex change     */
        /*--------------------*/

        /**
         * Applies reverb line to single <C>inputDataPoint</C> and
         * returns output data point with parameters <C>feedback</C>,
         * <C>hfDamping</C> and <C>gain</C>; the comb filters are
         * applied to the input in parallel, the allpass filters in
         * series.
         *
         * @param[in] inputDataPoint  single input data point
         * @param[in] feedback        feedback parameter for comb filters
         * @param[in] hfDamping       hf damping parameter for comb filters
         * @param[in] gain            the gain of the reverb line
         * @return  output data point of reverb line
         */
        AudioDataPoint apply (IN AudioDataPoint inputDataPoint,
                              IN Real feedback,
                              IN Real hfDamping,
                              IN Real gain);

        /*--------------------*/
        /*--------------------*/

        private:

            /** the list of all allpass filters in reverb line */
            GenericTuple<_AllpassFilter*,
                         _lineAllpassFilterCount> _allpassFilterList;

            /** the list of all comb filters in reverb line */
            GenericTuple<_CombFilter*,
                         _lineCombFilterCount>    _combFilterList;

    };

    /*====================*/

    /**
     * A <C>_ReverbLinePtr</C> object is a pointer to a reverb line.
     */
    typedef _ReverbLine* _ReverbLinePtr;

    /*====================*/
    /* Reverb Channel     */
    /*====================*/

    /**
     * A <C>_ReverbChannel</C> object processes a single stereo
     * channel via either one or two reverb lines (depending on
     * whether stereo depth is greater than 0 or not); also there is a
     * delay of the input data point via a channel delay line (when
     * predelay is non-zero)
     */
    struct _ReverbChannel {

        /*--------------------*/
        /* con-/destruction   */
        /*--------------------*/

        /**
         * Constructs a complete reverb line with predelay line and
         * all filters for one channel; creates all data point ring
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
        /* type conversion    */
        /*--------------------*/

        /**
         * Returns a string representation of reverb channel.
         *
         * @return  string representation
         */
        String toString () const;

        /*--------------------*/
        /* measurement        */
        /*--------------------*/

        /**
         * Returns length of the maximum delay path in data points.
         *
         * @return  maximum path length (in data points)
         */
        Real maximumDelayPathLength () const;

        /*--------------------*/
        /* property change    */
        /*--------------------*/

        /**
         * Adjusts lengths of all filter ring buffers in reverb
         * channel according to parameters <C>dataPointRate</C>,
         * <C>roomScale</C> and <C>stereoDepth</C> to their effective
         * length; adjusts input ring buffer to length
         * <C>predelay</C>.
         *
         * @param[in] dataPointRate  new data point rate for reverb channel
         * @param[in] predelay       new predelay for reverb channel
         * @param[in] roomScale      new room scale for reverb channel
         * @param[in] stereoDepth    new stereo depth for reverb channel
         */
        void adjustDelayLineLengths (IN Real dataPointRate,
                                      IN Real predelay,
                                      IN Real roomScale,
                                      IN Real stereoDepth);

        /*--------------------*/
        /* complex change     */
        /*--------------------*/

        /**
         * Applies current reverb channel with parameters
         * <C>feedback</C>, <C>hfDamping</C> and <C>gain</C> to single
         * <C>inputDataPoint</C> and returns output data points in
         * <C>wetDataPointPair</C>.
         *
         * @param[in]  inputDataPoint    single input data point
         * @param[in]  feedback          feedback of comb filters
         * @param[in]  hfDamping         hfDamping of comb filters
         * @param[in]  gain              the gain of the reverb lines
         * @param[out] wetDataPointPair  pair of output data points
         */
        void apply (IN AudioDataPoint inputDataPoint,
                    IN Real feedback,
                    IN Real hfDamping,
                    IN Real gain,
                    OUT _DataPointPair& wetDataPointPair);

        /*--------------------*/
        /*--------------------*/

        private:

            /** the delay line of input data points in this reverb
             * channel */
            DelayLine _inputDelayLine;

            /** the number of associated reverb lines (typically 1 or
             * 2) */
            Natural _reverbLineCount;

            /** the list of associated reverb lines */
            GenericList<_ReverbLinePtr> _reverbLineList;

    };

    /*--------------------*/

    /** a list of reverb channels */
    using _ReverbChannelList = GenericList<_ReverbChannel*>;

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

        /** the current data point rate for this reverb */
        Real dataPointRate;

        /** the list of reverb channels */
        _ReverbChannelList reverbChannelList{};

        /*--------------------*/
        /*--------------------*/

        /**
         * Returns string representation of parameter data object.
         *
         * @return string representation
         */
        String toString () const;

    };
}

/*====================*/

using SoXPlugins::Effects::SoXReverb::_AllpassFilter;
using SoXPlugins::Effects::SoXReverb::_CombFilter;
using SoXPlugins::Effects::SoXReverb::_DataPointPair;
using SoXPlugins::Effects::SoXReverb::_ReverbChannel;
using SoXPlugins::Effects::SoXReverb::_ReverbChannelList;
using SoXPlugins::Effects::SoXReverb::_ReverbEffectParameterData;
using SoXPlugins::Effects::SoXReverb::_ReverbLine;
using SoXPlugins::Effects::SoXReverb::_ReverbLinePtr;

/*============================================================*/

/**
 * Returns length of filter delay line for given parameters;
 * if <C>isCreation</C> is set, the maximum length for those
 * parameters is returned.
 *
 * @param[in] isCreation     tells whether reverb line effective
 *                           length or maximum length should be
 *                           calculated
 * @param[in] isCombFilter   tells whether length for comb filter
 *                           or allpass
 *                           filter should be calculated
 * @param[in] index          the index of the filter (starting at zero)
 * @param[in] dataPointRate  the data point rate of reverb
 * @param[in] roomScale      the room scale of reverb
 * @param[in] stereoDepth    the desired stereo depth of reverb
 * @return  length of filter delay line
 */
static
Real _reverbLineDelayLength (IN Boolean isCreation,
                             IN Boolean isCombFilter,
                             IN Natural index,
                             IN Real dataPointRate,
                             IN Real roomScale,
                             IN Real stereoDepth)
{
    const Real effectiveRoomScale =
        (isCreation ? _maximumRoomScale
         : (isCombFilter ? roomScale : 1.0));
    const Real factor =
        dataPointRate / _referenceDataPointRate * effectiveRoomScale;
    const Integer sign = (index % 2 == 0 ? 1 : -1);
    const Real offset = (isCreation ? _maximumStereoDepth
                         : stereoDepth * sign);
    const RealList& lengthList = (isCombFilter
                                  ? _combFilterLengthList
                                  : _allpassFilterLengthList);
    const Real length = lengthList[index];
    const Real adjustment = _stereoSpread;
    const Real result =
        Real::round(factor * (Real{length} + adjustment * offset));
    return result;
}

/*--------------------*/

/**
 * Returns initial filter delay line maximum length for the
 * delay line for given parameters.
 *
 * @param[in] isCombFilter   tells to adapt comb filter or allpass
 *                           filter
 * @param[in] index          the index of the filter (starting at zero)
 * @param[in] dataPointRate  the data point rate of reverb
 * @return maximum ring buffer length (for later setup)
 */
static
Real _initialReverbLineDelayLength (IN Boolean isCombFilter,
                                    IN Natural index,
                                    IN Real dataPointRate)
{
    /* calculate maximum ring buffer length with arbitrary values
       for <channel>, <roomScale> and <stereoDepth> */
    return _reverbLineDelayLength(true, isCombFilter,
                                  index, dataPointRate, 0.0, 0.0);
}

/*--------------------*/

/**
 * Returns adapted filter delay line length for the data point ring
 * buffer for given parameters <C>isCombFilter</C>,
 * <C>dataPointRate</C>, <C>roomScale</C> and <C>stereoDepth</C> to
 * their effective length.
 *
 * @param[in] isCombFilter   tells whether length for comb filter or
 *                           allpass filter should be calculated
 * @param[in] index          the index of the filter (starting at zero)
 * @param[in] dataPointRate  the data point rate of reverb
 * @param[in] roomScale      the room scale of reverb
 * @param[in] stereoDepth    the desired stereo depth of reverb
 * @return  adapted length of filter delay line
 */
static
Real _adjustedReverbLineDelayLength (IN Boolean isCombFilter,
                                     IN Natural index,
                                     IN Real dataPointRate,
                                     IN Real roomScale,
                                     IN Real stereoDepth)
{
    return _reverbLineDelayLength(false, isCombFilter, index,
                                  dataPointRate, roomScale, stereoDepth);
}

/*================*/
/* _AllpassFilter */
/*================*/

/*--------------------*/
/* con-/destruction   */
/*--------------------*/

_AllpassFilter::_AllpassFilter ()
{
}

/*--------------------*/
/* property access    */
/*--------------------*/

Real _AllpassFilter::delayLineLength () const
{
    return _delayLine.length();
}

/*--------------------*/
/* property change    */
/*--------------------*/

void _AllpassFilter::setDelayLineLength (IN Real length)
{
    _delayLine.setLength(length);
}

/*--------------------*/
/* complex change     */
/*--------------------*/

AudioDataPoint _AllpassFilter::apply (IN AudioDataPoint inputDataPoint)
{
    const AudioDataPoint outputDataPoint = _delayLine.first();
    const AudioDataPoint newDataPoint =
        inputDataPoint + outputDataPoint * _allpassFactor;
    _delayLine.advance(newDataPoint);
    return outputDataPoint - inputDataPoint;
}

/*=============*/
/* _CombFilter */
/*=============*/

/*--------------------*/
/* con-/destruction   */
/*--------------------*/

_CombFilter::_CombFilter ()
    : _storedDataPoint{0.0}
{
}

/*--------------------*/
/* property access    */
/*--------------------*/

Real _CombFilter::delayLineLength () const
{
    return _delayLine.length();
}

/*--------------------*/
/* property change    */
/*--------------------*/

void _CombFilter::setDelayLineLength (IN Real length)
{
    _delayLine.setLength(length);
}

/*--------------------*/
/* complex change     */
/*--------------------*/

AudioDataPoint _CombFilter::apply (IN AudioDataPoint inputDataPoint,
                                IN Real feedback,
                                IN Real hfDamping)
{
    const AudioDataPoint outputDataPoint = _delayLine.first();
    _storedDataPoint = (outputDataPoint
                     + (_storedDataPoint - outputDataPoint) * hfDamping);
    _delayLine.advance(inputDataPoint + _storedDataPoint * feedback);
    return outputDataPoint;
}

/*=============*/
/* _ReverbLine */
/*=============*/

/*--------------------*/
/* con-/destruction   */
/*--------------------*/

_ReverbLine::_ReverbLine ()
    : _allpassFilterList{},
      _combFilterList{}
{
    /* set allpass filters delay line lengths */
    for (Natural i = 0;  i < _lineAllpassFilterCount;  i++) {
        _AllpassFilter* allpassFilter = new _AllpassFilter();
        const Real length =
            _initialReverbLineDelayLength(false, i, _defaultDataPointRate);
        allpassFilter->setDelayLineLength(length);
        _allpassFilterList[i] = allpassFilter;
    }

    /* set comb filters delay line lengths */
    for (Natural i = 0;  i < _lineCombFilterCount;  i++) {
        _CombFilter* combFilter = new _CombFilter();
        const Real length =
            _initialReverbLineDelayLength(true, i, _defaultDataPointRate);
        combFilter->setDelayLineLength(length);
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
/* type conversion    */
/*--------------------*/

String _ReverbLine::toString () const {
    String st;

    st = "ReverbLine(";

    for (Natural i = 0;  i < _lineAllpassFilterCount;  i++) {
        const _AllpassFilter* filter = _allpassFilterList[i];
        const Real delayLineLength = filter->delayLineLength();
        st += ((i > 0 ? ", af(" : "af(")
               + TOSTRING(i) + ")="
               + TOSTRING(delayLineLength));
    }

    st += ", ";

    for (Natural i = 0;  i < _lineCombFilterCount;  i++) {
        const _CombFilter* filter = _combFilterList[i];
        const Real delayLineLength = filter->delayLineLength();
        st += ((i > 0 ? ", cf(" : "cf(")
               + TOSTRING(i) + ")="
               + TOSTRING(delayLineLength));
    }

    st += ")";
    return st;
}

/*--------------------*/
/* measurement        */
/*--------------------*/

Real _ReverbLine::maximumDelayPathLength () const
{
    Real result = 0.0;

    for (_CombFilter* filter : _combFilterList) {
        result = Real::maximum(result, filter->delayLineLength());
    }

    for (_AllpassFilter* filter : _allpassFilterList) {
        result += filter->delayLineLength();
    }

    return result;
}

/*--------------------*/
/* property change    */
/*--------------------*/

void _ReverbLine::adjustDelayLineLengths (IN Real dataPointRate,
                                           IN Real roomScale,
                                           IN Real stereoDepth)
{
    /* adjust allpass filter delay lines */
    for (Natural i = 0;  i < _lineAllpassFilterCount;  i++) {
        _AllpassFilter* allpassFilter = _allpassFilterList[i];
        const Real length =
            _adjustedReverbLineDelayLength(false, i, dataPointRate,
                                           roomScale, stereoDepth);
        allpassFilter->setDelayLineLength(length);
    }

    /* adjust comb filter delay lines */
    for (Natural i = 0;  i < _lineCombFilterCount;  i++) {
        _CombFilter* combFilter = _combFilterList[i];
        const Real length =
            _adjustedReverbLineDelayLength(true, i, dataPointRate,
                                           roomScale, stereoDepth);
        combFilter->setDelayLineLength(length);
    }
}

/*--------------------*/
/* complex change     */
/*--------------------*/

AudioDataPoint _ReverbLine::apply (IN AudioDataPoint inputDataPoint,
                                   IN Real feedback,
                                   IN Real hfDamping,
                                   IN Real gain)
{
    /* route input data point through the filters */
    AudioDataPoint outputDataPoint = 0.0;

    /* process comb filters in parallel */
    const auto applyCombFilterProc =
        [inputDataPoint, feedback, hfDamping] (AudioDataPoint currentValue,
                                               _CombFilter* filter) {
            return (currentValue
                    + filter->apply(inputDataPoint, feedback, hfDamping));
        };

    outputDataPoint =
        std::accumulate(_combFilterList.begin(), _combFilterList.end(),
                        Real::zero, applyCombFilterProc);

    /* process allpass filters in series */
    const auto applyAllpassFilterProc =
        [] (AudioDataPoint currentValue, _AllpassFilter* filter) {
            return filter->apply(currentValue);
        };

    outputDataPoint =
        std::accumulate(_allpassFilterList.begin(),
                        _allpassFilterList.end(),
                        outputDataPoint, applyAllpassFilterProc);

    outputDataPoint *= gain;
    return outputDataPoint;
}

/*================*/
/* _ReverbChannel */
/*================*/

/*--------------------*/
/* con-/destruction   */
/*--------------------*/

_ReverbChannel::_ReverbChannel ()
    : _inputDelayLine{0.0},
      _reverbLineCount{2},
      _reverbLineList{2}
{
    const auto reverbLineCreationProc =
        [] () {
            return new _ReverbLine();
        };

    std::generate(_reverbLineList.begin(), _reverbLineList.end(),
                  reverbLineCreationProc);
}

/*--------------------*/

_ReverbChannel::~_ReverbChannel ()
{
    for (_ReverbLinePtr reverbLine : _reverbLineList) {
        delete reverbLine;
    }
}

/*--------------------*/
/* type conversion    */
/*--------------------*/

String _ReverbChannel::toString () const
{
    String st = ("ReverbChannel("
                 "predelay = "
                 + TOSTRING(_inputDelayLine.length()));

    /* add information about reverb lines */
    for (const _ReverbLinePtr& reverbLine : _reverbLineList) {
        st += ", " + reverbLine->toString();
    }

    st += ")";
    return st;
}

/*--------------------*/
/* measurement        */
/*--------------------*/

Real _ReverbChannel::maximumDelayPathLength () const
{
    Real result = 0.0;

    for (_ReverbLinePtr reverbLine : _reverbLineList) {
        result = Real::maximum(result,
                                  reverbLine->maximumDelayPathLength());
    }

    return result;
}

/*--------------------*/
/* property change    */
/*--------------------*/

void _ReverbChannel::adjustDelayLineLengths (IN Real dataPointRate,
                                              IN Real predelay,
                                              IN Real roomScale,
                                              IN Real stereoDepth)
{
    const Real delayLineLength = Real::round(predelay * dataPointRate);
    _inputDelayLine.setLength(delayLineLength);

    /* adapt lengths of reverb lines; when stereo depth is zero,
       only a single reverb line is used per channel */
    _reverbLineCount = (stereoDepth == 0.0 ? 1 : 2);
    Real effectiveStereoDepth = 0.0;

    for (_ReverbLinePtr reverbLine : _reverbLineList) {
        reverbLine->adjustDelayLineLengths(dataPointRate, roomScale,
                                           effectiveStereoDepth);
        effectiveStereoDepth = stereoDepth;
    }
}

/*--------------------*/
/* complex change     */
/*--------------------*/

void _ReverbChannel::apply (IN AudioDataPoint inputDataPoint,
                            IN Real feedback,
                            IN Real hfDamping,
                            IN Real gain,
                            OUT _DataPointPair& wetDataPointPair)
{
    AudioDataPoint dataPoint = inputDataPoint;

    /* check and process predelay */
    if (_inputDelayLine.length() > 0.0) {
        const AudioDataPoint firstDataPoint = _inputDelayLine.first();
        _inputDelayLine.advance(dataPoint);
        dataPoint = firstDataPoint;
    }

    /* process all reverb lines for this channel and store their
       results in output data point list */
    for (Natural i = 0;  i < _reverbLineCount;  i++) {
        _ReverbLinePtr reverbLine = _reverbLineList[i];
        const AudioDataPoint outputDataPoint =
            reverbLine->apply(dataPoint, feedback, hfDamping, gain);
        wetDataPointPair[i] = outputDataPoint;
    }
}

/*--------------------*/

String _ReverbEffectParameterData::toString () const
{
    String prefix =
        STR::expand("isWetOnly = %1, feedback = %2,"
                    " hfDamping = %3%, predelay = %4s,"
                    " stereoDepth = %5%, wetGain = %6dB,"
                    " roomScale = %7%, channelCount = %8,"
                    " dataPointRate = %9Hz)",
                    TOSTRING(isWetOnly), TOSTRING(feedback),
                    TOSTRING(hfDamping), TOSTRING(predelay),
                    TOSTRING(stereoDepth), TOSTRING(wetGain),
                    TOSTRING(roomScale), TOSTRING(channelCount),
                    TOSTRING(dataPointRate));

    /* add information about reverb channels */
    String channelDataAsString;

    for (Natural channel = 0;   channel < channelCount; channel++) {
        const _ReverbChannel* reverbChannel = reverbChannelList[channel];
        channelDataAsString += ((channel == 0 ? "" : ", ")
                                + reverbChannel->toString());
    }

    String st = STR::expand("Reverb(%1, channels = (%2))",
                            prefix, channelDataAsString);
    return st;
}

/*============*/
/* _SoXReverb */
/*============*/

/*--------------------*/
/* con-/destruction   */
/*--------------------*/

_SoXReverb::_SoXReverb ()
{
    Logging_trace(">>");

    _effectParameterData = new _ReverbEffectParameterData();
    _ReverbEffectParameterData& effectParameterData =
        TOREFERENCE<_ReverbEffectParameterData>(_effectParameterData);

    effectParameterData.isWetOnly    = false;
    effectParameterData.feedback     = 0.0;
    effectParameterData.hfDamping    = 0.0;
    effectParameterData.stereoDepth  = 0.0;
    effectParameterData.wetGain      = 0.0;
    effectParameterData.predelay     = 0.0;
    effectParameterData.roomScale    = 10.0;
    effectParameterData.channelCount = 0;
    effectParameterData.reverbChannelList.clear();

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

_SoXReverb::~_SoXReverb ()
{
    Logging_trace(">>");
    /* get rid of all embedded reverb channels by resizing to zero
       length */
    resize(_defaultDataPointRate, 0);

    _ReverbEffectParameterData* effectParameterData =
        (_ReverbEffectParameterData*) _effectParameterData;
    delete effectParameterData;
    Logging_trace("<<");
}

/*--------------------*/
/* measurement        */
/*--------------------*/

Real _SoXReverb::tailLength () const
{
    Logging_trace(">>");

    const _ReverbEffectParameterData& effectParameterData =
        TOREFERENCE<_ReverbEffectParameterData>(_effectParameterData);

    Real maximumPathLength = 0.0;

    for (const _ReverbChannel* reverbChannel
             : effectParameterData.reverbChannelList) {
        maximumPathLength =
            Real::maximum(maximumPathLength,
                          reverbChannel->maximumDelayPathLength());
    }

    const Real result =
        Real{maximumPathLength} / effectParameterData.dataPointRate;
    
    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/
/* property change    */
/*--------------------*/

void _SoXReverb::setParameters (IN Boolean isWetOnly,
                                IN Percentage reverberance,
                                IN Percentage hfDamping,
                                IN Percentage roomScale,
                                IN Percentage stereoDepth,
                                IN Real predelay,
                                IN Real wetDbGain)
{
    Logging_trace7(">>: isWetOnly = %1, reverberance = %2,"
                   " hfDamping = %3, roomScale = %4,"
                   " stereoDepth = %5, predelay = %6,"
                   " wetDbGain = %7",
                   TOSTRING(isWetOnly), TOSTRING(reverberance),
                   TOSTRING(hfDamping), TOSTRING(roomScale),
                   TOSTRING(stereoDepth), TOSTRING(predelay),
                   TOSTRING(wetDbGain));
    
    _ReverbEffectParameterData& effectParameterData =
        TOREFERENCE<_ReverbEffectParameterData>(_effectParameterData);

    /* adjust parameter value ranges */
    const Percentage currentReverberance = reverberance.forceToPercentage();
    const Percentage currentHfDamping    = hfDamping.forceToPercentage();
    const Percentage currentRoomScale    = roomScale.forceToPercentage();
    const Percentage currentStereoDepth  = stereoDepth.forceToPercentage();
    const Real currentPredelay =
        predelay.forceToInterval(0.0, _maximumPredelay);
    const Real currentWetDbGain = wetDbGain.forceToInterval(-10.0, 10.0);

    /* calculate technical parameters */
    const Real minimumFeedback =  -1 / log(1 - 0.3);
    const Real maximumFeedback =
        (Real{100.0} / (Real{1 - 0.98}.log() * minimumFeedback + 1.0));

    /* set reverb parameters */
    effectParameterData.isWetOnly   = isWetOnly;
    effectParameterData.feedback    =
        Real::one - Real::exp((Real{currentReverberance} - maximumFeedback)
                            / (minimumFeedback * maximumFeedback));
    effectParameterData.hfDamping   = currentHfDamping / 100.0 * 0.3 + 0.2;
    effectParameterData.predelay    = currentPredelay;
    effectParameterData.stereoDepth = currentStereoDepth / 100.0;
    effectParameterData.roomScale   = currentRoomScale / 100.0 * 0.9 + 0.1;
    effectParameterData.wetGain     =
        SoXAudioHelper::dBToLinear(currentWetDbGain) * 0.015;

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

void _SoXReverb::resize (IN Real dataPointRate,
                         IN Natural channelCount)
{
    Logging_trace2(">>: dataPointRate = %1, channelCount = %2",
                   TOSTRING(dataPointRate), TOSTRING(channelCount));

    _ReverbEffectParameterData& effectParameterData =
        TOREFERENCE<_ReverbEffectParameterData>(_effectParameterData);

    effectParameterData.channelCount  = channelCount;
    effectParameterData.dataPointRate = dataPointRate;
    _ReverbChannelList& reverbChannelList =
        effectParameterData.reverbChannelList;

    const Natural oldChannelCount = reverbChannelList.size();

    /* get rid of extraneous channels */
    for (Natural channel = channelCount;
         channel < oldChannelCount;  channel++) {
        delete reverbChannelList[channel];
    }

    reverbChannelList.setLength(channelCount);

    /* create new channels */
    for (Natural channel = oldChannelCount;
         channel < channelCount;  channel++) {
        reverbChannelList[channel] = new _ReverbChannel();
    }

    for (_ReverbChannel* reverbChannel : reverbChannelList) {
        reverbChannel->adjustDelayLineLengths
                           (dataPointRate,
                            effectParameterData.predelay,
                            effectParameterData.roomScale,
                            effectParameterData.stereoDepth);
    }

    Logging_trace("<<");
}

/*--------------------*/
/* complex change     */
/*--------------------*/

void _SoXReverb::apply (IN AudioDataPointRingBuffer& inputDataPointList,
                        OUT AudioDataPointRingBuffer& outputDataPointList)
{
    Logging_trace(">>");

    _ReverbEffectParameterData& effectParameterData =
        TOREFERENCE<_ReverbEffectParameterData>(_effectParameterData);

    AudioDataPoint inputDataPoint;
    _ReverbChannelList& reverbChannelList =
        effectParameterData.reverbChannelList;
    const Natural channelCount = effectParameterData.channelCount;
    GenericTuple<_DataPointPair, _maxChannelCount> wetDataPointPairList;

    /* when stereo depth is non-zero for a reverb, each reverb channel
       produces a pair of output data points to be stored in a list of
       wet result pairs */
    for (Natural channel = 0;  channel < channelCount;  channel++) {
        /* collect all wet data point pairs */
        _DataPointPair& wetDataPointPair = wetDataPointPairList[channel];
        _ReverbChannel* reverbChannel = reverbChannelList[channel];
        inputDataPoint = inputDataPointList[channel];
        reverbChannel->apply(inputDataPoint,
                             effectParameterData.feedback,
                             effectParameterData.hfDamping,
                             effectParameterData.wetGain,
                             wetDataPointPair);
    }

    /* combine wet data point pairs with input data points */
    const Boolean hasMultipleLines =
        (effectParameterData.stereoDepth > 0.0 && channelCount == 2);

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        const _DataPointPair& wetDataPointPair =
            wetDataPointPairList[channel];
        AudioDataPoint outputDataPoint;

        if (!hasMultipleLines) {
            outputDataPoint = wetDataPointPair[0];
        } else {
            const _DataPointPair& otherWetDataPointPair =
                wetDataPointPairList[Natural{1} - channel];
            outputDataPoint = (wetDataPointPair[channel]
                               + otherWetDataPointPair[channel]) / 2.0;
        }

        inputDataPoint  = inputDataPointList[channel];
        outputDataPoint += (effectParameterData.isWetOnly ? 0.0
                            : inputDataPoint);
        outputDataPointList[channel] = outputDataPoint;
    }

    Logging_trace("<<");
}

/*--------------------*/
/* type conversion    */
/*--------------------*/

String _SoXReverb::toString () const
{
    const _ReverbEffectParameterData& effectParameterData =
        TOREFERENCE<_ReverbEffectParameterData>(_effectParameterData);
    return effectParameterData.toString();
}
