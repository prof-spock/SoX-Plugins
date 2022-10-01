/**
 * @file
 * The <C>SoXAudioParameterMap</C> body implements a map of string
 * keys and associated values of <C>SoXAudioParameterKind</C>; those
 * values can be set and queried; this provides an interpretative
 * coupling between audio processors and audio editors.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioParameterMap.h"

#include "Assertion.h"
#include "Logging.h"

/*--------------------*/

using SoXPlugins::Helpers::SoXAudioParameterKind;
using SoXPlugins::Helpers::SoXAudioParameterMap;

namespace Helpers = SoXPlugins::Helpers;

/** abbreviated form of function name */
#define prefix StringUtil::prefix
/** abbreviated form of function name */
#define stringMapToStringExplicit StringUtil::stringMapToStringExplicit
/** abbreviated form of function name */
#define substring StringUtil::substring

/*====================*/

static const String rangeListSeparator = "¦";
static const Natural notFound = std::string::npos;

/*--------------------*/

const String SoXAudioParameterMap::unknownValue = "???";
const String SoXAudioParameterMap::widgetPageSeparator = "#";

/*--------------------*/
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
        SoXAudioParameterMap::unknownValue;
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
static void _splitParameterName (IN String parameterName,
                                 OUT String& effectiveParameterName,
                                 OUT Natural& pageIndex,
                                 OUT Integer& nominalPageIndex)
{
    const String separator = SoXAudioParameterMap::widgetPageSeparator;
    const Natural position = parameterName.find(separator);
    const Natural separatorLength = separator.length();

    if (position == notFound) {
        effectiveParameterName = parameterName;
        pageIndex = 0;
        nominalPageIndex = Integer{0};
    } else {
        effectiveParameterName =
            substring(parameterName, position + separatorLength);
        nominalPageIndex =
            StringUtil::toInteger(prefix(parameterName, position));
        pageIndex = (nominalPageIndex < 0 ? Natural{0}
                     : Natural{(size_t) nominalPageIndex});
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
StringList _splitRangeData (IN SoXAudioParameterMap* parameterMap,
                            IN Dictionary parameterNameToValueRangeMap,
                            IN String& parameterName,
                            IN SoXAudioParameterKind kind)
{
    StringList result;
    const SoXAudioParameterKind currentKind =
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

String Helpers::audioParameterKindToString (IN SoXAudioParameterKind& kind)
{
    String result;

    switch (kind) {
        case SoXAudioParameterKind::realKind:
            result = "realKind";
            break;

        case SoXAudioParameterKind::intKind:
            result = "intKind";
            break;

        case SoXAudioParameterKind::enumKind:
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

SoXAudioParameterMap::SoXAudioParameterMap ()
{
    Logging_trace(">>");
    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

SoXAudioParameterMap::~SoXAudioParameterMap ()
{
    Logging_trace(">>");
    Logging_trace("<<");
}

/*--------------------*/

String SoXAudioParameterMap::toString () const
{
    const String parameterNameToKindRepr =
        stringMapToStringExplicit<
            map<String, SoXAudioParameterKind>,
            SoXAudioParameterKind,
            audioParameterKindToString
        >("Map", _parameterNameToKindMap);

    const String result =
        ("SoXAudioParameterMap("
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

void SoXAudioParameterMap::clear()
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

StringList SoXAudioParameterMap::parameterNameList () const
{
    Logging_trace(">>");
    const StringList& result = _parameterNameList;
    Logging_trace1("<<: %1", result.toString());
    return result;
}

/*--------------------*/

Dictionary SoXAudioParameterMap::parameterNameToValueMap () const
{
    return _parameterNameToValueMap;
}

/*--------------------*/

SoXAudioParameterKind SoXAudioParameterMap::kind
                                          (IN String& parameterName)
    const
{
    Logging_trace1(">>: %1", parameterName);

    SoXAudioParameterKind result;
    const Boolean isKey = (_parameterNameToKindMap.find(parameterName)
                           != _parameterNameToKindMap.end());
    result = (isKey ? _parameterNameToKindMap.at(parameterName)
              : SoXAudioParameterKind::unknownKind);

    Logging_trace1("<<: %1", audioParameterKindToString(result));
    return result;
}

/*--------------------*/

void SoXAudioParameterMap::valueRangeEnum (IN String& parameterName,
                                           OUT StringList& result)
    const
{
    Logging_trace1(">>: %1", parameterName);

    const SoXAudioParameterKind p = kind(parameterName);

    if (p == SoXAudioParameterKind::enumKind
        && _parameterNameToValueRangeMap.contains(parameterName)) {
        const String listAsString =
            _parameterNameToValueRangeMap.at(parameterName);
        result = StringList::makeBySplit(listAsString,
                                         rangeListSeparator);
    }

    Logging_trace1("<<: %1", result.toString());
}

/*--------------------*/

void SoXAudioParameterMap::valueRangeInt (IN String& parameterName,
                                          OUT Integer& lowValue,
                                          OUT Integer& highValue,
                                          OUT Integer& delta)
    const
{
    Logging_trace1(">>: %1", parameterName);

    const StringList rangeValueList =
        _splitRangeData(this, _parameterNameToValueRangeMap, parameterName,
                        SoXAudioParameterKind::intKind);

    if (rangeValueList.size() != 3) {
        lowValue  = Integer{0};
        highValue = Integer{0};
        delta     = Integer{0};
    } else {
        lowValue  = StringUtil::toInteger(rangeValueList[0]);
        highValue = StringUtil::toInteger(rangeValueList[1]);
        delta     = StringUtil::toInteger(rangeValueList[2]);
    }

    Logging_trace3("<<: (%1, %2, %3)",
                   lowValue.toString(), highValue.toString(),
                   delta.toString());
}

/*--------------------*/

void SoXAudioParameterMap::valueRangeReal (IN String& parameterName,
                                           OUT Real& lowValue,
                                           OUT Real& highValue,
                                           OUT Real& delta)
    const
{
    Logging_trace1(">>: %1", parameterName);

    const StringList rangeValueList =
        _splitRangeData(this, _parameterNameToValueRangeMap,
                        parameterName, SoXAudioParameterKind::realKind);

    if (rangeValueList.size() != 3) {
        lowValue  = 0.0;
        highValue = 0.0;
        delta     = 0.0;
    } else {
        lowValue  = StringUtil::toReal(rangeValueList[0]);
        highValue = StringUtil::toReal(rangeValueList[1]);
        delta     = StringUtil::toReal(rangeValueList[2]);
    }

    Logging_trace3("<<: (%1, %2, %3)",
                   TOSTRING(lowValue), TOSTRING(highValue),
                   TOSTRING(delta));
}

/*--------------------*/

Boolean SoXAudioParameterMap::isAllowedValue (IN String& parameterName,
                                              IN String& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    Boolean isOkay;

    if (!_parameterNameList.contains(parameterName)) {
        isOkay = false;
    } else {
        const SoXAudioParameterKind kind =
            _parameterNameToKindMap.at(parameterName);

        if (kind == SoXAudioParameterKind::intKind) {
            isOkay = StringUtil::isInt(value);
        } else if (kind == SoXAudioParameterKind::realKind) {
            isOkay = StringUtil::isReal(value);
        } else if (kind == SoXAudioParameterKind::enumKind) {
            StringList valueList;
            valueRangeEnum(parameterName, valueList);
            isOkay = valueList.contains(value);
        } else {
            isOkay = false;
        }

        if (isOkay && kind != SoXAudioParameterKind::enumKind) {
            // do the range check
            if (kind == SoXAudioParameterKind::intKind) {
                const Integer currentValue = StringUtil::toInteger(value);
                Integer lowValue, highValue, delta;
                valueRangeInt(parameterName, lowValue, highValue, delta);
                isOkay = (lowValue <= currentValue
                          && currentValue <= highValue
                          && (currentValue - lowValue) % delta == 0);
            } else {
                const Real currentValue = StringUtil::toReal(value);
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

Boolean SoXAudioParameterMap::adaptValueEnum (IN String& parameterName,
                                              INOUT String& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    Boolean isOkay;

    if (kind(parameterName) != SoXAudioParameterKind::enumKind) {
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

Boolean SoXAudioParameterMap::adaptValueInt (IN String& parameterName,
                                             INOUT Integer& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value.toString());

    Boolean isOkay;

    if (kind(parameterName) != SoXAudioParameterKind::intKind) {
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

Boolean SoXAudioParameterMap::adaptValueReal (IN String& parameterName,
                                              INOUT Real& value) const
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, TOSTRING(value));

    Boolean isOkay;

    if (kind(parameterName) != SoXAudioParameterKind::realKind) {
        isOkay = false;
    } else {
        Real lowValue, highValue, delta;
        valueRangeReal(parameterName, lowValue, highValue, delta);
        isOkay = (lowValue <= value && value <= highValue);
        value = (value < lowValue ? lowValue
                 : (value > highValue ? highValue
                    : value));
    }

    Logging_trace2("<<: isOkay = %1, value = %2",
                   TOSTRING(isOkay), TOSTRING(value));
    return isOkay;
}

/*--------------------*/
/* active state       */
/*--------------------*/

void SoXAudioParameterMap::setActiveness (IN String& parameterName,
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

Boolean SoXAudioParameterMap::isActive (IN String& parameterName) const
{
    Logging_trace1(">>: %1", parameterName);
    Boolean result = _activeParameterNameSet.contains(parameterName);
    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}

/*--------------------*/

void SoXAudioParameterMap::changeActivenessByPage (IN Natural lastPageIndex)
{
    Logging_trace1(">>: %1", TOSTRING(lastPageIndex));

    for (String& parameterName : _parameterNameList) {
        String effectiveParameterName;
        Natural pageIndex;
        Integer nominalPageIndex;
        _splitParameterName(parameterName, effectiveParameterName,
                            pageIndex, nominalPageIndex);
        const Boolean isActive = (pageIndex <= lastPageIndex);
        setActiveness(parameterName, isActive);
    }

    Logging_trace("<<");
}

/*--------------------*/
/* data access/change */
/*--------------------*/

void SoXAudioParameterMap::setValue (IN String& parameterName,
                                     IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2", parameterName, value);

    if (isAllowedValue(parameterName, value)) {
        _parameterNameToValueMap[parameterName] = value;
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioParameterMap::invalidateValue (IN String& parameterName)
{
    Logging_trace1(">>: %1", parameterName);

    if (_parameterNameList.contains(parameterName)) {
        _parameterNameToValueMap[parameterName] = unknownValue;
    }

    Logging_trace("<<");
}

/*--------------------*/

String SoXAudioParameterMap::value (IN String& parameterName) const
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

void SoXAudioParameterMap::setKindInt (IN String& parameterName,
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
    _parameterNameToKindMap[parameterName] = SoXAudioParameterKind::intKind;
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

void SoXAudioParameterMap::setKindReal (IN String& parameterName,
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
        SoXAudioParameterKind::realKind;
    const String lowValueAsString = TOSTRING(lowValue);
    const String rangeAsString =
        (lowValueAsString + rangeListSeparator
         + TOSTRING(highValue) + rangeListSeparator
         + TOSTRING(delta));
    _parameterNameToValueRangeMap[parameterName] = rangeAsString;
    setValue(parameterName, lowValueAsString);

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioParameterMap::setKindEnum (IN String& parameterName,
                                        IN StringList& valueList)
{
    Assertion_pre(valueList.size() > 0, "value list must be non-empty");
    Logging_trace2(">>: %1, range = %2",
                   parameterName, valueList.toString());

    _addToParameterList(_parameterNameList, _parameterNameToValueMap,
                        _activeParameterNameSet, parameterName);
    _parameterNameToKindMap[parameterName] = SoXAudioParameterKind::enumKind;
    const String rangeAsString = valueList.join(rangeListSeparator);
    _parameterNameToValueRangeMap[parameterName] = rangeAsString;
    setValue(parameterName, valueList[0]);

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioParameterMap::setKindAndValueInt
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

void SoXAudioParameterMap::setKindAndValueReal
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

void SoXAudioParameterMap::setKindAndValueEnum
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
SoXAudioParameterMap::pagedParameterName (IN String parameterName,
                                          IN Natural pageIndex)
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
SoXAudioParameterMap
::splitParameterName (IN String parameterName,
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
SoXAudioParameterMap
::splitParameterName (IN String parameterName,
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
SoXAudioParameterMap::isPageSelector (IN String parameterName) {
    Logging_trace1(">>: %1", parameterName);

    String effectiveParameterName;
    Natural pageIndex;
    Integer nominalPageIndex;
    _splitParameterName(parameterName, effectiveParameterName,
                        pageIndex, nominalPageIndex);
    const Boolean isPageSelector = (nominalPageIndex == -1);

    Logging_trace1("<<: %1", TOSTRING(isPageSelector));
    return isPageSelector;
}
