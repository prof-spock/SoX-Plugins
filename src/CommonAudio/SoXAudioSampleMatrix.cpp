/**
 * @file
 * The <C>SoXAudioSampleMatrix</C> body implements a list of ring
 * buffers for audio samples.
 *
 * @author Dr. Thomas Tensi
 */

/*====================*/

#include "SoXAudioSampleMatrix.h"

#include "Assertion.h"
#include "Logging.h"
#include "StringUtil.h"

/*====================*/

using SoXPlugins::CommonAudio::SoXAudioSampleMatrix;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
using StringUtil::listToStringImplicit;

/*====================*/

/*--------------------*/
/* internal routines  */
/*--------------------*/

/**
 * Returns the real index into the list for <channel> and <position>
 * also taking flag <hasTwoQueuesPerChannel> into account
 */
static
Natural _effectiveIndex (IN bool hasTwoQueuesPerChannel,
                         IN Natural channelIndex,
                         IN Natural position)
{
    return (Natural{(size_t) (hasTwoQueuesPerChannel ? 2 : 1)}
            * channelIndex + position);
}

/*--------------------*/
/* exported routines  */
/*--------------------*/

SoXAudioSampleMatrix::SoXAudioSampleMatrix
                          (IN Natural channelCount,
                           IN bool hasTwoQueuesPerChannel,
                           IN Natural sampleQueueLength)
    : _data{_effectiveIndex(hasTwoQueuesPerChannel, channelCount, 0)},
      _hasTwoQueuesPerChannel(hasTwoQueuesPerChannel)
{
    Logging_trace(">>");
    setQueueLength(sampleQueueLength);
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

String SoXAudioSampleMatrix::toString() const
{
    const String queueListToString =
        listToStringImplicit<std::vector<SoXAudioSampleQueue>,
                             SoXAudioSampleQueue>("SoXAudioSampleMatrix",
                                                  _data);
    String result = "SoXAudioSampleMatrix(";
    result += ("_hasTwoQueuesPerChannel = "
               + TOSTRING(_hasTwoQueuesPerChannel));
    result += ", _data = " + queueListToString;
    result += ")";

    return result;
}

/*--------------------*/

void SoXAudioSampleMatrix::clear ()
{
    Logging_trace(">>");
    _data.clear();
    Logging_trace("<<");
}

/*--------------------*/

Natural SoXAudioSampleMatrix::queueCount () const
{
    Logging_trace(">>");
    Natural result = _data.length();
    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/

void SoXAudioSampleMatrix::setQueueCount (IN Natural count)
{
    Logging_trace1(">>: %1", TOSTRING(count));
    _data.setLength(count);

    for (SoXAudioSampleQueue& queue : _data) {
        queue.setLength(0);
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioSampleMatrix::setQueueLength (IN Natural length)
{
    Logging_trace1(">>: %1", TOSTRING(length));

    for (SoXAudioSampleQueue& queue : _data) {
        queue.setLength(length);
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioSampleMatrix::setToZero ()
{
    Logging_trace(">>");

    for (SoXAudioSampleQueue& queue : _data) {
        queue.setToZero();
    }

    Logging_trace("<<");
}

/*--------------------*/

SoXAudioSampleQueue& SoXAudioSampleMatrix::at
                         (IN Natural channelIndex,
                          IN Natural position)
{
    return _data[_effectiveIndex(_hasTwoQueuesPerChannel,
                                 channelIndex, position)];
}


/*--------------------*/

SoXAudioSampleQueue& SoXAudioSampleMatrix::at
                         (IN Natural channelIndex)
{
    Assertion_pre(!_hasTwoQueuesPerChannel,
                  "a simple matrix may only have one queue per channel");
    return at(channelIndex, 0);
}

/*--------------------*/

SoXAudioSampleQueue& SoXAudioSampleMatrix::operator []
                         (IN Natural channelIndex)
{
    return at(channelIndex);
}

/*--------------------*/

void SoXAudioSampleMatrix::append (IN SoXAudioSampleQueue& sampleQueue)
{
    Logging_trace(">>");
    _data.append(sampleQueue);
    Logging_trace("<<");
}

/*--------------------*/

SoXAudioSampleQueue& SoXAudioSampleMatrix::last ()
{
    return _data.back();
}
