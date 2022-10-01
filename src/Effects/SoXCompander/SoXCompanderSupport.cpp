/**
 * @file
 * The <C>SoXCompanderSupport</C> body implements several internal
 * compander classes for the compander algorithm and provides a single
 * <B>SoXMultibandCompander</B> class as its interface; the internal
 * routines and types implement transfer functions, the LR-filtering
 * and the processing.
 *
 * The original algorithms can be found in the <A
 * HREF="https://sourceforge.net/p/sox/code">SoX repository</A>
 * and the files
 *
 *   - "compandt.c" by Rob Sewell for the compander transfer function,
 *   - "compand.c" by Chris Bagwell and Nick Bailey for the compander
 *     effect,
 *   - "mcompand_xover.h" by Rob Sewell for the crossover function, and
 *   - "mcompand.c" by Daniel Pouzzner for the mcompander effect.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

/*====================*/

#include "SoXCompanderSupport.h"

#include <array>

#include "AudioSampleRingBuffer.h"
#include "IIRFilter.h"
#include "Logging.h"
#include "GenericSequence.h"
#include "RealList.h"
#include "StringUtil.h"

/*====================*/

using std::array;

using Audio::IIRFilter;
using BaseTypes::Containers::GenericSequence;
using BaseTypes::Containers::RealList;
using SoXPlugins::Effects::SoXCompander::SoXMultibandCompander;
using BaseModules::StringUtil;

/*============================================================*/

namespace SoXPlugins::Effects::SoXCompander {

    /** the maximum number of channels supported */
    const Natural _maximumChannelCount = 10;

    /*===============================*/
    /* Point in twodimensional space */
    /*===============================*/

    /**
     * A <C>_Point2D</C> object is a point in the two-dimensional
     * euklidian plane (with x- and y-coordinate).
     */
    struct _Point2D {

        /** the x-coordinate of the point */
        Real x;
        /** the y-coordinate of the point */
        Real y;

        /*--------------------*/

        /**
         * Makes new point
         */
        _Point2D ();

        /*--------------------*/

        /**
         * Makes new point from <C>newX</C> and <C>newY</C>
         *
         * @param[in] newX  new x-coordinate of point
         * @param[in] newY  new y-coordinate of point
         */
        _Point2D (IN Real newX, IN Real newY);

        /*--------------------*/

        /**
         * Return string representation of current point
         *
         * @return string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Adds <C>otherPoint</C> to current point
         *
         * @param[in] otherPoint  point to be added to current point
         */
        void add (IN _Point2D otherPoint);

        /*--------------------*/

        /**
         * Sets current point to (0, 0)
         */
        void clear ();

        /*--------------------*/

        /**
         * Returns the euklidian distance between current point
         * and <C>otherPoint</C>.
         *
         * @param[in] otherPoint  point with distance to current point
         * @return distance between points
         */
        Real distance (IN _Point2D otherPoint) const;

        /*--------------------*/

        /**
         * Scales current point by <C>factor</C>.
         *
         * @param[in] factor  factor to be multiplied scalarly into
         *                    point
         */
        void scale (IN Real factor);

        /*--------------------*/

        /**
         * Subtracts <C>otherPoint</C> from current point
         *
         * @param[in] otherPoint  point to be subtracted from current
         *                        point
         */
        void subtract (IN _Point2D otherPoint);

    };

    /*===========================*/
    /* Transfer Function Segment */
    /*===========================*/

    /**
     * A <C>_TfSegment</C> object is a segment between two points
     * within a compander transfer function (and represents either a
     * linear or a quadratic function).
     */
    struct _TfSegment {

        /**
         * Returns string representation of current segment
         *
         * @return string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Tells whether <C>x</C> lies in domain of current segment
         *
         * @param[in] x  x-coordinate to be checked for containment
         * @return information on x being in segment
         */
        bool domainContains (IN Real x) const;

        /*--------------------*/

        /**
         * Return the gradient of current segment
         *
         * @return segment gradient
         */
        Real gradient () const;

        /*--------------------*/

        /**
         * Returns the length of current segment
         *
         * @return segment length
         */
        Real length () const;

        /*--------------------*/

        /**
         * Adapts coefficients of current segment such that
         * <C>point</C> as well as start and end points are
         * covered by a quadratic function
         *
         * @param[in] point  point to be covered by quadratic function
         *                   of segment
         */
        void adaptCoefficients (IN _Point2D point);

        /*--------------------*/

        /**
         * Returns point as linear interpolation of current segment at
         * <C>position</C>.
         *
         * @param[in] position  x-coordinate of point
         * @return  linearly interpolated point on segment with
         *          given x-coordinate
         */
        _Point2D interpolate (IN Real position) const;

        /*--------------------*/
        /*--------------------*/

        /** tells whether current segment is a straight line */
        Boolean isStraightLine;

        /** the start point of the segment */
        _Point2D startPoint;

        /** the end point of the segment */
        _Point2D endPoint;

        /** coefficient a2 of a quadratic function */
        Real a2;

        /** coefficients a1 of a quadratic function */
        Real a1;

    };

    /*================================*/
    /* Transfer Function Segment List */
    /*================================*/

    /**
     * A <C>_TfSegmentList</C> is a list of transfer function segments.
     */
    using _TfSegmentList = GenericSequence<_TfSegment>;

    /*===================*/
    /* Transfer Function */
    /*===================*/

    /**
     * A <C>_TransferFunction</C> object represents the transfer
     * function of a compander consisting of several segments (either
     * linear or quadratic).
     */
    struct _TransferFunction {

        /**
         * Initializes the transfer function
         */
        _TransferFunction ();

        /*--------------------*/

        /**
         * Returns string representation of transfer function
         *
         * @return string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Adapts values of transfer function to a simple
         * compander function with a compression <C>ratio</C>,
         * <C>dBThreshold</C>, <C>dBKnee</C> and <C>dBGain</C>
         *
         * @param[in] dBKnee        the knee in decibels
         * @param[in] dBThreshold   the threshold in decibels
         * @param[in] ratio         the ratio (as a factor)
         * @param[in] dBGain        the gain in decibels
         */
        void adapt (IN Real dBKnee, IN Real dBThreshold,
                    IN Real ratio, IN Real dBGain);

        /*--------------------*/

        /**
         * Applies transfer function to <value>
         *
         * @param[in] value   value to be adapted by function
         * @return associated function result
         */
        Real apply (IN Real value) const;

        /*====================*/

        private:

            /** the list of segments in this transfer function */
            _TfSegmentList _segmentList;

            /** the minimum input value for this transfer function */
            Real _minimumLinearInValue;

            /** the minimum output value for this transfer function */
            Real _minimumLinearOutValue;

            /** the global gain value in decibels for this transfer
              * function */
            Real _dBGain;

            /** the global knee value in decibels for this transfer
              * function */
            Real _dBKnee;

            /*--------------------*/

            /** Adapts curve segments in current transfer function by the
              * dBKnee value */
            void _adaptCurvesInSegmentList ();

            /*--------------------*/

            /** Adapts values of current transfer function segments in
              * segment list by dBGain and makes values natural
              * logarithms */
            void _shiftScaleSegmentList ();

            /*--------------------*/

            /** Adapts start values and kinds of transfer function
              * segments in segment list */
            void _updateSegmentListEnds ();

            /*--------------------*/

            /** Adapts start values and kinds of SoX compander transfer
              * segments in segment list */
            void _updateSegmentListKinds ();

            /*--------------------*/

            /** Adapts values of transfer function segments */
            void _updateSegmentList ();

            /*--------------------*/

            /** initial offset of first segment relative to threshold
             * point */
            static const Real _leftDbOffset;

    };

    /*=============================*/
    /* Compander or Compander Band */
    /*=============================*/

    /**
     * A <C>_Compander</C> object is a simple compander for several
     * channels integrating the sample volumes and applying a transfer
     * function to them; as opposed to a <C>_MCompanderBand</C> there
     * is no crossover filtering done here, but a compander band uses
     * the basic functionality of a compander and splits its output
     * into a local and propagated result list.
     */
    struct _Compander {

        /**
         * Constructs new compander
         */
        _Compander ();

        /*--------------------*/

        /**
         * Return string representation of current compander
         *
         * @return string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Sets parameters of single line compander to <C>attack</C>,
         * <C>release</C>, <C>dBKnee</C>, <C>dBThreshold</C>,
         * <C>ratio</C> and <C>dBGain</C> based on <C>sampleRate</C>.
         *
         * @param[in] sampleRate    the new sample rate
         * @param[in] attack        the attack time for this compander
         * @param[in] release       the release time for this compander
         * @param[in] dBKnee        the knee in decibels for this
         *                          compander
         * @param[in] dBThreshold   the threshold setting in decibels
         *                          for this compander
         * @param[in] ratio         the compression ration for this
         *                          compander
         * @param[in] dBGain        the out gain in decibels for this
         *                          compander
         */
        void adapt (IN Real sampleRate,
                    IN Real attack,
                    IN Real release,
                    IN Real dBKnee,
                    IN Real dBThreshold,
                    IN Real ratio,
                    IN Real dBGain);

        /*--------------------*/

        /**
         * Applies compander to all channels based on samples from
         * <C>inputSampleList</C> and returns them in
         * <C>outputSampleList</C>; if <C>outputValuesAreSummed</C> is
         * set, the output samples are summed up into
         * <C>outputSampleList</C>.
         *
         * @param[in]  inputSampleList       the list of samples for all
         *                                   input channels
         * @param[out] outputSampleList      the list of samples for all
         *                                   output channels
         * @param[in] outputValuesAreSummed  tells whether old values
         *                                   are kept in output sample
         *                                   list
         */
        void apply (IN AudioSampleList& inputSampleList,
                    OUT AudioSampleList& outputSampleList,
                    IN bool outputValuesAreSummed);

        /*--------------------*/

        /**
         * Sets number of channels to <C>channelCount</C>.
         *
         * @param[in] channelCount  the new channel count for compander
         */
        void setLength (IN Natural channelCount);

        /*====================*/

        private:

            /** the transfer function to be applied */
            _TransferFunction _transferFunction;

            /** tells whether all channels are combined for compression */
            Boolean _channelsAreAggregated;

            /** list of attack times for all channels */
            RealList _attackTimeList;

            /** list of release times for all channels */
            RealList _releaseTimeList;

            /** list of volumes for all channels */
            RealList _volumeList;

            /*--------------------*/
            /*--------------------*/

            /**
             * Adapts envelope time <C>t</C> in seconds into a delta
             * value relative to <C>sampleRate</C>.
             *
             * @param[in] t           envelope time in seconds
             * @param[in] sampleRate  sample rate of effect
             * @return  delta value relative to sample rate
             */
            static Real _adaptEnvelopeTime (IN Real t,
                                            IN Real sampleRate);

            /*--------------------*/

            /**
             * Adapts current volume of compander for <C>channel</C>
             * within attack-release curve from <C>inputVolume</C> by
             * some integration.
             *
             * @param[in] channel      compander channel to be
             *                         integrated
             * @param[in] inputVolume  current input volume
             */
            void _integrateVolume (IN Natural channel,
                                   IN Real inputVolume);

            /*--------------------*/

            /**
             * Finds and returns maximum absolute sample among all
             * samples in <C>sampleList</C>.
             *
             * @param[in] sampleList  audio sample list to be scanned
             * @return  maximum absolute sample value
             */
            static AudioSample _maximumAbsoluteSample (
                IN AudioSampleList& sampleList);

    };

    /*=======================*/
    /* Linkwitz-Riley Filter */
    /*=======================*/

    /**
     * A <C>_LRFilter</C> object is a Linkwitz-Riley filter of order
     * 5.
     */
    struct _LRFilter : IIRFilter {

        /** the order of the filter */
        static const Natural order;

        /*--------------------*/

        /**
         * Constructs a 4th order Linkwitz-Riley filter as an IIR
         * filter
         */
        _LRFilter ();

        /*--------------------*/

        /**
         * Sets up Linkwitz-Riley filter by using plain coefficients
         * from <C>coefficientListA</C> and <C>coefficientListB</C>.
         *
         * @param[in] coefficientListA  first LR coefficient list
         * @param[in] coefficientListB  second LR coefficient list
         */
        void adapt (IN RealList& coefficientListA,
                    IN RealList& coefficientListB);

    };

    /*===============================================*/
    /* Crossover Filter (Linkwitz - Riley 4th order) */
    /*===============================================*/

    /**
     * A <C>_LRCrossoverFilter</C> object is a crossover filter
     * consisting of two Linkwitz-Riley filters with the same
     * frequency; one is a lowpass, another one is a highpass.
     */
    struct _LRCrossoverFilter {

        /**
         * Makes a Linkwitz-Riley 4th order crossover filter
         */
        _LRCrossoverFilter ();

        /*--------------------*/

        /**
         * Returns the string representation of current
         * Linkwitz-Riley crossover filter.
         *
         * @return  string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Initializes a Linkwitz-Riley 4th order crossover filter at
         * <C>frequency</C> consisting of a lowpass, highpass and allpass
         * into current filter using <C>sampleRate</C>.
         *
         * @param[in] frequency   the crossover frequency of the filter
         * @param[in] sampleRate  the sample rate in the filter
         */
        void adapt (IN Real frequency, IN Real sampleRate);

        /*--------------------*/

        /**
         * Applies current LR4 crossover filter to <C>inputBuffer</C>
         * and <C>outputBufferLow</C> and <C>outputBufferHigh</C>;
         * assumes that first entry in input buffer is current sample
         * and writes results sample into top of output buffers.
         *
         * @param[in]  inputBuffer       input sample buffer
         * @param[out] outputBufferLow   sample buffer for low filter output
         * @param[out] outputBufferHigh  sample buffer for high filter output
         */
        void apply (IN AudioSampleRingBuffer& inputBuffer,
                    OUT AudioSampleRingBuffer& outputBufferLow,
                    OUT AudioSampleRingBuffer& outputBufferHigh);

        /*--------------------*/

        /**
         * Sets crossover filter to identity lowpass and rejecting
         * highpass
         */
        void setToIdentity ();

        /*====================*/

        private:

            /** the lowpass filter for this crossover */
            _LRFilter _lowpassFilter;

            /*--------------------*/

            /** the highpass filter for this crossover */
            _LRFilter _highpassFilter;

    };

    /*=======================*/
    /* _CompanderStreamKind */
    /*=======================*/

    /**
     * The <C>_CompanderStreamKind</C> type is an enumeration for the
     * kind of audio sample streams relevant for a compander band:
     * input, lowOutput and highOutput as input for the next band
    */
    enum class _CompanderStreamKind : int {
        inputStream = 0, lowOutputStream = 1, highOutputStream = 2
    };

    /** the count of stream kinds */
    constexpr int _companderStreamKindCount = 3;

    /** a map from stream kind to string */
    String _streamKindAsString[] = { "input", "lowOutput", "highOutput" };

    /*========================*/
    /* _CompanderSampleBuffer */
    /*========================*/

    /**
     * A <C>_CompanderSampleBufferEntry</C> object contains pointers
     * to a buffer for input, low and high output samples; it is the
     * element type in a <B>_CompanderSampleBuffer</B>.
     */
    using _CompanderSampleBufferEntry =
        array<AudioSampleRingBuffer*, _companderStreamKindCount>;

    /**
     * A <C>_CompanderSampleBuffer</C> object contains pointers to the
     * buffers for input, low and high output samples for each channel
     * in a crossover band; the input buffer of the first band contain
     * the input samples, the following bands use the high output of
     * the previous crossover band
     */
    using _CompanderSampleBuffer =
        GenericSequence<_CompanderSampleBufferEntry>;

    /*=================*/
    /* _MCompanderBand */
    /*=================*/

    /**
     * A <C>_MCompanderBand</C> object is a single band in a multiband
     * compander consisting of a compander and sample buffers for the
     * crossover filter.
     *
     * For each channel the crossover calculation is done on an input
     * buffer resulting in low output and high output (for the next
     * band), companding is done on the input buffer only
     */
    struct _MCompanderBand {

        /**
         * Initializes the multiband compander band
         */
        _MCompanderBand ();

        /*--------------------*/

        /**
         * Returns the string representation of current compander band.
         *
         * @return  string representation
         */
        String toString () const;

        /*--------------------*/

        /**
         * Sets channel count for compander band to
         * <C>channelCount</C>.
         *
         * @param[in] channelCount  the new channel count for band
         */
        void setChannelCount (IN Natural channelCount);

        /*--------------------*/

        /**
         * Sets stream buffer for compander band for <C>channel</C> and
         * <C>stream</C> to <C>buffer</C>.
         *
         * @param[in] channel  channel of band to be changed
         * @param[in] stream   stream of band to be changed
         * @param[in] buffer    buffer to be assigned to this position
         */
        void setBuffer (IN Natural channel,
                       IN _CompanderStreamKind stream,
                       AudioSampleRingBuffer* buffer);

        /*--------------------*/

        /**
         * Sets crossover filter for this compander to <C>filter</C>.
         *
         * @param[in] filter  new crossover filter
         */
        void setFilter (_LRCrossoverFilter* filter);

        /*--------------------*/

        /**
         * Sets up compander band using standard compander parameters
         * <C>attack</C>, <C>release</C>, <C>dBKnee</C>,
         * <C>dBThreshold</C>, <C>ratio</C> and <C>dBGain</C>;
         * <C>sampleRate</C> is the current sample rate,
         * <C>topFrequency</C> defines the crossover frequency for
         * this band.
         *
         * @param[in] attack        the attack time for this band
         * @param[in] release       the release time for this band
         * @param[in] dBKnee        the knee in decibels for this band
         * @param[in] dBThreshold   the threshold setting in decibels
         *                          for this band
         * @param[in] ratio         the compression ration for this band
         * @param[in] dBGain        the out gain in decibels for this band
         * @param[in] sampleRate    the sample rate for this band
         * @param[in] topFrequency  the top crossover frequency for this
         *                          band
         */
        void adapt (IN Real attack,
                    IN Real release,
                    IN Real dBKnee,
                    IN Real dBThreshold,
                    IN Real ratio,
                    IN Real dBGain,
                    IN Real sampleRate,
                    IN Real topFrequency);

        /*--------------------*/

        /**
         * Applies band compander to internal input stream and updates
         * <C>outputSampleList</C> (by adding resulting values per
         * channel).
         *
         * @param[out] outputSampleList  the sample list to be written
         *                               to
         */
        void apply (OUT AudioSampleList& outputSampleList);

        /*--------------------*/

        /**
         * Processes samples from input for band compander; updates
         * associated sample buffers for low and high output of this
         * band
         */
        void calculateCrossover ();

        /*--------------------*/
        /*--------------------*/

        /** the maximum top frequency allowed for a band */
        static const Real maxTopFrequency;

        /*====================*/

        private:

            /** the number of channels in this compander band */
            Natural _channelCount;

            /** the compander associated with this compander band */
            _Compander _compander;

            /** the top frequency of this compander band */
            Real _topFrequency;

            /** the top separation filter for this compander (for a
             * multiband compander) */
            _LRCrossoverFilter _crossoverFilter;

            /** the sample list for the separation filter (for a
             * multiband compander) */
            _CompanderSampleBuffer _buffer;

            /** a temporary input sample list only used in the apply
             * method*/
            AudioSampleList _inputSampleList;
    };

    /*=====================*/
    /* _MCompanderBandList */
    /*=====================*/

    /**
     * A <C>_MCompanderBandList</C> is a list of compander bands.
     */
    using _MCompanderBandList = GenericSequence<_MCompanderBand>;

    /*============================================================*/
    /*============================================================*/

    _Point2D::_Point2D ()
        : x{0.0},
          y{0.0}
    {
    }

    /*--------------------*/

    _Point2D::_Point2D (IN Real newX, IN Real newY)
        : x{newX},
          y{newY}
    {
    }

    /*--------------------*/

    String _Point2D::toString () const {
        String st = ("_Point2D("
                     + TOSTRING(x)
                     + ", " + TOSTRING(y)
                     + ")");

        return st;
    }

    /*--------------------*/

    void _Point2D::add (IN _Point2D otherPoint) {
        x += otherPoint.x;
        y += otherPoint.y;
    }

    /*--------------------*/

    void _Point2D::clear () {
        x = 0.0;
        y = 0.0;
    }

    /*--------------------*/


    Real _Point2D::distance (IN _Point2D otherPoint) const {
        return Real::sqrt((x - otherPoint.x).sqr()
                           + (y - otherPoint.y).sqr());
    }

    /*--------------------*/

    void _Point2D::scale (IN Real factor) {
        x *= factor;
        y *= factor;
    }

    /*--------------------*/

    void _Point2D::subtract (IN _Point2D otherPoint) {
        x -= otherPoint.x;
        y -= otherPoint.y;
    }

    /*============================================================*/

    String _TfSegment::toString () const {
        String st = "_TfSegment(";
        st += "isLine = " + TOSTRING(isStraightLine);
        st += ", start = " + startPoint.toString();
        st += ", end = " + endPoint.toString();
        st += ", a2 = " + TOSTRING(a2);
        st += ", a1 = " + TOSTRING(a1);
        st += ")";

        return st;
    }

    /*--------------------*/

    bool _TfSegment::domainContains (IN Real x) const {
        return startPoint.x <= x && x <= endPoint.x;
    }

    /*--------------------*/

    Real _TfSegment::gradient () const {
        return ((endPoint.y - startPoint.y)
                / (endPoint.x - startPoint.x));
    }

    /*--------------------*/

    Real _TfSegment::length () const {
        return startPoint.distance(endPoint);
    }

    /*--------------------*/

    void _TfSegment::adaptCoefficients (IN _Point2D point) {
        if (length() == 0.0) {
            a1 = 0.0;
            a2 = 0.0;
        } else {
            const Real inA  = point.x - startPoint.x;
            const Real outA = point.y - startPoint.y;
            const Real inB  = endPoint.x - startPoint.x;
            const Real outB = endPoint.y - startPoint.y;
            a2 = (outB / inB - outA / inA) / (inB - inA);
            a1 = outA / inA - a2 * inA;
        }
    }

    /*--------------------*/

    _Point2D _TfSegment::interpolate (IN Real position) const {
        const Real segmentLength = length();
        Real relativePosition;

        if (segmentLength == 0.0) {
            relativePosition = 0.0;
        } else {
            relativePosition = position / segmentLength;
        }

        _Point2D result = endPoint;
        result.subtract(startPoint);
        result.scale(relativePosition);
        result.add(startPoint);

        return result;
    }

    /*============================================================*/

    const Real _TransferFunction::_leftDbOffset = 10.0;

    /*--------------------*/

    void _TransferFunction::_adaptCurvesInSegmentList () {
        const Real radius = _dBKnee * log(10) / 20.0;
        const Natural segmentCount = _segmentList.size();

        if (segmentCount >= 2) {
            _TfSegment* segment = nullptr;
            _TfSegment* nextSegment = &_segmentList[0];

            for (Natural i = 0;  i < segmentCount;  i++) {
                _TfSegment* previousSegment = segment;
                segment = nextSegment;
                nextSegment++;

                if (!segment->isStraightLine) {
                    const _Point2D originalNextSegmentStartPoint =
                        nextSegment->startPoint;
                    _Point2D& segmentStartPoint = segment->startPoint;
                    _Point2D& segmentEndPoint = segment->endPoint;

                    // adapt curve segment to the left
                    Real length = previousSegment->length();
                    Real position = Real::maximum(0.0, length - radius);
                    segmentStartPoint = previousSegment->interpolate(position);
                    previousSegment->endPoint = segmentStartPoint;

                    // adapt curve segment to the right
                    length = nextSegment->length();
                    position = Real::minimum(radius, length / 2.0);
                    segmentEndPoint = nextSegment->interpolate(position);
                    nextSegment->startPoint = segmentEndPoint;

                    // find some intermediate point
                    _Point2D intermediatePoint = segmentStartPoint;
                    intermediatePoint.add(segmentEndPoint);
                    intermediatePoint.add(originalNextSegmentStartPoint);
                    intermediatePoint.scale(1/3.0);

                    // set coefficients of curve
                    segment->adaptCoefficients(intermediatePoint);
                }
            }
        }
    }

    /*--------------------*/

    void _TransferFunction::_shiftScaleSegmentList () {
        Real factor = log(10) / 20;

        for (_TfSegment& segment : _segmentList) {
            segment.startPoint.y += _dBGain;
            segment.endPoint.y   += _dBGain;
            segment.startPoint.scale(factor);
            segment.endPoint.scale(factor);

            if (segment.isStraightLine) {
                segment.a2 = 0.0;
                segment.a1 = segment.gradient();
            }
        }
    }

    /*--------------------*/

    void _TransferFunction::_updateSegmentListEnds () {
        Natural segmentCount = _segmentList.size();

        for (Natural i = 0;  i < segmentCount - 1;  i++) {
            _TfSegment& segment = _segmentList[i];
            const _TfSegment& nextSegment = _segmentList[i+1];
            segment.endPoint = nextSegment.startPoint;
        }
    }

    /*--------------------*/

    void _TransferFunction::_updateSegmentListKinds () {
        Natural segmentCount = _segmentList.size();

        if (segmentCount >= 2) {
            bool isLastSegment = true;
            for (Natural i = segmentCount;  i-- > 0;) {
                _TfSegment& segment = _segmentList[i];
                const bool isStraightLine = ((i % 2) == 0);
                segment.isStraightLine = isStraightLine;

                if (!isStraightLine && !isLastSegment) {
                    const _TfSegment& nextSegment =
                        _segmentList[i + 1];
                    segment.startPoint = nextSegment.startPoint;
                }

                isLastSegment = false;
            }
        }
    }

    /*--------------------*/

    void _TransferFunction::_updateSegmentList () {
        // pass 1: fill all segments with kind and start values
        _updateSegmentListKinds();

        // pass 2: fill all segments with end values and gradient
        //         for the straight lines
        _updateSegmentListEnds();

        // pass 3: adapt transfer function by output gain and make
        //         values natural logarithms
        _shiftScaleSegmentList();

        // pass 4: adapt transfer function knees
        _adaptCurvesInSegmentList();
    }

    /*--------------------*/

    _TransferFunction::_TransferFunction ()
        : _segmentList{3},
          _minimumLinearInValue{1.0},
          _minimumLinearOutValue{1.0},
          _dBGain{0.0},
          _dBKnee{0.01}
    {
        Logging_trace(">>");
        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    String _TransferFunction::toString () const {
        String st = "TransferFct(";
        st += "minLin = " + TOSTRING(_minimumLinearInValue);
        st += ", minOut = " + TOSTRING(_minimumLinearOutValue);
        st += ", dBGain = "  + TOSTRING(_dBGain);
        st += ", dBKnee = " + TOSTRING(_dBKnee);

        st += ", segments = (";
        bool isFirst = true;

        for (const _TfSegment& segment : _segmentList) {
            st += (isFirst ? "" : ", ");
            st += segment.toString();
            isFirst = false;
        }

        st += "))";
        return st;
    }

    /*--------------------*/

    void _TransferFunction::adapt (IN Real dBKnee,
                                   IN Real cDBThreshold,
                                   IN Real cRatio,
                                   IN Real dBGain) {
        Logging_trace4(">>: dBKnee = %1, dBThreshold = %2,"
                       " ratio = %3, dBGain = %4",
                       TOSTRING(dBKnee), TOSTRING(cDBThreshold),
                       TOSTRING(cRatio), TOSTRING(dBGain));

        const Real ratio = Real::maximum(1.0, cRatio);
        const Real dBThreshold = Real::minimum(0.0, cDBThreshold);
        _dBKnee = Real::maximum(0.0, dBKnee);
        _dBGain = dBGain;

        // set data for the straight segments
        _Point2D& segmentStart = _segmentList[0].startPoint;
        segmentStart = _Point2D(dBThreshold - _leftDbOffset, 0.0);

        _TfSegment& segment = _segmentList[2];
        segment.startPoint = _Point2D(dBThreshold, 0.0);
        segment.endPoint   = _Point2D(0.0, (ratio - 1.0) * dBThreshold / ratio);

        _updateSegmentList();

        const _Point2D& firstSegmentStart = _segmentList[1].startPoint;
        _minimumLinearInValue  = firstSegmentStart.x.exp();
        _minimumLinearOutValue = firstSegmentStart.y.exp();

        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    Real _TransferFunction::apply (IN Real cValue) const {
        Real result;

        if (cValue <= _minimumLinearInValue) {
            result = _minimumLinearOutValue;
        } else {
            const Real value = Real::minimum(cValue, 1.0);
            Real lnValue = value.log();
            result = value;

            for (const _TfSegment& segment : _segmentList) {
                if (segment.domainContains(lnValue)) {
                    lnValue -= segment.startPoint.x;
                    const Real a1 = segment.a1;
                    const Real a2 = segment.a2;
                    const Real y  = segment.startPoint.y;
                    const Real lnResult = y + lnValue * (a2 * lnValue + a1);
                    result = lnResult.exp();
                    break;
                }
            }
        }

        return result;
    }

    /*============================================================*/

    Real _Compander::_adaptEnvelopeTime (IN Real t,
                                         IN Real sampleRate) {
        const Real one{1.0};
        return Real::minimum(one,
                             one - (-one / (sampleRate * t)).exp());
    }

    /*--------------------*/

    _Compander::_Compander ()
        : _transferFunction{},
          _channelsAreAggregated{true},
          _attackTimeList{},
          _releaseTimeList{},
          _volumeList{}
    {
        Logging_trace(">>");
        _volumeList.setLength(_maximumChannelCount);
        _attackTimeList.setLength(_maximumChannelCount);
        _releaseTimeList.setLength(_maximumChannelCount);
        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    String _Compander::toString () const {
        String st = "_Compander(";
        st += "transferFunction = " + _transferFunction.toString();
        st += (", _channelsAreAggregated = "
               + TOSTRING(_channelsAreAggregated));
        st += ", _attackTimeList = " + _attackTimeList.toString();
        st += ", _releaseTimeList = " + _releaseTimeList.toString();
        st += ", _volumeList = " + _volumeList.toString();
        st += ")";

        return st;
    }

    /*--------------------*/

    void _Compander::adapt (IN Real sampleRate,
                            IN Real attack,
                            IN Real release,
                            IN Real dBKnee,
                            IN Real dBThreshold,
                            IN Real ratio,
                            IN Real dBGain) {
        Logging_trace7(">>: sampleRate = %1, attack = %2, release = %3,"
                       " dBKnee = %4, dBThreshold = %5, ratio = %6,"
                       " dBGain = %7",
                       TOSTRING(sampleRate), TOSTRING(attack),
                       TOSTRING(release), TOSTRING(dBKnee),
                       TOSTRING(dBThreshold), TOSTRING(ratio),
                       TOSTRING(dBGain));

        Real time;
        _transferFunction.adapt(dBKnee, dBThreshold, ratio, dBGain);
        _channelsAreAggregated = true;
        _volumeList.fill(1.0);
        time = _adaptEnvelopeTime(attack, sampleRate);
        _attackTimeList.fill(time);
        time = _adaptEnvelopeTime(release, sampleRate);
        _releaseTimeList.fill(time);

        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    void _Compander::apply (IN AudioSampleList& inputSampleList,
                            OUT AudioSampleList& outputSampleList,
                            IN bool outputValuesAreSummed) {
        Natural channel;
        const Natural channelCount = inputSampleList.size();

        if (_channelsAreAggregated) {
            // use settings of first channel to represent all channels
            channel = 0;
            const AudioSample maximumSample =
                _maximumAbsoluteSample(inputSampleList);
            _integrateVolume(channel, maximumSample);
        } else {
            for (channel = 0;  channel < channelCount;  channel++) {
                const AudioSample inputSample = inputSampleList[channel];
                _integrateVolume(channel, inputSample);
            }
        }

        for (channel = 0;  channel < channelCount;  channel++) {
            const AudioSample inputSample = inputSampleList[channel];
            const Real currentVolume = _volumeList[channel];
            const Real amplificationFactor =
                _transferFunction.apply(currentVolume);
            AudioSample outputSample = inputSample * amplificationFactor;

            if (outputValuesAreSummed) {
                outputSample += outputSampleList[channel];
            }

            outputSampleList[channel] = outputSample;
        }
    }

    /*--------------------*/

    void _Compander::_integrateVolume (IN Natural channel,
                                       IN Real inputVolume) {
        Real volume = _volumeList[channel];
        const Real delta = inputVolume - volume;
        const RealList& timeList = (delta > 0.0
                                    ? _attackTimeList
                                    : _releaseTimeList);
        const Real increment = timeList[channel];
        volume += (delta * increment);

        if (_channelsAreAggregated) {
            // volume represents all channels
            _volumeList.fill(volume);
        } else {
            _volumeList[channel] = volume;
        }
    }

    /*--------------------*/

    AudioSample
    _Compander::_maximumAbsoluteSample (IN AudioSampleList& sampleList) {
        AudioSample result = 0.0;

        for (const AudioSample& value : sampleList) {
            const AudioSample absValue = value.abs();

            if (absValue > result) {
                result = absValue;
            }
        }

        return result;
    }

    /*--------------------*/

    void _Compander::setLength (IN Natural channelCount) {
        Logging_trace1(">>: %1", TOSTRING(channelCount));

        _volumeList.setLength(channelCount);
        _attackTimeList.setLength(channelCount);
        _releaseTimeList.setLength(channelCount);

        Logging_trace1("<<: %1", toString());
    }

    /*============================================================*/

    const Natural _LRFilter::order{5};

    /*--------------------*/

    _LRFilter::_LRFilter ()
            : IIRFilter(order)
    {
        Logging_trace1(">>: %1", TOSTRING(order));
        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    void _LRFilter::adapt (IN RealList& coefficientListA,
                           IN RealList& coefficientListB) {
        Logging_trace2(">>: listA = %1, listB = %2",
                       coefficientListA.toString(),
                       coefficientListB.toString());

        Natural i = 0;

        for (Natural j = 0;  j < 2;  j++) {
            const RealList& coefficientList =
                (j == 0
                 ? coefficientListA
                 : coefficientListB);
            _data[i + 0] = coefficientList[0].sqr();
            _data[i + 1] = Real{2.0} * coefficientList[0] * coefficientList[1];
            _data[i + 2] = (Real{2.0} * coefficientList[0] * coefficientList[2]
                            + coefficientList[1].sqr());
            _data[i + 3] = Real{2.0} * coefficientList[1] * coefficientList[2];
            _data[i + 4] = coefficientList[2].sqr();
            i += order;
        }

        Logging_trace1("<<: %1", toString());
    }

    /*============================================================*/

    _LRCrossoverFilter::_LRCrossoverFilter ()
        : _lowpassFilter{},
          _highpassFilter{}
    {
        Logging_trace(">>");
        _lowpassFilter.set(1.0);
        _highpassFilter.set(0.0);
        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    String _LRCrossoverFilter::toString () const
    {
        String st = ("_LRCrossoverFilter("
                     "lowpass = " + _lowpassFilter.toString()
                     + ", highpass = " + _highpassFilter.toString()
                     + ")");
        return st;
    }

    /*--------------------*/

    void _LRCrossoverFilter::adapt (IN Real frequency,
                                    IN Real sampleRate) {
        Logging_trace2(">>: frequency = %1, sampleRate = %2",
                       TOSTRING(frequency), TOSTRING(sampleRate));

        if (frequency >= sampleRate / 2.0) {
            _lowpassFilter.set(1.0);
            _highpassFilter.set(0.0);
        } else {
            const Real w0 = Real::twoPi * frequency / sampleRate;
            const Real filterQuality = sqrt(0.5);
            const Real alpha = w0.sin() / (Real{2.0} * filterQuality);
            const Real cosW0 = w0.cos();
            RealList coefficientListA{Natural{3}};
            RealList coefficientListB{Natural{3}};
            RealList coefficientListC{Natural{3}};

            // biquad lowpass filter numerator
            coefficientListA[0] = (Real{1.0} - cosW0) / Real{2.0};
            coefficientListA[1] = Real{1.0} - cosW0;
            coefficientListA[2] = coefficientListA[0];

            // biquad highpass filter numerator
            coefficientListB[0] = (Real{1.0} + cosW0) / Real{2.0};
            coefficientListB[1] = -Real{1.0} - cosW0;
            coefficientListB[2] = coefficientListB[0];

            // biquad LP/HP filter denominator
            coefficientListC[0] = Real{1.0} + alpha;
            coefficientListC[1] = -Real{2.0} *cosW0;
            coefficientListC[2] = Real{1.0} - alpha;

            // normalize coefficients
            Real referenceValue = Real{1.0} / coefficientListC[0];
            coefficientListA.multiply(referenceValue);
            coefficientListB.multiply(referenceValue);
            coefficientListC.multiply(referenceValue);

            _lowpassFilter.adapt(coefficientListA,
                                 coefficientListC);
            _highpassFilter.adapt(coefficientListB,
                                  coefficientListC);
        }

        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    void
    _LRCrossoverFilter::apply (IN AudioSampleRingBuffer& inputBuffer,
                               OUT AudioSampleRingBuffer& outputBufferLow,
                               OUT AudioSampleRingBuffer& outputBufferHigh)
    {
        _lowpassFilter.apply(inputBuffer, outputBufferLow);
        _highpassFilter.apply(inputBuffer, outputBufferHigh);
    }

    /*--------------------*/

    void _LRCrossoverFilter::setToIdentity () {
        Logging_trace(">>");
        _lowpassFilter.set(1.0);
        _highpassFilter.set(0.0);
        Logging_trace1("<<: %1", toString());
    }

    /*============================================================*/

    /**
     * Returns string representation of <C>buffer</C>.
     */
    static String _sampleBufferToString (IN _CompanderSampleBuffer& buffer)
    {
        const Natural channelCount = buffer.size();
        String st = "_CompanderSampleBuffer(";

        for (Natural channel = 0;  channel < channelCount;  channel++) {
            st += (channel == 0 ? "" : ", ");

            for (int stream = 0;  stream < _companderStreamKindCount;
                 stream++) {
                const AudioSampleRingBuffer* ringBuffer =
                    buffer[channel][stream];
                const String bufferAsString =
                    (ringBuffer == NULL ? "NULL" : ringBuffer->toString());
                st += ("channel_" + TOSTRING(channel) +
                       + "_" + _streamKindAsString[stream] + " = (");
                st += bufferAsString;
                st += ")";
            }
        }

        st += ")";

        return st;
    }

    /*============================================================*/

    const Real _MCompanderBand::maxTopFrequency = 1E9;

    /*--------------------*/

    _MCompanderBand::_MCompanderBand ()
        : _channelCount{0},
          _compander{},
          _topFrequency{maxTopFrequency},
          _crossoverFilter{},
          _buffer{},
          _inputSampleList{}
    {
        Logging_trace(">>");
        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    String _MCompanderBand::toString () const
    {
        String st = "_MCompanderBand(";

        st += "_channelCount = " + TOSTRING(_channelCount);
        st += ", _topFrequency = " + TOSTRING(_topFrequency) + "Hz";
        st += ", _crossoverFilter = " + _crossoverFilter.toString();
        st += ", _compander = " + _compander.toString();
        st += ", _buffer = " + _sampleBufferToString(_buffer);
        st += ", _inputSampleList = " + _inputSampleList.toString();
        st += ")";

        return st;
    }

    /*--------------------*/

    void _MCompanderBand::adapt (IN Real sampleRate,
                                 IN Real attack,
                                 IN Real release,
                                 IN Real dBKnee,
                                 IN Real dBThreshold,
                                 IN Real ratio,
                                 IN Real dBGain,
                                 IN Real topFrequency)
    {
        Logging_trace8(">>: sampleRate = %1, attack = %2, release = %3,"
                       " dBKnee = %4, dBThreshold = %5, ratio = %6,"
                       " dBGain = %7, topFrequency = %8",
                       TOSTRING(sampleRate), TOSTRING(attack),
                       TOSTRING(release), TOSTRING(dBKnee),
                       TOSTRING(dBThreshold), TOSTRING(ratio),
                       TOSTRING(dBGain), TOSTRING(topFrequency));

        _compander.adapt(sampleRate, attack, release,
                         dBKnee, dBThreshold, ratio, dBGain);
        _crossoverFilter.adapt(topFrequency, sampleRate);
        _topFrequency = topFrequency;

        Logging_trace1("<<: %1", toString());
    }

    /*--------------------*/

    void _MCompanderBand::setChannelCount (IN Natural channelCount)
    {
        Logging_trace1(">>: %1", TOSTRING(channelCount));
        _channelCount = channelCount;
        _buffer.setLength(channelCount);
        _compander.setLength(channelCount);
        _inputSampleList.setLength(channelCount);
        Logging_trace("<<");
    }

    /*--------------------*/

    void _MCompanderBand::setBuffer (IN Natural channel,
                                    IN _CompanderStreamKind stream,
                                    AudioSampleRingBuffer* buffer)
    {
        _buffer[channel][(int) stream] = buffer;
    }

    /*--------------------*/

    void _MCompanderBand::apply (OUT AudioSampleList& outputSampleList)
    {
        for (Natural channel = 0;  channel < _channelCount;  channel++) {
            const _CompanderSampleBufferEntry& bufferEntry =
                _buffer[channel];
            const AudioSampleRingBuffer* outputBufferLow =
                bufferEntry[(int) _CompanderStreamKind::lowOutputStream];
            AudioSample inputSample = outputBufferLow->first();
            _inputSampleList[channel] = inputSample;
        }

        _compander.apply(_inputSampleList, outputSampleList, true);
    }

    /*--------------------*/

    void _MCompanderBand::calculateCrossover ()
    {
        for (_CompanderSampleBufferEntry& bufferEntry : _buffer) {
            const AudioSampleRingBuffer* inputBuffer =
                bufferEntry[(int) _CompanderStreamKind::inputStream];
            AudioSampleRingBuffer* outputBufferLow =
                bufferEntry[(int) _CompanderStreamKind::lowOutputStream];
            AudioSampleRingBuffer* outputBufferHigh =
                bufferEntry[(int) _CompanderStreamKind::highOutputStream];

            _crossoverFilter.apply(*inputBuffer,
                                   *outputBufferLow,
                                   *outputBufferHigh);
        }
    }

    /*============================================================*/

    static String _mCompanderBandListToString (IN _MCompanderBandList& list)
    {
        const Natural bandCount = list.size();
        String st = "_MCompanderBandList(";

        for (Natural bandIndex = 0;  bandIndex < bandCount;  bandIndex++) {
            st += (bandIndex == 0 ? "" : ", ");
            st += "band_" + TOSTRING(bandIndex) + " = ";
            st += list[bandIndex].toString();
        }

        st += ")";
        return st;

    }

}

/*============================================================*/

SoXMultibandCompander::SoXMultibandCompander ()
    : _allocatedBandCount{0},
      _bandCount{0},
      _channelCount{0},
      _sampleRingBufferVector{}
{
    Logging_trace(">>");
    _companderBandList = new _MCompanderBandList(_allocatedBandCount);
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXMultibandCompander::~SoXMultibandCompander () {
    Logging_trace(">>");
    _MCompanderBandList* list = (_MCompanderBandList*) _companderBandList;
    delete list;
    Logging_trace("<<");
}

/*--------------------*/

String SoXMultibandCompander::toString () const {
    const _MCompanderBandList* companderBandList =
        (_MCompanderBandList*) _companderBandList;
    String st = "SoXMultibandCompander(";
    st += "_allocatedBandCount = " + TOSTRING(_allocatedBandCount);
    st += ", _effectiveBandCount = " + TOSTRING(_bandCount);
    st += ", _channelCount = " + TOSTRING(_channelCount);
    st += (", _companderBandList = "
           + _mCompanderBandListToString(*companderBandList));
    st += ")";

    return st;
}

/*--------------------*/

void
SoXMultibandCompander::setCompanderBandData (IN Natural bandIndex,
                                             IN Real sampleRate,
                                             IN Real attack,
                                             IN Real release,
                                             IN Real dBKnee,
                                             IN Real dBThreshold,
                                             IN Real ratio,
                                             IN Real dBGain,
                                             IN Real topFrequency)
{
    Logging_trace9(">>: bandIndex = %1, sampleRate = %2, attack = %3,"
                   " release = %4, dBKnee = %5, dBThreshold = %6,"
                   " ratio = %7, dBGain = %8, topFrequency = %9",
                   TOSTRING(bandIndex), TOSTRING(sampleRate),
                   TOSTRING(attack), TOSTRING(release),
                   TOSTRING(dBKnee), TOSTRING(dBThreshold),
                   TOSTRING(ratio), TOSTRING(dBGain),
                   TOSTRING(topFrequency));

    _MCompanderBandList* companderBandList =
        (_MCompanderBandList*) _companderBandList;
    _MCompanderBand& companderBand =
        companderBandList->at(bandIndex);
    companderBand.adapt(sampleRate,
                        attack, release, dBKnee, dBThreshold,
                        ratio, dBGain, topFrequency);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

void SoXMultibandCompander::resize (IN Natural bandCount,
                                    IN Natural channelCount) {
    Logging_trace2(">>: bandCount = %1, channelCount = %2",
                   TOSTRING(bandCount), TOSTRING(channelCount));

    _allocatedBandCount = bandCount;
    _bandCount          = Natural::minimum(bandCount, _bandCount);
    _channelCount       = channelCount;

    // setup all compander bands
    _MCompanderBandList* companderBandList =
        (_MCompanderBandList*) _companderBandList;
    companderBandList->setLength(_allocatedBandCount);

    for (_MCompanderBand& companderBand : *companderBandList) {
        companderBand.setChannelCount(channelCount);
    }

    const Natural bufferCountPerChannel = _allocatedBandCount * 2 + 1;
    _sampleRingBufferVector.resize(_channelCount, bufferCountPerChannel);

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        // connect the compander buffers to the sample buffer matrix
        Natural i = 0;

        for (_MCompanderBand& companderBand : *companderBandList) {
            companderBand.setBuffer(channel,
                                    _CompanderStreamKind::inputStream,
                                    &_sampleRingBufferVector.at(channel, i));
            companderBand.setBuffer(channel,
                                    _CompanderStreamKind::lowOutputStream,
                                    &_sampleRingBufferVector.at(channel,
                                                                i + 1));
            companderBand.setBuffer(channel,
                                    _CompanderStreamKind::highOutputStream,
                                    &_sampleRingBufferVector.at(channel,
                                                                i + 2));
            // take care of single overlapping buffer between the bands
            i += 2;
        }

        for (Natural j = 0;  j < bufferCountPerChannel;  j++) {
            AudioSampleRingBuffer& sampleBuffer =
                _sampleRingBufferVector.at(channel, j);
            sampleBuffer.setLength(_LRFilter::order);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXMultibandCompander::setEffectiveSize (IN Natural bandCount)
{
    Logging_trace1(">>: bandCount = %1", TOSTRING(bandCount));
    _bandCount =
        Natural::maximum(1, Natural::minimum(_allocatedBandCount, bandCount));
    Logging_trace1("<<: new band count = %1", TOSTRING(_bandCount));
}

/*--------------------*/

void
SoXMultibandCompander::apply (IN AudioSampleList& inputSampleList,
                              OUT AudioSampleList& outputSampleList)
{
    outputSampleList.setToZero();
    const Natural bufferCountPerChannel = _bandCount * 2 + 1;

    for (Natural channel = 0;  channel < _channelCount;  channel++) {
        for (Natural j = 0;  j < bufferCountPerChannel;  j++) {
            AudioSampleRingBuffer& sampleBuffer =
                _sampleRingBufferVector.at(channel, j);
            sampleBuffer.shiftRight(0.0);
        }

        // setup sample buffer for processing
        AudioSampleRingBuffer& inputBuffer =
            _sampleRingBufferVector.at(channel, 0);
        AudioSample inputSample = inputSampleList[channel];
        inputBuffer.setFirst(inputSample);
    }

    _MCompanderBandList* companderBandList =
        (_MCompanderBandList*) _companderBandList;

    // calculate crossover filtering from input buffers into
    // low and high outputs
    for (Natural bandIndex = 0;  bandIndex < _bandCount;  bandIndex++) {
        _MCompanderBand& companderBand =
            companderBandList->at(bandIndex);
        companderBand.calculateCrossover();
    }

    // do compression across all bands
    for (Natural bandIndex = 0;  bandIndex < _bandCount;  bandIndex++) {
        _MCompanderBand& companderBand =
            companderBandList->at(bandIndex);
        companderBand.apply(outputSampleList);
    }

    // write all output channel samples to buffer
    for (Natural channel = 0;  channel < _channelCount;  channel++) {
        const AudioSample outputSample = outputSampleList[channel];
        AudioSampleRingBuffer& outputBuffer =
            _sampleRingBufferVector.at(channel, bufferCountPerChannel - 1);
        outputBuffer.setFirst(outputSample);
    }
}
