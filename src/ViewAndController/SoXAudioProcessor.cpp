/**
 * @file
 * The <C>SoXAudioProcessor</C> body implements an audio effect
 * wrapper for a plugin responsible for the communication to the
 * enclosing plugin host.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "GenericSet.h"
#include "Logging.h"
#include "MyArray.h"
#include "SoXAudioEditor.h"

/*--------------------*/

using Audio::AudioSampleListVector;
using BaseTypes::Containers::convertArray;
using BaseTypes::GenericTypes::GenericSet;
using SoXPlugins::Helpers::SoXEffectParameterKind;
using SoXPlugins::ViewAndController::SoXAudioEditor;
using SoXPlugins::Effects::ValueChangeNotification;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

using BaseTypes::Primitives::String_normalize;

/*============================================================*/

/** a set of audio editor pointers */
typedef GenericSet<SoXAudioEditor*> _SoXAudioEditorPtrSet;

/** a mapping from string to natural */
typedef GenericMap<String, Natural> StringToNaturalMap;

/** a JUCE array of audio processor parameter pointers */
typedef juce::Array<juce::AudioProcessorParameter*>
    _AudioProcessorParameterArray;

/*--------------------*/
/*--------------------*/

/** the double quote character */
static const Character quoteCharacter = '"';

/** number of decimal places for reals in serialized form */
static const Natural decimalPlaceCount = 5;

/** set of parameter names considered 'meta parameters' implicitely
 * changing other parameter values */
static const StringSet metaParameterNameSet =
    StringSet::fromList(StringList::fromList({"Effect Kind"}));

/*============================================================*/

static Integer _stretchToIntegerInterval (IN Real unitIntervalValue,
                                          IN Integer lowValue,
                                          IN Integer highValue);

static Real _stretchToRealInterval (IN Real unitIntervalValue,
                                    IN Real lowValue,
                                    IN Real highValue);

static void _updateAudioProcessorParameterRange
                    (INOUT juce::AudioProcessorParameter& parameterObject,
                     IN String& parameterName,
                     IN SoXEffectParameterMap& effectParameterMap);

/*============================================================*/

namespace SoXPlugins::ViewAndController {

    /** a listener for effect parameters */
    struct _EffectParameterListener
        : juce::AudioProcessorParameter::Listener {

        /** the associated processor for this listener */
        SoXAudioProcessor* processor;

        /*--------------------*/

        void parameterValueChanged (int paramIndex,
                                    float newValue) override;

        /*--------------------*/

        void parameterGestureChanged (int, bool) override;

    };

    /*--------------------*/
    /*--------------------*/

    /** the associated descriptor type for an audio processor */
    struct _SoXAudioProcessorDescriptor {

        /** the associated effect */
        SoXAudioEffect* effect{NULL};

        /** the set of observers to be notified on a value change */
        _SoXAudioEditorPtrSet observerSet;

        /** a listener for the effect parameters */
        _EffectParameterListener listener;

        /** a map from parameter name to index within parameter
         * list */
        StringToNaturalMap parameterNameToIndexMap;
    };
}

/*============================================================*/

using SoXPlugins::ViewAndController::_EffectParameterListener;
using SoXPlugins::ViewAndController::_SoXAudioProcessorDescriptor;

/*--------------------------*/
/* _EffectParameterListener */
/*--------------------------*/

void _EffectParameterListener::parameterGestureChanged (int, bool)
{
}

/*--------------------*/

void _EffectParameterListener::parameterValueChanged (int paramIndex,
                                                      float newValue)
{
    const Natural parameterIndex{paramIndex};
    const Real unitIntervalValue{newValue};
    Logging_trace2(">>: parameterIndex = %1, value = %2",
                   TOSTRING(parameterIndex),
                   TOSTRING(unitIntervalValue));

    /* find associated parameter */
    const juce::AudioProcessorParameter* parameter =
        processor->getParameters()[(int) parameterIndex];
    const String parameterName{parameter->getName(1000).toStdString()};
    Logging_trace1("--: parameterName = %1", parameterName);
    const SoXEffectParameterMap& parameterMap =
        processor->effectParameterMap();

    if (parameterMap.contains(parameterName)) {
        String value;
        const SoXEffectParameterKind kind =
            parameterMap.kind(parameterName);
        Logging_trace1("--: parameterKind = %1",
                       effectParameterKindToString(kind));

        if (kind == SoXEffectParameterKind::realKind) {
            Real lowValue, highValue, delta;
            parameterMap.valueRangeReal(parameterName,
                                        lowValue, highValue, delta);
            /* scale to range */
            value =
                TOSTRING(_stretchToRealInterval(unitIntervalValue,
                                                lowValue, highValue));
        } else if (kind == SoXEffectParameterKind::intKind) {
            Integer lowValue, highValue, delta;
            parameterMap.valueRangeInt(parameterName,
                                       lowValue, highValue, delta);
            /* scale to range */
            value =
                TOSTRING(_stretchToIntegerInterval(unitIntervalValue,
                                                   lowValue, highValue));
        } else if (kind == SoXEffectParameterKind::enumKind) {
            StringList enumValueList;
            parameterMap.valueRangeEnum(parameterName, enumValueList);
            Integer elementCount{(int) enumValueList.size()};
            Natural valueIndex =
                Natural{_stretchToIntegerInterval(unitIntervalValue,
                                                  0, elementCount - 1)};
            value = enumValueList.at(valueIndex);
        } else {
            Logging_traceError1("cannot happen, kind = %1",
                                effectParameterKindToString(kind));
        }

        if (value > "") {
            /* lock the juce message manager for updating */
            const juce::MessageManagerLock mutex;
            processor->setValue(parameterName, value, false);
        }
    }

    Logging_trace("<<");
}

/*============================================================*/

/*--------------------*/
/* internal routines  */
/*--------------------*/

/**
 * Constructs juce parameter object from <C>parameterName</C> with
 * data taken from <C>effectParameterMap</C>.
 *
 * @param[in] parameterName      name of audio parameter in map
 * @param[in] effectParameterMap  map of all audio parameters to
 *                               associated attributes
 * @return  juce audio parameter object
 */
static juce::AudioProcessorParameter*
_constructParameterObject (IN String& parameterName,
                           IN SoXEffectParameterMap& effectParameterMap)
{
    Logging_trace1(">>: %1", parameterName);

    const SoXEffectParameterKind kind =
        effectParameterMap.kind(parameterName);
    const String value = effectParameterMap.value(parameterName);

    juce::NormalisableRange<float> range;
    const juce::String juceParameterName = juce::String(parameterName);
    const juce::ParameterID juceParameterID = juceParameterName;
    juce::AudioProcessorParameter* parameterObject = NULL;

    if (kind == SoXEffectParameterKind::realKind
        || kind == SoXEffectParameterKind::intKind) {
        parameterObject =
            new juce::AudioParameterFloat(juceParameterID,
                                          juceParameterName,
                                          range, 1.0);
    } else if (kind == SoXEffectParameterKind::enumKind) {
        StringList enumValueList;
        effectParameterMap.valueRangeEnum(parameterName, enumValueList);
        juce::StringArray juceChoiceList;
        Natural parameterIndex = 0;
        Boolean isFound = false;

        for (const String& enumValue : enumValueList) {
            juceChoiceList.add(juce::String(enumValue));
            isFound = isFound || (value == enumValue);
            parameterIndex += (isFound ? 0 : 1);
        }

        /* workaround */
        while (juceChoiceList.size() < 2) {
            juceChoiceList.add("x");
        }

        /* for Apple audio units when changing a meta parameter, other
           parameter values may be affected by that */
        const Boolean isMetaParameter =
            metaParameterNameSet.contains(parameterName);
        const juce::AudioParameterChoiceAttributes attributes =
            juce::AudioParameterChoiceAttributes()
            .withMeta((bool)isMetaParameter);
        parameterObject =
            new juce::AudioParameterChoice(juceParameterID,
                                           juceParameterName,
                                           juceChoiceList,
                                           (int) parameterIndex,
                                           attributes);
    }

    _updateAudioProcessorParameterRange(*parameterObject,
                                        parameterName,
                                        effectParameterMap);

    Logging_trace1("<<: %1", (parameterObject == NULL ? "NULL" : "OKAY"));
    return parameterObject;
}

/*--------------------*/

/**
 * Converts audio parameter in <C>map</C> to serialized string form;
 * <C>title</C> is added for reference as the first line
 *
 * @param[in] parameterMap  map of audio parameters in processor
 * @param[in] title         title for audio processor parameter
 *                          serialization
 */
static String _convertMapToString (IN SoXEffectParameterMap& parameterMap,
                                   IN String& title)
{
    Logging_trace2(">>: map = %1, title = %2",
                   parameterMap.toString(), title);

    const StringList parameterNameList = parameterMap.parameterNameList();
    String result = title + "\n";

    for (Natural i = 0;  i < parameterNameList.size();  i++) {
        const String parameterName = parameterNameList[i];
        const SoXEffectParameterKind kind =
            parameterMap.kind(parameterName);
        String parameterValue = parameterMap.value(parameterName);

        if (kind == SoXEffectParameterKind::enumKind) {
            STR::prepend(parameterValue, quoteCharacter);
            STR::append(parameterValue, quoteCharacter);
        }

        /* write each entry as a line */
        result += (parameterName + " = " + parameterValue + "\n");
    }

    Logging_trace1("<<: %1", String_normalize(result));
    return result;
}

/*--------------------*/

/**
 * Sets parameter named <C>parameterName</C> to <C>value</C> for audio
 * processor given as <C>object</C>; calls associated setValue
 * function for processor
 *
 * @param[in] parameterName          name of parameter to be set
 * @param[in] value                  associated value of parameter
 * @param[in] recalculationIsForced  flag to tell whether complex
 *                                   recalculations should be done
 */
static void _dispatchValueChange (INOUT Object object,
                                  IN String& parameterName,
                                  IN String& value,
                                  IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    SoXAudioProcessor& processor = TOREFERENCE<SoXAudioProcessor>(object);
    processor.setValue(parameterName, value, recalculationIsForced);
    
    Logging_trace("<<");
}

/*--------------------*/

/**
 * Reads value settings from serialized form in <C>st</C> to
 * <C>audioProcessor</C> and into <C>parameterMap</C>
 *
 * @param[in] audioProcessor  the audio processor to be set by external
 *                            string
 * @param[in] parameterMap    map from audio parameters to associated
 *                            data
 * @param[in] st              string with serialized processor
 *                            information
 */
static void _readKeyValueMapString (INOUT SoXAudioProcessor* audioProcessor,
                                    INOUT SoXEffectParameterMap& parameterMap,
                                    IN String& st)
{
    Logging_trace1(">>: st = %1", st);

    /* read the data as written by the string conversion */
    const StringList lineList = StringList::makeBySplit(st, "\n");

    /* throw away empty last line and title => first index is 1,
       last index is lineCount - 1 */
    const Natural firstIndex = 1;
    const Natural lastIndex  = lineList.size() - 2;
    Natural previousPageNumber = Natural::maximumValue();

    for (Natural i = firstIndex; i <= lastIndex; i++) {
        const String line = lineList[i];
        const StringList partList = StringList::makeBySplit(line, "=");

        if (partList.size() == 2) {
            Natural pageNumber;
            String labelName;
            const String parameterName = STR::strip(partList[0]);
            String value = STR::strip(partList[1]);
            SoXEffectParameterMap::splitParameterName(parameterName,
                                                      labelName,
                                                      pageNumber);

            if (value.length() >= 2
                && STR::firstCharacter(value) == quoteCharacter
                && STR::lastCharacter(value) == quoteCharacter) {
                value = value.substr(1, value.length() - 2);
            }

            /* the recalculation is forced when the value is
               outside a page or the last value of the sequence */
            const Boolean isOnSamePage =
                (previousPageNumber == pageNumber);
            const Boolean recalculationIsForced =
              (i == lastIndex || !isOnSamePage);

            /* make sure that the value in parameter map does not
               match the new value */
            if (!parameterMap.isAllowedValue(parameterName, value)) {
                value = parameterMap.value(parameterName);
            }

            parameterMap.invalidateValue(parameterName);
            audioProcessor->setValue(parameterName, value,
                                     recalculationIsForced);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Reads current time and returns its value.
 *
 * @param[in] playHead  the JUCE audio play head
 * @return  current time (as a real value)
 */
static Real _readTime (juce::AudioPlayHead* playHead)
{
    Real currentTime;

    if (playHead == nullptr) {
        currentTime = 0.0;
    } else {
        currentTime =
            Real{*(playHead->getPosition()->getTimeInSeconds())};
    }

    return currentTime;
}

/*--------------------*/

/**
 * Returns value from interval <C>lowValue</C> to <C>highValue</C>
 * defined by unit interval factor <C>unitIntervalValue</C>.
 *
 * @param[in] unitIntervalValue  value between 0 and 1
 * @param[in] lowValue           lower bound value of target interval
 * @param[in] highValue          higher bound value of target interval
 * @return  scaled value in target interval
 */
static Integer _stretchToIntegerInterval (IN Real unitIntervalValue,
                                          IN Integer lowValue,
                                          IN Integer highValue)
{
    Assertion_pre(unitIntervalValue.isInInterval(Real::zero, Real::one),
                  STR::expand("value must be in unit interval- %1",
                              TOSTRING(unitIntervalValue)));
    const Real stretchedValue =
        _stretchToRealInterval(unitIntervalValue, lowValue, highValue);
    return (Integer) Real::round(stretchedValue);
}

/*--------------------*/

/**
 * Returns value from interval <C>lowValue</C> to <C>highValue</C>
 * defined by unit interval factor <C>unitIntervalValue</C>.
 *
 * @param[in] unitIntervalValue  value between 0 and 1
 * @param[in] lowValue           lower bound value of target interval
 * @param[in] highValue          higher bound value of target interval
 * @return  scaled value in target interval
 */
static Real _stretchToRealInterval (IN Real unitIntervalValue,
                                    IN Real lowValue,
                                    IN Real highValue)
{
    Assertion_pre(unitIntervalValue.isInInterval(Real::zero, Real::one),
                  STR::expand("value must be in unit interval- %1",
                              TOSTRING(unitIntervalValue)));
    return lowValue + unitIntervalValue * (highValue - lowValue);
}

/*--------------------*/

/**
 * Updates range of juce parameter object <C>parameterObject</C> named
 * <C>parameterName</C> using data taken from
 * <C>effectParameterMap</C>.
 *
 * @param[inout] parameterObject     juce audio parameter object
 * @param[in]    parameterName       name of audio parameter in map
 * @param[in]    effectParameterMap  map of all audio parameters to
 *                                   associated attributes
 */
static void _updateAudioProcessorParameterRange
                (INOUT juce::AudioProcessorParameter& parameterObject,
                 IN String& parameterName,
                 IN SoXEffectParameterMap& effectParameterMap)
{
    Logging_trace1(">>: %1", parameterName);

    const SoXEffectParameterKind kind =
        effectParameterMap.kind(parameterName);
    const String value = effectParameterMap.value(parameterName);

    const juce::String juceParameterName = juce::String(parameterName);
    const juce::ParameterID juceParameterID = juceParameterName;

    if (kind == SoXEffectParameterKind::enumKind) {
        Logging_traceError("combo box change is not supported");
        // juce::AudioParameterChoice& choiceParameterObject =
        //     dynamic_cast<juce::AudioParameterChoice&>(parameterObject);
        // StringList enumValueList;
        // effectParameterMap.valueRangeEnum(parameterName, enumValueList);
        // juce::StringArray juceChoiceList;
        // Natural parameterIndex = 0;
        // Boolean isFound = false;

        // for (const String& enumValue : enumValueList) {
        //     juceChoiceList.add(juce::String(enumValue));
        //     isFound = isFound || (value == enumValue);
        //     parameterIndex += (isFound ? 0 : 1);
        // }

        // /* workaround */
        // while (juceChoiceList.size() < 2) {
        //     juceChoiceList.add("x");
        // }
    } else if (kind == SoXEffectParameterKind::realKind
               || kind == SoXEffectParameterKind::intKind) {
        juce::AudioParameterFloat& floatParameterObject =
            dynamic_cast<juce::AudioParameterFloat&>(parameterObject);
        Real lowValue, highValue, delta;

        if (kind == SoXEffectParameterKind::realKind) {
            effectParameterMap.valueRangeReal(parameterName,
                                              lowValue, highValue, delta);
        } else {
            Integer lowV, highV, deltaV;
            effectParameterMap.valueRangeInt(parameterName,
                                             lowV, highV, deltaV);
            lowValue  = lowV;
            highValue = highV;
            delta     = deltaV;
        }
        
        juce::NormalisableRange<float>& range =
            floatParameterObject.range;
        range.start    = (float) lowValue;
        range.end      = (float) highValue;
        range.interval = (float) delta;
        const Real currentValue = STR::toReal(value);
        floatParameterObject = (float) currentValue;
    }

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Updates value of juce parameter object <C>parameterObject</C> named
 * <C>parameterName</C> to <C>value</C> using data taken from
 * <C>effectParameterMap</C>.
 *
 * @param[inout] parameterObject     juce audio parameter object
 * @param[in]    parameterName       name of audio parameter in map
 * @param[in]    effectParameterMap  map of all audio parameters to
 *                                   associated attributes
 * @param[in]    value               new value for parameter
 */
static void
_updateAudioProcessorParameterValue
        (INOUT juce::AudioProcessorParameter* parameterObject,
         IN String& parameterName,
         IN SoXEffectParameterMap& effectParameterMap,
         IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    const SoXEffectParameterKind kind =
        effectParameterMap.kind(parameterName);

    if (kind == SoXEffectParameterKind::realKind
        || kind == SoXEffectParameterKind::intKind) {
        Real realValue = STR::toReal(value);

        if (kind == SoXEffectParameterKind::intKind) {
            realValue = Real::round(realValue);
        }

        juce::AudioParameterFloat& numericParameterObject =
            TOREFERENCE<juce::AudioParameterFloat>(parameterObject);
        numericParameterObject = (float) realValue;
    } else if (kind == SoXEffectParameterKind::enumKind) {
        StringList enumValueList;
        effectParameterMap.valueRangeEnum(parameterName, enumValueList);
        const int valueIndex =
            (int) Integer::maximum(0,
                                   enumValueList.position(value));
        juce::AudioParameterChoice& choiceParameterObject =
            TOREFERENCE<juce::AudioParameterChoice>(parameterObject);
        choiceParameterObject = (int) valueIndex;
    }


    Logging_trace("<<");
}


/*--------------------*/

/**
 * Ensures that all audio parameter ranges correspond to the
 * underlying parameter map.
 *
 * @param[in] audioParameterList  list of audio parameter objects
 * @param[in] effectParameterMap  map with parameter data
 */
static void
_updateParameterRanges (IN _AudioProcessorParameterArray& audioParameterList,
                        IN SoXEffectParameterMap& effectParameterMap)
{
    Logging_trace(">>");

    for (juce::AudioProcessorParameter* parameterObject : audioParameterList) {
        String parameterName =
            parameterObject->getName(1000).toStdString();
        _updateAudioProcessorParameterRange(*parameterObject,
                                            parameterName,
                                            effectParameterMap);
    }
    
    Logging_trace("<<");
}

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXAudioProcessor::SoXAudioProcessor ()
     : juce::AudioProcessor(BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    Logging_trace(">>");
    _descriptor = new _SoXAudioProcessorDescriptor();
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);

    descriptor.listener.processor = this;

    Logging_trace("<<");
}

/*--------------------*/

SoXAudioProcessor::~SoXAudioProcessor ()
{
    Logging_trace(">>");
    _SoXAudioProcessorDescriptor* descriptor =
        (_SoXAudioProcessorDescriptor*) _descriptor;
    delete descriptor;
    Logging_trace("<<");
    Logging_finalize();
}

/*--------------------*/

juce::AudioProcessorEditor* SoXAudioProcessor::createEditor ()
{
    Logging_trace(">>");
    Logging_trace("<<");
    return new SoXAudioEditor(*this);
}

/*--------------------*/
/* property queries   */
/*--------------------*/

bool SoXAudioProcessor::supportsDoublePrecisionProcessing () const
{
    return false;
}

/*--------------------*/

bool SoXAudioProcessor::acceptsMidi () const
{
    return false;
}

/*--------------------*/

bool SoXAudioProcessor::producesMidi () const
{
    return false;
}

/*--------------------*/

bool SoXAudioProcessor::isMidiEffect () const
{
    return false;
}

/*--------------------*/

double SoXAudioProcessor::getTailLengthSeconds () const
{
    const _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    const SoXAudioEffect* effect = descriptor.effect;
    return (double) effect->tailLength();
}

/*--------------------*/

int SoXAudioProcessor::getNumPrograms ()
{
    return 1;
}

/*--------------------*/

int SoXAudioProcessor::getCurrentProgram ()
{
    return 0;
}

/*--------------------*/

void SoXAudioProcessor::setCurrentProgram (int)
{
    /* does not apply */
}

/*--------------------*/

const juce::String SoXAudioProcessor::getProgramName (int)
{
    return "default";
}

/*--------------------*/

void SoXAudioProcessor::changeProgramName (int, const juce::String&)
{
    /* does not apply */
}

/*--------------------*/

bool SoXAudioProcessor::hasEditor () const
{
    return true;
}

/*--------------------*/

const juce::String SoXAudioProcessor::getName () const
{
    return juce::String(name());
}

/*--------------------*/

String SoXAudioProcessor::name () const
{
    return String("???");
}

/*--------------------*/
/* persistence        */
/*--------------------*/

void SoXAudioProcessor::getStateInformation (OUT juce::MemoryBlock& destData)
{
    Logging_trace(">>");

    /* stores state of audio processor in <destData> */
    const String title = getName().toStdString();
    const String st = _convertMapToString(effectParameterMap(), title);
    const Natural sizeInBytes = st.size();
    destData.setSize((int) sizeInBytes);
    destData.copyFrom(st.c_str(), 0, (int) sizeInBytes);

    Logging_trace1("<<: %1", String_normalize(st));
}

/*--------------------*/

void SoXAudioProcessor::setStateInformation (const void* data,
                                             int sizeInBytes)
{
    Logging_trace(">>");

    const _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect* effect = descriptor.effect;

    /* restores state of audio processor from <data> */
    String st(static_cast<char*>(const_cast<void*>(data)),
              sizeInBytes);
    _readKeyValueMapString(this, effectParameterMap(), st);
    effect->setParameterValidity(true);

    Logging_trace2("<<: data = %1, processor = %2",
                   String_normalize(st), effect->toString());
}

/*--------------------*/
/* parameter map      */
/*--------------------*/

SoXEffectParameterMap& SoXAudioProcessor::effectParameterMap () const
{
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    const SoXAudioEffect* effect = descriptor.effect;
    return effect->effectParameterMap();
}

/*--------------------*/

void SoXAudioProcessor::setValue (IN String& parameterName,
                                  IN String& value,
                                  IN Boolean recalculationIsForced)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsForced = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsForced));

    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    const SoXEffectParameterMap& parameterMap = effectParameterMap();

    if (!parameterMap.contains(parameterName)) {
        Logging_trace1("--: bad parameter - %1", parameterName);
    } else {
        if (parameterMap.valueIsDifferent(parameterName, value)) {
            /* update effect and parameter map */
            SoXAudioEffect* effect = descriptor.effect;
            const SoXParameterValueChangeKind changeKind =
                effect->setValue(parameterName, value,
                                 recalculationIsForced);

            if (changeKind != SoXParameterValueChangeKind::noChange) {
                /* notify listeners */
                SoXParameterValueChangeKind parameterChange = 
                    SoXParameterValueChangeKind::parameterChange;

                if (changeKind == SoXParameterValueChangeKind::globalChange) {
                    _updateParameterRanges(getParameters(),
                                           effectParameterMap());
                }
            
                if (changeKind != parameterChange) {
                    _notifyObserversAboutChange(changeKind, parameterName);
                }

                _notifyObserversAboutChange(parameterChange, parameterName);
                
                /* update juce parameter object */
                Natural parameterIndex =
                    descriptor.parameterNameToIndexMap.at(parameterName);
                juce::AudioProcessorParameter* parameter =
                    getParameters()[(int) parameterIndex];
                _updateAudioProcessorParameterValue(parameter,
                                                    parameterName,
                                                    parameterMap,
                                                    value);
            }
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::setValues (IN Dictionary& dictionary)
{
    Logging_trace1(">>: %1", dictionary.toString());

    Natural count = dictionary.size();

    for (const auto& entry : dictionary) {
        const String parameterName = entry.first;
        const String value         = entry.second;
        const Boolean recalculationIsForced = (--count == 0);
        setValue(parameterName, value, recalculationIsForced);
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::_setAssociatedEffect (IN SoXAudioEffect* effect)
{
    Logging_trace(">>");

    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    descriptor.effect = const_cast<SoXAudioEffect*>(effect);

    const SoXEffectParameterMap& parameterMap = effectParameterMap();
    Logging_trace1("--: parameterMap = %1", parameterMap.toString());
    const StringList parameterNameList = parameterMap.parameterNameList();

    for (const String& parameterName : parameterNameList) {
        juce::AudioProcessorParameter* parameterObject =
            _constructParameterObject(parameterName, parameterMap);

        if (parameterObject != NULL) {
            addParameter(parameterObject);
            Natural parameterIndex = parameterObject->getParameterIndex();
            descriptor.parameterNameToIndexMap.set(parameterName,
                                                   parameterIndex);
            parameterObject->addListener(&descriptor.listener);
        }
    }

    descriptor.effect->setValueChangeHandler(this, _dispatchValueChange);

    Logging_trace("<<");
}

/*--------------------*/
/* observer mgmt      */
/*--------------------*/

void SoXAudioProcessor::registerObserver (INOUT Object observer)
{
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEditor* editor = static_cast<SoXAudioEditor*>(observer);
    descriptor.observerSet.add(editor);
}

/*--------------------*/

void SoXAudioProcessor::unregisterObserver (INOUT Object observer)
{
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEditor* editor = static_cast<SoXAudioEditor*>(observer);
    descriptor.observerSet.remove(editor);
}

/*--------------------*/

void
SoXAudioProcessor::_notifyObserversAboutChange
                       (IN SoXParameterValueChangeKind kind,
                        IN String& data)
{
    Logging_trace2(">>: kind = %1, data = %2",
                   SoXParameterValueChangeKind_toString(kind),
                   data);

    const _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    _SoXAudioEditorPtrSet observerSet = descriptor.observerSet;

    for (SoXAudioEditor* editor : observerSet) {
        if (editor != nullptr) {
            editor->notifyAboutChange(kind, data);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXAudioProcessor::prepareToPlay (double sampleRate, int)
{
    Logging_trace1(">>: sampleRate = %1",
                   TOSTRING(Real{sampleRate}));

    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect* effect = descriptor.effect;

    if (!effect->hasValidParameters()) {
        effect->setDefaultValues();
        effect->setParameterValidity(true);
    }

    effect->prepareToPlay(sampleRate);

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::releaseResources ()
{
    Logging_trace(">>");
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect* effect = descriptor.effect;
    effect->releaseResources();
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer&)
{
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect* effect = descriptor.effect;

    const Natural channelCount = getTotalNumInputChannels();
    const Natural outputChannelCount = getTotalNumOutputChannels();
    const Natural sampleCount{buffer.getNumSamples()};
    
    /* In case we have more outputs than inputs, this code clears any
       output channels that didn't contain input data */
    for (Natural i = channelCount;  i < outputChannelCount;  i++) {
        buffer.clear((int) i, 0, (int) sampleCount);
    }

    AudioSampleListVector audioSampleBuffer{};

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        const float* inputPtr = buffer.getReadPointer((int) channel);
        AudioSampleList sampleList{sampleCount};
        convertArray(sampleList.asArray(), inputPtr, sampleCount);
        audioSampleBuffer.append(sampleList);
    }

    const Real currentTimePosition = _readTime(getPlayHead());
    effect->processBlock(currentTimePosition, audioSampleBuffer);

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        float* outputPtr = buffer.getWritePointer((int) channel);
        AudioSampleList& sampleList = audioSampleBuffer[channel];
        convertArray(outputPtr, sampleList.asArray(), sampleCount);
    }
}
