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

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/*====================*/

static const String _undefinedString = "???";

/*--------------------*/
/* setup              */
/*--------------------*/

SoXAudioEffect::SoXAudioEffect ()
     : _valueChangeObserver{nullptr},
       _dataPointRate{100.0},
       _channelCount{0},
       _effectParameterMap{},
       _currentTimePosition{-99999.99},
       _deltaTime{-99999.99},
       _expectedNextTimePosition{-99999.99},
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
    Logging_trace(">>");
    _effectDescriptor = nullptr;
    Logging_trace("<<");
}

/*-----------------------*/
/* string representation */
/*-----------------------*/

String SoXAudioEffect::toString () const
{
    String effectName = name();
    STR::replace(effectName, " ", "");
    String className = (effectName == _undefinedString
                        ? "SoX"
                        : effectName + "_");
    className += "AudioEffect";
    String st;
    st += className + "(";
    st += "_dataPointRate = " + TOSTRING(_dataPointRate) + "Hz";
    st += ", _channelCount = " + TOSTRING(_channelCount);
    st += (", _currentTimePosition = "
           + TOSTRING(_currentTimePosition) + "s");
    st += (", _deltaTime = " + TOSTRING(_deltaTime) + "s");
    st += (", _expectedNextTimePosition = "
           + TOSTRING(_expectedNextTimePosition) + "s");
    st += (", _timePositionHasMoved = "
           + TOSTRING(_timePositionHasMoved));
    st += ", _parametersAreValid = " + TOSTRING(_parametersAreValid);
    st += ", _effectParameterMap = " +  _effectParameterMap.toString();
    st += ", _effectDescriptor = " + _effectDescriptorToString();
    st += ")";

    return st;
}

/*--------------------*/

String SoXAudioEffect::_effectDescriptorToString () const
{
    return (_effectDescriptor == nullptr
            ? "nullptr"
            : "_EffectDescriptor()");
}

/*--------------------*/

String SoXAudioEffect::_internalState () const
{
    return "";
}

/*--------------------*/
/* property queries   */
/*--------------------*/

Natural SoXAudioEffect::channelCount () const
{
    return _channelCount;
}

/*--------------------*/

Real SoXAudioEffect::dataPointRate () const
{
    return _dataPointRate;
}

/*--------------------*/

Boolean SoXAudioEffect::isTimeVariant () const
{
    return false;
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

String SoXAudioEffect::name () const
{
    return _undefinedString;
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

void SoXAudioEffect::setParameterValidity (IN Boolean isValid)
{
    Logging_trace1(">>: %1", TOSTRING(isValid));
    _parametersAreValid = isValid;
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::setValue (IN String& parameterName,
                               IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    Dictionary dictionary;
    dictionary.set(parameterName, value);
    setValues(dictionary);

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::setValues (IN Dictionary& dictionary)
{
    Logging_trace1(">>: %1", dictionary.toString());
    
    Dictionary changedValueDictionary;
    SoXValueChangeKind valueChangeKind =
        SoXValueChangeKind::noChange;

    for (const auto& [parameterName, value] : dictionary) {
        if (!_effectParameterMap.valueIsDifferent(parameterName, value)) {
            /* break cycles: if value is already known, ignore change */
        } else if (!_effectParameterMap
                   .isAllowedValue(parameterName, value)) {
            /* do not store a bad value */
        } else {
            _effectParameterMap.setValue(parameterName, value);
            changedValueDictionary.set(parameterName, value);
            SoXValueChangeKind localValueChangeKind =
                _setValueInternal(parameterName, value);
            valueChangeKind =
                std::max(localValueChangeKind, valueChangeKind);
        }
    }

    _valueChangeObserver.notifyAboutValueChange(changedValueDictionary,
                                                valueChangeKind);

    Logging_trace("<<");
}

/*----------------------*/
/* data change listener */
/*----------------------*/

void SoXAudioEffect::
setValueChangeHandler (Object valueChangeHandler,
                       SoXValueChangeNotification notificationProc)
{
    Logging_trace(">>");
    _valueChangeObserver.set(valueChangeHandler, notificationProc);
    Logging_trace("<<");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXAudioEffect::_handleDataPointRateChange (IN Real dataPointRate)
{
    Logging_trace1(">>: dataPointRate = %1", TOSTRING(dataPointRate));
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::prepareToPlay (IN Real dataPointRate)
{
    Logging_trace1(">>: dataPointRate = %1", TOSTRING(dataPointRate));

    _deltaTime                = Real::one / dataPointRate;
    _expectedNextTimePosition = Real::infinity;

    if (dataPointRate == _dataPointRate) {
        Logging_trace("--: no recalculation");
    } else {
        _dataPointRate = dataPointRate;
        _handleDataPointRateChange(dataPointRate);
    }
    
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::processBlock (IN Real timePosition,
                                   INOUT AudioDataPointListVector& buffer)
{
    Logging_trace1(">>: timePosition = %1", TOSTRING(timePosition));

    const Natural newChannelCount = buffer.size();
    const Boolean channelCountHasChanged =
        _channelCount != newChannelCount;

    _currentTimePosition = timePosition;
    _channelCount        = newChannelCount;

    /* check whether timing has changed significantly */
    const Real absoluteDifference =
        Real::abs(_currentTimePosition - _expectedNextTimePosition);
    _timePositionHasMoved = (absoluteDifference > 1E-3);

    if (_timePositionHasMoved && isTimeVariant()
        || channelCountHasChanged) {
        /* channel count has changed or playhead was moved ==> keep
           time synchronisation of modulation waveforms */
        recalculate();
    }

    if (_channelCount > 0) {
        const Natural dataPointCount = buffer[0].size();
        _expectedNextTimePosition = (timePosition
                                     + Real(dataPointCount) * _deltaTime);
    }

    if (_parametersAreValid) {
        String internalState, st;
        Real effectiveTime = timePosition;

        const Natural dataPointCount = buffer[0].size();
        AudioDataPointList inputDataPointList{_channelCount};
        AudioDataPointList outputDataPointList{_channelCount};

        /* go through all frames */
        for (Natural i = 0;  i < dataPointCount;  i++) {
            buffer.getFrame(inputDataPointList, i);

            if (Logging_isActive) {
                st = STR::expand("time = %1ms, inputDataPoints = %2",
                                 TOSTRING(effectiveTime * 1000.0),
                                 inputDataPointList.toString(false));
                internalState = _internalState();

                if (internalState > "") {
                    st += ", " + internalState;
                }
            }

            _applyForFrame(inputDataPointList, outputDataPointList);

            Logging_trace2("--: %1, outputDataPoints = %2",
                           st, outputDataPointList.toString(false));
            buffer.setFrame(outputDataPointList, i);
            effectiveTime += _deltaTime;
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::recalculate ()
{
    Logging_trace(">>");
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioEffect::releaseResources ()
{
    Logging_trace(">>");
    _expectedNextTimePosition = Real::infinity;
    Logging_trace("<<");
}

