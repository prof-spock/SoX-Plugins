/**
 * @file
 * The <C>SoXAudioEffect</C> body implements the root class for
 * all SoX audio effects.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#include "SoXAudioEffect.h"

#include "Logging.h"
#include "StringUtil.h"

/*====================*/

using SoXPlugins::CommonAudio::SoXAudioParameterMap;
using SoXPlugins::CommonAudio::SoXAudioEffect;
using SoXPlugins::CommonAudio::SoXAudioValueChangeKind;

/*====================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXAudioEffect::SoXAudioEffect ()
     : _audioParameterMap{},
       _currentTimePosition{Real::infinity},
       _previousTimePosition{Real::infinity},
       _timePositionHasMoved{true},
       _effectDescriptor{},
       _sampleRate{100},
       _channelCount{0},
       _parametersAreValid{false}
{
}

/*--------------------*/

SoXAudioEffect::~SoXAudioEffect ()
{
    Logging_trace1(">>: %1", toString());
    delete _effectDescriptor;
    Logging_trace("<<");
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXAudioEffect::toString () const
{
    String st = "SoXAudioEffect(";
    st += _asRawString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXAudioEffect::_asRawString () const
{
    String st;
    st += "_sampleRate = " + TOSTRING(_sampleRate) + "Hz";
    st += ", _channelCount = " + TOSTRING(_channelCount);
    st += (", _currentTimePosition = "
           + TOSTRING(_currentTimePosition) + "s");
    st += (", _previousTimePosition = "
           + TOSTRING(_previousTimePosition) + "s");
    st += (", _timePositionHasMoved = "
           + TOSTRING(_timePositionHasMoved));
    st += ", _parametersAreValid = " + TOSTRING(_parametersAreValid);
    st += ", _audioParameterMap = " + _audioParameterMap.toString();
    st += ", _effectDescriptor = " + _effectDescriptorToString();

    return st;
}

/*--------------------*/

String SoXAudioEffect::_effectDescriptorToString () const
{
    return "_EffectDescriptor()";
}

/*--------------------*/
/* property queries   */
/*--------------------*/

String SoXAudioEffect::name () const
{
    return "???";
}

/*--------------------*/
/* parameter map      */
/*--------------------*/

SoXAudioParameterMap& SoXAudioEffect::audioParameterMap () const
{
    return (SoXAudioParameterMap &) _audioParameterMap;
}

/*--------------------*/

SoXAudioValueChangeKind
SoXAudioEffect::setValue (IN String& parameterName,
                          IN String& value,
                          IN bool recalculationIsSuppressed)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsSuppressed = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsSuppressed));

    SoXAudioValueChangeKind result =
        SoXAudioValueChangeKind::parameterChange;

    if (value == _audioParameterMap.value(parameterName)) {
        // break cycles: if value is already known, ignore this
        // request
    } else if (!_audioParameterMap
               .isAllowedValue(parameterName, value)) {
        // do not store a bad value
    } else {
        _audioParameterMap.setValue(parameterName, value);
        result = _setValueInternal(parameterName, value,
                                   recalculationIsSuppressed);
    }

    Logging_trace1("<<: %1", SoXAudioValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

bool SoXAudioEffect::hasValidParameters () const
{
    Logging_trace(">>");
    const bool result = _parametersAreValid;
    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/

void SoXAudioEffect::setParameterValidity (IN bool isValid)
{
    Logging_trace1(">>: %1", TOSTRING(isValid));
    _parametersAreValid = isValid;
    Logging_trace("<<");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXAudioEffect::prepareToPlay (IN Real sampleRate)
{
    Logging_trace1(">>: sampleRate = %1", TOSTRING(sampleRate));
    _previousTimePosition = Real::infinity;
    _sampleRate           = sampleRate;
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::releaseResources ()
{
    Logging_trace(">>");
    _previousTimePosition = Real::infinity;
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::processBlock (IN Real timePosition,
                                   INOUT SoXAudioSampleBuffer& buffer)
{
    // Logging_trace1(">>: timePosition = %1", TOSTRING(timePosition));

    _currentTimePosition = timePosition;
    _channelCount        = buffer.size();

    // check whether timing has changed significantly
    const Real difference = _currentTimePosition - _previousTimePosition;
    _timePositionHasMoved = (difference < 0 || difference > 0.1);

    // Logging_trace("<<");
}
