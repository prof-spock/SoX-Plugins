/**
 * @file
 * The <C>SoXEffectParameterMap</C> body implements a map of string
 * keys to associated values of <C>SoXEffectParameterKind</C>; those
 * values can be set and queried; this provides an interpretative
 * coupling between audio processors and audio editors.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXEffectParameterMap.h"

#include <cmath>
#include "Assertion.h"
#include "Logging.h"

/*--------------------*/

using SoXPlugins::Helpers::SoXEffectParameterKind;
using SoXPlugins::Helpers::SoXEffectParameterMap;

namespace Helpers = SoXPlugins::Helpers;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

/*====================*/

static const String rangeListSeparator = "¦";

/*--------------------*/

const String SoXEffectParameterMap::unknownValue = "???";
const String SoXEffectParameterMap::widgetPageSeparator = "#";
const Integer SoXEffectParameterMap::selectorPage    = -1;
const Integer SoXEffectParameterMap::pageCounterPage = -2;

/*--------------------*/
/*--------------------*/

/**
 * Checks and adapts string <C>value</C> for a real parameter named
 * to precision <C>delta</C> of that parameter.
 *
 * @param[inout] value          value of real parameter to be adapted
 * @param[in]    delta          the delta value for this parameter
 */
void _adaptRealValueToPrecision (INOUT String& value,
                                 IN Real delta)
{
    Logging_trace2(">>: value = %1, delta = %2",
                   value, TOSTRING(delta));

    Real v = STR::toReal(value);
    Real integralDigitCount =
        Real::maximum(Real::one,
                      Real::one + (log10((float) v.abs())));
    Real fractionalDigitCount = Real{-log10((double) delta)}.round();

    if (fractionalDigitCount > Real::zero) {
        Natural precision =
            (Natural) (integralDigitCount + fractionalDigitCount);
        value = STR::toString(v,
                              precision,
                              (Natural) fractionalDigitCount);
    }
    
    Logging_trace1("<<: value = %1", value);
}

/*--------------------*/

/**
 * Adds a new parameter name <C>parameterName</C> to internal lists and
 * clears any existing value, kind and value range settings for it
 * 
 * @param[inout] parameterNameList        list of parameter names in map
 * @param[inout] parameterNameToValueMap  mapping from names to values
 * @param[inout] activeParameterNameSet   set of parameter names considered
 *                                        active
 * @param[in]    parameterName            name of parameter to be added
 */
void _addToParameterList (INOUT StringList& parameterNameList,
                          INOUT Dictionary& parameterNameToValueMap,
                          INOUT StringSet& activeParameterNameSet,
                          IN String& parameterName)
{
    if (!parameterNameList.contains(parameterName)) {
        parameterNameList.append(parameterName);
    }

    parameterNameToValueMap[parameterName] =
        SoXEffectParameterMap::unknownValue;
    activeParameterNameSet.add(parameterName);
}

/*--------------------*/

/**
 * Splits <C>parameterName</C> into <C>pageIndex</C>,
 * <C>effectiveParameterName</C> and <C>nominalPageIndex</C>.
 *
 * @param[in]  parameterName           parameter name containing
 *                                     page index
 * @param[out] effectiveParameterName  the effective parameter name
 *                                     without page index
 * @param[out] pageIndex               the normalized page index for
 *                                     this parameter
 * @param[out} nominalPageIndex        the effective page index for
 *                                     this parameter
 */
static void _splitParameterName (IN String& parameterName,
                                 OUT String& effectiveParameterName,
                                 OUT Natural& pageIndex,
                                 OUT Integer& nominalPageIndex)
{
    const Natural notFound = Natural::maximumValue();
    const String separator = SoXEffectParameterMap::widgetPageSeparator;
    const Natural position = STR::find(parameterName, separator);
    const Natural separatorLength = separator.length();

    if (position == notFound) {
        effectiveParameterName = parameterName;
        pageIndex = 0;
        nominalPageIndex = Integer{0};
    } else {
        effectiveParameterName =
            STR::substring(parameterName, position + separatorLength);
        nominalPageIndex =
            STR::toInteger(STR::prefix(parameterName, position));
        pageIndex = (Natural) Integer::maximum(nominalPageIndex, 0);
    }
}

/*--------------------*/

/**
 * Returns a string triple of (lowValue, highValue, delta) for given
 * <C>parameterName</C> within audio parameter map <C>parameterMap</C>
 * and its associated range map <C>parameterNameToValueRangeMap</C>;
 * also ensures that <C>kind</C> is identical to kind of parameter
 */
static
StringList _splitRangeData (IN SoXEffectParameterMap* parameterMap,
                            IN Dictionary parameterNameToValueRangeMap,
                            IN String& parameterName,
                            IN SoXEffectParameterKind kind)
{
    StringList result;
    const SoXEffectParameterKind currentKind =
        parameterMap->kind(parameterName);

    if (currentKind == kind
        && parameterNameToValueRangeMap.contains(parameterName)) {
        const String rangeAsString =
            parameterNameToValueRangeMap.at(parameterName);
        result = StringList::makeBySplit(rangeAsString,
                                         rangeListSeparator);
    }

    return result;
}

/*--------------------*/
/* EXPORTED ROUTINES  */
/*--------------------*/

String Helpers::effectParameterKindToString
                    (IN SoXEffectParameterKind& kind)
{
    String result;

    switch (kind) {
        case SoXEffectParameterKind::realKind:
            result = "realKind";
            break;

        case SoXEffectParameterKind::intKind:
            result = "intKind";
            break;

        case SoXEffectParameterKind::enumKind:
            result = "enumKind";
            break;

        default:
            result = "unknownKind";
            break;
    }

    return result;
}

/*--------------------*/
/* setup              */
/*--------------------*/

SoXEffectParameterMap::SoXEffectParameterMap ()
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXEffectParameterMap::~SoXEffectParameterMap ()
{
    Logging_trace(">>");
    Logging_trace("<<");
}

/*--------------------*/

String SoXEffectParameterMap::toString () const
{
    const String parameterNameToKindRepr =
        STR::stringMapToStringExplicit<
            map<String, SoXEffectParameterKind>,
            SoXEffectParameterKind,
            effectParameterKindToString
        >("Map", _parameterNameToKindMap);

    const String result =
        ("SoXEffectParameterMap("
         "_parameterNameList = "
             + _parameterNameList.toString()
         + ", _parameterNameToValueMap = "
             + _parameterNameToValueMap.toString()
         + ", _parameterNameToKindMap = "
             + parameterNameToKindRepr
         + ", _parameterNameToValueRangeMap = "
             + _parameterNameToValueRangeMap.toString()
         + ", _activeParameterNameSet = "
             + _activeParameterNameSet.toString()
         + ")");

    return result;
}


/*--------------------*/
/* reset              */
/*--------------------*/

void SoXEffectParameterMap::clear()
{
    Logging_trace(">>");
    _parameterNameList.clear();
    _parameterNameToValueMap.clear();
    _parameterNameToKindMap.clear();
    _parameterNameToValueRangeMap.clear();
    _activeParameterNameSet.clear();
    Logging_trace("<<");
}

/*--------------------*/
/* property queries   */
/*--------------------*/

Boolean SoXEffectParameterMap::contains (IN String& parameterName) const
{
    Logging_trace(">>");
    Boolean result = _parameterNameToValueMap.contains(parameterName);
    Logging_trace1("<<: %1", result.toString());
    return result;
}

/*--------------------*/

StringList SoXEffectParameterMap::parameterNameList () const
{
    Logging_trace(">>");
    const StringList& result = _parameterNameList;
    Logging_trace1("<<: %1", result.toString());
    return result;
}

/*--------------------*/

Dictionary SoXEffectParameterMap::parameterNameToValueMap () const
{
    return _parameterNameToValueMap;
}

/*--------------------*/

SoXEffectParameterKind SoXEffectParameterMap::kind
                                          (IN String& parameterName)
    const
{
    Logging_trace1(">>: %1", parameterName);

    SoXEffectParameterKind result;
    const Boolean isKey = (_parameterNameToKindMap.find(parameterName)
                           != _parameterNameToKindMap.end());
    result = (isKey ? _parameterNameToKindMap.at(parameterName)
              : SoXEffectParameterKind::unknownKind);

    Logging_trace1("<<: %1", effectParameterKindToString(result));
    return result;
}

/*--------------------*/

void SoXEffectParameterMap::valueRangeEnum (IN String& parameterName,
                                            OUT StringList& result)
    const
{
    Logging_trace1(">>: %1", parameterName);

    const SoXEffectParameterKind p = kind(parameterName);

    if (p == SoXEffectParameterKind::enumKind
        && _parameterNameToValueRangeMap.contains(parameterName)) {
        const String listAsString =
            _parameterNameToValueRangeMap.at(parameterName);
        result = StringList::makeBySplit(listAsString,
                                         rangeListSeparator);
    }

    Logging_trace1("<<: %1", result.toString());
}

/*--------------------*/

void SoXEffectParameterMap::valueRangeInt (IN String& parameterName,
                                           OUT Integer& lowValue,
                                           OUT Integer& highValue,
                                           OUT Integer& delta)
    const
{
    Logging_trace1(">>: %1", parameterName);

    const StringList rangeValueList =
        _splitRangeData(this, _parameterNameToValueRangeMap, parameterName,
                        SoXEffectParameterKind::intKind);

    if (rangeValueList.size() != 3) {
        lowValue  = Integer{0};
        highValue = Integer{0};
        delta     = Integer{0};
    } else {
        lowValue  = STR::toInteger(rangeValueList[0]);
        highValue = STR::toInteger(rangeValueList[1]);
        delta     = STR::toInteger(rangeValueList[2]);
    }

    Logging_trace3("<<: (%1, %2, %3)",
                   lowValue.toString(), highValue.toString(),
                   delta.toString());
}

/*--------------------*/

void SoXEffectParameterMap::valueRangeReal (IN String& parameterName,
                                            OUT Real& lowValue,
                                            OUT Real& highValue,
                                            OUT Real& delta)
    const
{
    Logging_trace1(">>: %1", parameterName);

    const StringList rangeValueList =
        _splitRangeData(this, _parameterNameToValueRangeMap,
                        parameterName, SoXEffectParameterKind::realKind);

    if (rangeValueList.size() != 3) {
        lowValue  = 0.0;
        highValue = 0.0;
        delta     = 0.0;
    } else {
        lowValue  = STR::toReal(rangeValueList[0]);
        highValue = STR::toReal(rangeValueList[1]);
        delta     = STR::toReal(rangeValueList[2]);
    }

    Logging_trace3("<<: (%1, %2, %3)",
                   TOSTRING(lowValue), TOSTRING(highValue),
                   TOSTRING(delta));
}

/*--------------------*/

Boolean SoXEffectParameterMap::isAllowedValue (IN String& parameterName,
                                               IN String& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    Boolean isOkay;

    if (!_parameterNameList.contains(parameterName)) {
        isOkay = false;
    } else {
        const SoXEffectParameterKind parameterKind =
            _parameterNameToKindMap.at(parameterName);

        if (parameterKind == SoXEffectParameterKind::intKind) {
            isOkay = STR::isInt(value);
        } else if (parameterKind == SoXEffectParameterKind::realKind) {
            isOkay = STR::isReal(value);
        } else if (parameterKind == SoXEffectParameterKind::enumKind) {
            StringList valueList;
            valueRangeEnum(parameterName, valueList);
            isOkay = valueList.contains(value);
        } else {
            isOkay = false;
        }

        if (isOkay && parameterKind != SoXEffectParameterKind::enumKind) {
            /* do the range check */
            if (parameterKind == SoXEffectParameterKind::intKind) {
                const Integer currentValue = STR::toInteger(value);
                Integer lowValue, highValue, delta;
                valueRangeInt(parameterName, lowValue, highValue, delta);
                isOkay = (lowValue <= currentValue
                          && currentValue <= highValue
                          && (currentValue - lowValue) % delta == 0);
            } else {
                const Real currentValue = STR::toReal(value);
                Real lowValue, highValue, delta;
                valueRangeReal(parameterName, lowValue, highValue, delta);
                isOkay = (lowValue <= currentValue
                          && currentValue <= highValue);
            }
        }
    }

    Logging_trace1("<<: %1", TOSTRING(isOkay));
    return isOkay;
}

/*--------------------*/

Boolean SoXEffectParameterMap::adaptValueEnum (IN String& parameterName,
                                               INOUT String& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    Boolean isOkay;

    if (kind(parameterName) != SoXEffectParameterKind::enumKind) {
        isOkay = false;
    } else {
        isOkay = true;
        StringList valueList{};
        valueRangeEnum(parameterName, valueList);

        if (!valueList.contains(value)) {
            value = valueList[0];
        }
    }

    Logging_trace2("<<: isOkay = %1, value = %2",
                   TOSTRING(isOkay), value);
    return isOkay;
}

/*--------------------*/

Boolean SoXEffectParameterMap::adaptValueInt (IN String& parameterName,
                                              INOUT Integer& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value.toString());

    Boolean isOkay;

    if (kind(parameterName) != SoXEffectParameterKind::intKind) {
        isOkay = false;
    } else {
        Integer lowValue, highValue, delta;
        valueRangeInt(parameterName, lowValue, highValue, delta);
        isOkay = (lowValue <= value && value <= highValue);
        value = (value < lowValue ? lowValue
                 : (value > highValue ? highValue
                    : value));
    }

    Logging_trace2("<<: isOkay = %1, value = %2",
                   TOSTRING(isOkay), value.toString());
    return isOkay;
}

/*--------------------*/

Boolean SoXEffectParameterMap::adaptValueReal (IN String& parameterName,
                                               INOUT Real& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, TOSTRING(value));

    Boolean isOkay;

    if (kind(parameterName) != SoXEffectParameterKind::realKind) {
        isOkay = false;
    } else {
        Real lowValue, highValue, delta;
        valueRangeReal(parameterName, lowValue, highValue, delta);
        isOkay = (lowValue <= value && value <= highValue);
        value = Real::maximum(lowValue, Real::minimum(value, highValue));

        /* bring value onto raster */
        Real fp = value.fractionalPart();
        fp = Real::round(fp / delta) * delta;
        value = value.integralPart() + fp;
    }

    Logging_trace2("<<: isOkay = %1, value = %2",
                   isOkay.toString(), TOSTRING(value));
    return isOkay;
}

/*--------------------*/

Boolean SoXEffectParameterMap::valueIsDifferent (IN String& parameterName,
                                                 IN String& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    Boolean isDifferent;
    const Boolean isRealValue =
        (kind(parameterName) == SoXEffectParameterKind::realKind);
    String storedValue = _parameterNameToValueMap.at(parameterName);

    if (!isRealValue) {
        isDifferent = (storedValue != value);
    } else {
        Real lowValue, highValue, delta;
        valueRangeReal(parameterName, lowValue, highValue, delta);
        Real sV = STR::toReal(storedValue);
        Real v  = STR::toReal(value);
        isDifferent = (Real::abs(sV - v) >= delta);
    }
    
    Logging_trace2("<<: %1 (storedValue = '%2')",
                   TOSTRING(isDifferent), storedValue);
    return isDifferent;
}

/*--------------------*/
/* active state       */
/*--------------------*/

void SoXEffectParameterMap::setActiveness (IN String& parameterName,
                                           IN Boolean isActive)
{
    Logging_trace2(">>: parameterName = %1, isActive = %2",
                   parameterName, TOSTRING(isActive));

    if (isActive) {
        _activeParameterNameSet.add(parameterName);
    } else {
        _activeParameterNameSet.remove(parameterName);
    }

    Logging_trace("<<");
}

/*--------------------*/

Boolean SoXEffectParameterMap::isActive (IN String& parameterName) const
{
    Logging_trace1(">>: %1", parameterName);
    Boolean result = _activeParameterNameSet.contains(parameterName);
    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/

void SoXEffectParameterMap::setActivenessForNameList
                               (IN StringList parameterNameList,
                                IN Boolean isActive)
{
    Logging_trace2(">>: isActive = %1, nameList = %2",
                   TOSTRING(isActive), parameterNameList.toString());

    if (isActive) {
        for (const String& parameterName : parameterNameList) {
            _activeParameterNameSet.add(parameterName);
        }
    } else {
        for (const String& parameterName : parameterNameList) {
            _activeParameterNameSet.remove(parameterName);
        }
    }
    
    Logging_trace("<<");
}

/*--------------------*/

void SoXEffectParameterMap::changeActivenessByPage (IN Natural lastPageIndex)
{
    Logging_trace1(">>: %1", TOSTRING(lastPageIndex));

    for (const String& parameterName : _parameterNameList) {
        String effectiveParameterName;
        Natural pageIndex;
        Integer nominalPageIndex;
        _splitParameterName(parameterName, effectiveParameterName,
                            pageIndex, nominalPageIndex);
        const Boolean parameterIsActive = (pageIndex <= lastPageIndex);
        setActiveness(parameterName, parameterIsActive);
    }

    Logging_trace("<<");
}

/*--------------------*/
/* data access/change */
/*--------------------*/

void SoXEffectParameterMap::setValue (IN String& parameterName,
                                      IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);
    String adaptedValue = value;

    if (isAllowedValue(parameterName, adaptedValue)) {
        Boolean isRealValue =
            (kind(parameterName) == SoXEffectParameterKind::realKind);

        if (isRealValue) {
            Real lowValue, highValue, delta;
            valueRangeReal(parameterName, lowValue, highValue, delta);
            _adaptRealValueToPrecision(adaptedValue, delta);
        }

        _parameterNameToValueMap[parameterName] = adaptedValue;
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXEffectParameterMap::invalidateValue (IN String& parameterName)
{
    Logging_trace1(">>: %1", parameterName);

    if (_parameterNameList.contains(parameterName)) {
        _parameterNameToValueMap[parameterName] = unknownValue;
    }

    Logging_trace("<<");
}

/*--------------------*/

String SoXEffectParameterMap::value (IN String& parameterName) const
{
    Logging_trace1(">>: %1", parameterName);
    String result = _parameterNameToValueMap.atWithDefault(parameterName,
                                                           unknownValue);
    Logging_trace1("<<: %1", result);
    return result;
}

/*--------------------*/
/* kind change        */
/*--------------------*/

void SoXEffectParameterMap::setKindInt (IN String& parameterName,
                                        IN Integer lowValue,
                                        IN Integer highValue,
                                        IN Integer delta)
{
    Assertion_pre(lowValue <= highValue, "interval must be non-empty");
    Assertion_pre(delta != 0, "delta must be non-zero");
    Logging_trace4(">>: %1, range = (%2, %3, %4)",
                   parameterName, lowValue.toString(),
                   highValue.toString(), delta.toString());

    _addToParameterList(_parameterNameList, _parameterNameToValueMap,
                        _activeParameterNameSet, parameterName);
    _parameterNameToKindMap[parameterName] = SoXEffectParameterKind::intKind;
    const String lowValueAsString = lowValue.toString();
    const String rangeAsString =
        (lowValueAsString + rangeListSeparator
         + highValue.toString() + rangeListSeparator
         + delta.toString());
    _parameterNameToValueRangeMap[parameterName] = rangeAsString;
    setValue(parameterName, lowValueAsString);

    Logging_trace("<<");
}

/*--------------------*/

void SoXEffectParameterMap::setKindReal (IN String& parameterName,
                                         IN Real lowValue,
                                         IN Real highValue,
                                         IN Real delta)
{
    Assertion_pre(lowValue <= highValue, "interval must be non-empty");
    Assertion_pre(delta > 1E-9, "delta must be greater than zero");
    Logging_trace4(">>: %1, range = (%2, %3, %4)",
                   parameterName, TOSTRING(lowValue),
                   TOSTRING(highValue), TOSTRING(delta));

    _addToParameterList(_parameterNameList, _parameterNameToValueMap,
                        _activeParameterNameSet, parameterName);
    _parameterNameToKindMap[parameterName] =
        SoXEffectParameterKind::realKind;
    const String lowValueAsString = TOSTRING(lowValue);
    const String rangeAsString =
        (lowValueAsString + rangeListSeparator
         + TOSTRING(highValue) + rangeListSeparator
         + STR::toString(delta, 0, 15, "0", true));
    _parameterNameToValueRangeMap[parameterName] = rangeAsString;
    setValue(parameterName, lowValueAsString);

    Logging_trace("<<");
}

/*--------------------*/

void SoXEffectParameterMap::setKindEnum (IN String& parameterName,
                                         IN StringList& valueList)
{
    Assertion_pre(valueList.size() > 0, "value list must be non-empty");
    Logging_trace2(">>: %1, range = %2",
                   parameterName, valueList.toString());

    _addToParameterList(_parameterNameList, _parameterNameToValueMap,
                        _activeParameterNameSet, parameterName);
    _parameterNameToKindMap[parameterName] = SoXEffectParameterKind::enumKind;
    const String rangeAsString = valueList.join(rangeListSeparator);
    _parameterNameToValueRangeMap[parameterName] = rangeAsString;
    setValue(parameterName, valueList[0]);

    Logging_trace("<<");
}

/*--------------------*/

void SoXEffectParameterMap::setKindAndValueInt
                               (IN String& parameterName,
                                IN Integer lowValue,
                                IN Integer highValue,
                                IN Integer delta,
                                IN Integer value)
{
    Logging_trace5(">>: %1, range = (%2, %3, %4), value = %5",
                   parameterName,
                   lowValue.toString(), highValue.toString(),
                   delta.toString(), value.toString());
    setKindInt(parameterName, lowValue, highValue, delta);
    setValue(parameterName, value.toString());
    Logging_trace("<<");
}

/*--------------------*/

void SoXEffectParameterMap::setKindAndValueReal
                               (IN String& parameterName,
                                IN Real lowValue,
                                IN Real highValue,
                                IN Real delta,
                                IN Real value)
{
    Logging_trace5(">>: %1, range = (%2, %3, %4), value = %5",
                   parameterName,
                   TOSTRING(lowValue), TOSTRING(highValue),
                   TOSTRING(delta), TOSTRING(value));
    setKindReal(parameterName, lowValue, highValue, delta);
    setValue(parameterName, TOSTRING(value));
    Logging_trace("<<");
}

/*--------------------*/

void SoXEffectParameterMap::setKindAndValueEnum
                               (IN String& parameterName,
                                IN StringList& valueList,
                                IN String& value)
{
    Logging_trace3(">>: %1, range = %2, value = %3",
                   parameterName, valueList.toString(), value);
    setKindEnum(parameterName, valueList);
    setValue(parameterName, value);
    Logging_trace("<<");
}

/*---------------------------*/
/* parameter names for pages */
/*---------------------------*/

String
SoXEffectParameterMap::pagedParameterName (IN String& parameterName,
                                           IN Integer pageIndex)
{
    Logging_trace2(">>: parameterName = %1, page = %2",
                   parameterName, TOSTRING(pageIndex));
    String result =
        (TOSTRING(pageIndex) + widgetPageSeparator + parameterName);
    Logging_trace1("<<: %1", result);
    return result;
}

/*--------------------*/

void
SoXEffectParameterMap
::splitParameterName (IN String& parameterName,
                      OUT String& effectiveParameterName,
                      OUT Natural& pageIndex)
{
    Integer nominalPageIndex;
    Logging_trace1(">>: %1", parameterName);
    _splitParameterName(parameterName, effectiveParameterName,
                        pageIndex, nominalPageIndex);
    Logging_trace2("<<: name = %1, page = %2",
                   effectiveParameterName, TOSTRING(pageIndex));
}

/*--------------------*/

void
SoXEffectParameterMap
::splitParameterName (IN String& parameterName,
                      OUT String& effectiveParameterName,
                      OUT Natural& pageIndex,
                      OUT Integer& nominalPageIndex)
{
    Logging_trace1(">>: %1", parameterName);
    _splitParameterName(parameterName, effectiveParameterName,
                        pageIndex, nominalPageIndex);
    Logging_trace3("<<: name = %1, page = %2, isPageSelector = %3",
                   effectiveParameterName, TOSTRING(pageIndex),
                   nominalPageIndex.toString());
}

/*--------------------*/

Boolean
SoXEffectParameterMap::isPageSelector (IN String& parameterName) {
    Logging_trace1(">>: %1", parameterName);

    String effectiveParameterName;
    Natural pageIndex;
    Integer nominalPageIndex;
    _splitParameterName(parameterName, effectiveParameterName,
                        pageIndex, nominalPageIndex);
    const Boolean isPageSelector = (nominalPageIndex == selectorPage);

    Logging_trace1("<<: %1", TOSTRING(isPageSelector));
    return isPageSelector;
}
