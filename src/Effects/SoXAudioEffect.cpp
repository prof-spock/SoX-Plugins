/**
 * @file
 * The <C>SoXAudioEffect</C> body implements the root class for
 * all SoX audio effects.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioEffect.h"
#include "Logging.h"

/*--------------------*/

using SoXPlugins::Effects::SoXAudioEffect;

/*====================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXAudioEffect::SoXAudioEffect ()
     : _valueChangeHandler{NULL},
       _notificationProc{NULL},
       _sampleRate{100.0},
       _channelCount{0},
       _effectParameterMap{},
       _currentTimePosition{Real::infinity},
       _expectedNextTimePosition{Real::infinity},
       _timePositionHasMoved{true},
       _parametersAreValid{false},
       _effectDescriptor{}
{
    Logging_trace(">>");
    Logging_trace("<<");
}

/*--------------------*/

SoXAudioEffect::~SoXAudioEffect ()
{
    Logging_trace1(">>: %1", toString());
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
    st += (", _expectedNextTimePosition = "
           + TOSTRING(_expectedNextTimePosition) + "s");
    st += (", _timePositionHasMoved = "
           + TOSTRING(_timePositionHasMoved));
    st += ", _parametersAreValid = " + TOSTRING(_parametersAreValid);
    st += ", _effectParameterMap = " + _effectParameterMap.toString();
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

Real SoXAudioEffect::tailLength () const
{
    return 0.0;
}

/*--------------------*/
/* parameter map      */
/*--------------------*/

SoXEffectParameterMap& SoXAudioEffect::effectParameterMap () const
{
    return (SoXEffectParameterMap &) _effectParameterMap;
}

/*--------------------*/

SoXParameterValueChangeKind
SoXAudioEffect::setValue (IN String& parameterName,
                          IN String& value,
                          IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    SoXParameterValueChangeKind result =
        SoXParameterValueChangeKind::noChange;

    if (!_effectParameterMap.valueIsDifferent(parameterName, value)) {
        /* break cycles: if value is already known, ignore this
           request */
    } else if (!_effectParameterMap
               .isAllowedValue(parameterName, value)) {
        /* do not store a bad value */
    } else {
        _effectParameterMap.setValue(parameterName, value);
        result = _setValueInternal(parameterName, value,
                                   recalculationIsForced);
    }

    Logging_trace1("<<: %1", SoXParameterValueChangeKind_toString(result));
    return result;
}

/*--------------------*/

void SoXAudioEffect::setValueViaParent (IN String& parameterName,
                                        IN String& value,
                                        IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    if (_valueChangeHandler == NULL || _notificationProc == NULL) {
        /* set value directly in effect */
        setValue(parameterName, value, recalculationIsForced);
    } else {
        /* use indirection via enclosing object */
        (*_notificationProc)(_valueChangeHandler,
                             parameterName, value, recalculationIsForced);
    }

    Logging_trace("<<");
}

/*--------------------*/

Boolean SoXAudioEffect::hasValidParameters () const
{
    Logging_trace(">>");
    const Boolean result = _parametersAreValid;
    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/

void SoXAudioEffect::setParameterValidity (IN Boolean isValid)
{
    Logging_trace1(">>: %1", TOSTRING(isValid));
    _parametersAreValid = isValid;
    Logging_trace("<<");
}

/*----------------------*/
/* data change listener */
/*----------------------*/

void SoXAudioEffect::
setValueChangeHandler (Object valueChangeHandler,
                       ValueChangeNotification notificationProc)
{
    Logging_trace(">>");

    _valueChangeHandler = valueChangeHandler;
    _notificationProc   = notificationProc;

    Logging_trace("<<");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXAudioEffect::prepareToPlay (IN Real sampleRate)
{
    Logging_trace1(">>: sampleRate = %1", TOSTRING(sampleRate));
    _expectedNextTimePosition = Real::infinity;
    _sampleRate               = sampleRate;
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::releaseResources ()
{
    Logging_trace(">>");
    _expectedNextTimePosition = Real::infinity;
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::processBlock (IN Real timePosition,
                                   INOUT AudioSampleListVector& buffer)
{
    Logging_trace1(">>: timePosition = %1", TOSTRING(timePosition));

    _currentTimePosition = timePosition;
    _channelCount        = buffer.size();

    /* check whether timing has changed significantly */
    const Real absoluteDifference =
        Real::abs(_currentTimePosition - _expectedNextTimePosition);
    _timePositionHasMoved = (absoluteDifference > 1E-3);

    const Natural sampleCount = buffer[0].size();
    _expectedNextTimePosition = (timePosition
                                 + Real(sampleCount) / _sampleRate);

    Logging_trace("<<");
}
