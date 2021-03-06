/**
 * @file
 * The <C>SoXIIRFilter</C> body implements an infinite impulse
 * response filter.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#include "SoXIIRFilter.h"

#include "Assertion.h"
#include "Logging.h"
#include "SoXAudioSample.h"
#include "StringUtil.h"

/*====================*/

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;

using SoXPlugins::CommonAudio::SoXAudioSample;
using SoXPlugins::CommonAudio::SoXIIRFilter;

/*====================*/

/*--------------------*/
/* internal routines  */
/*--------------------*/

/**
 * Normalizes coefficients in <C>data</C> for IIR filter of
 * <C>order</C> by dividing all by a0.
 *
 * @param data   IIR filter coefficient list
 * @param order  IIR filter order
 */
void _normalize (INOUT RealList& data, IN Natural order)
{
    Logging_trace1(">>: order = %1", TOSTRING(order));

    const Real referenceValue = data[order];

    if (referenceValue != 0.0) {
        for (Real& listElement : data) {
            listElement /= referenceValue;
        }
    }

    Logging_trace("<<");
}

/*--------------------*/
/* exported routines  */
/*--------------------*/

SoXIIRFilter::SoXIIRFilter (IN Natural order)
    : _data(),
      _order(order)
{
    Logging_trace(">>");
    _data.setLength(order * 2, 0.0);
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXIIRFilter::~SoXIIRFilter ()
{
    Logging_trace(">>");
    Logging_trace("<<");
}

/*--------------------*/

String SoXIIRFilter::toString() const
{
    String result = "SoXIIRFilter(";
    result += "_order = " + TOSTRING(_order);
    result += ", _data = " + _data.toString();
    result += ")";

    return result;
}

/*--------------------*/

void SoXIIRFilter::clear ()
{
    Logging_trace(">>");
    _data.fill(0.0);
    Logging_trace("<<");
}

/*--------------------*/

void SoXIIRFilter::set (IN Real b0)
{
    Logging_trace1(">>: %1", TOSTRING(b0));

    clear();
    _data[0]      = b0;
    _data[_order] = 1.0;

    Logging_trace("<<");
}

/*--------------------*/

void SoXIIRFilter::set (IN Real b0, IN Real b1, IN Real b2,
                        IN Real a0, IN Real a1, IN Real a2)
{
    Logging_trace6(">>: b = (%1, %2, %3), a = (%4, %5, %6)",
                   TOSTRING(b0), TOSTRING(b1), TOSTRING(b2),
                   TOSTRING(a0), TOSTRING(a1), TOSTRING(a2));

    Assertion_pre(_order == 3, "filter order must be 3");

    _data[0] = b0;
    _data[1] = b1;
    _data[2] = b2;
    _data[3] = a0;
    _data[4] = a1;
    _data[5] = a2;
    _normalize(_data, _order);

    Logging_trace("<<");
}

/*--------------------*/

void SoXIIRFilter::set (IN Real b0, IN Real b1, IN Real b2,
                        IN Real b3, IN Real b4,
                        IN Real a0, IN Real a1, IN Real a2,
                        IN Real a3, IN Real a4)
{
    Logging_trace10(">>: b = (%1, %2, %3, %4, %5), a = (%6, %7, %8, %9, %A)",
                    TOSTRING(b0), TOSTRING(b1), TOSTRING(b2),
                    TOSTRING(b3), TOSTRING(b4),
                    TOSTRING(a0), TOSTRING(a1), TOSTRING(a2),
                    TOSTRING(a3), TOSTRING(a4));

    Assertion_pre(_order == 5, "filter order must be 5");

    _data[0] = b0;
    _data[1] = b1;
    _data[2] = b2;
    _data[3] = b3;
    _data[4] = b4;
    _data[5] = a0;
    _data[6] = a1;
    _data[7] = a2;
    _data[8] = a3;
    _data[9] = a4;
    _normalize(_data, _order);

    Logging_trace("<<");
}

/*--------------------*/

void SoXIIRFilter::apply (IN SoXAudioSampleQueue& inputQueue,
                          INOUT SoXAudioSampleQueue& outputQueue) const
{
    // support vectorization by using arrays for the queues
    SoXAudioSample* inputQueueAsArray  = new SoXAudioSample(_order);
    SoXAudioSample* outputQueueAsArray = new SoXAudioSample(_order);
    inputQueue.toArray(inputQueueAsArray);
    outputQueue.toArray(outputQueueAsArray);
    
    SoXAudioSample outputValue = 0;

    for (Natural i = 1;  i < _order;  i++) {
        outputValue += _data[i] * inputQueueAsArray[(int) i];
    }

    for (Natural i = 1;  i < _order;  i++) {
        outputValue -= _data[i + _order] * outputQueueAsArray[(int) i];
    }

    outputQueue.setFirst(outputValue);
}
