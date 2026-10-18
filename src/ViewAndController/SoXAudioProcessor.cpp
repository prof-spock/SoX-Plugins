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

using Audio::AudioDataPointListVector;
using BaseTypes::Containers::Array_convert;
using BaseTypes::GenericTypes::GenericSet;
using SoXPlugins::Helpers::SoXEffectParameterKind;
using SoXPlugins::ViewAndController::SoXAudioEditor;

/** abbreviation for StringUtil */
using STR = BaseModules::StringUtil;

using BaseTypes::Primitives::String_normalize;

/*============================================================*/

/** a set of audio editor pointers */
typedef GenericSet<SoXAudioEditor*> _SoXAudioEditorPtrSet;

/** a mapping from string to natural */
typedef GenericMap<String, Natural> StringToNaturalMap;

/*--------------------*/
/*--------------------*/

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

/*============================================================*/

/*--------------------------*/
/* _AudioProcessorParameter */
/*--------------------------*/

/** a JUCE audio processor parameter */
typedef juce::AudioProcessorParameter _AudioProcessorParameter;

/** a JUCE array of audio processor parameter pointers */
typedef juce::Array<_AudioProcessorParameter*> _AudioProcessorParameterArray;

/*--------------------*/

/**
 * Updates range of juce parameter object <C>parameterObject</C> named
 * <C>parameterName</C> using data taken from
 * <C>parameterMap</C>.
 *
 * @param[inout] parameterObject  juce audio parameter object
 * @param[in]    parameterName    name of audio parameter in map
 * @param[in]    parameterMap     map of all audio parameters to
 *                                associated attributes
 */
static void _AudioProcessorParameter_updateRange
                (INOUT _AudioProcessorParameter& parameterObject,
                 IN String& parameterName,
                 IN SoXEffectParameterMap& parameterMap)
{
    Logging_trace1(">>: %1", parameterName);

    const SoXEffectParameterKind kind = parameterMap.kind(parameterName);
    const String value = parameterMap.value(parameterName);

    const juce::String juceParameterName = juce::String(parameterName);
    const juce::ParameterID juceParameterID = juceParameterName;

    if (kind == SoXEffectParameterKind::enumKind) {
        Logging_traceError("combo box change is not supported");
        /* TODO: check whether some combo box change is possible

           juce::AudioParameterChoice& choiceParameterObject =
           dynamic_cast<juce::AudioParameterChoice&>(parameterObject);
           StringList enumValueList;
           parameterMap.valueRangeEnum(parameterName, enumValueList);
           juce::StringArray juceChoiceList;
           Natural parameterIndex = 0;
           Boolean isFound = false;
           
           for (const String& enumValue : enumValueList) {
               juceChoiceList.add(juce::String(enumValue));
               isFound = isFound || (value == enumValue);
               parameterIndex += (isFound ? 0 : 1);
           }

           while (juceChoiceList.size() < 2) {
               juceChoiceList.add("x");
           }
        */
    } else if (kind == SoXEffectParameterKind::realKind
               || kind == SoXEffectParameterKind::intKind) {
        juce::AudioParameterFloat& floatParameterObject =
            dynamic_cast<juce::AudioParameterFloat&>(parameterObject);
        Real lowValue, highValue, delta;

        if (kind == SoXEffectParameterKind::realKind) {
            parameterMap.valueRangeReal(parameterName,
                                        lowValue, highValue, delta);
        } else {
            Integer lowV, highV, deltaV;
            parameterMap.valueRangeInt(parameterName,
                                       lowV, highV, deltaV);
            lowValue  = lowV;
            highValue = highV;
            delta     = deltaV;
        }
        
        juce::NormalisableRange<float>& range =
            floatParameterObject.range;
        range.start    = float(lowValue);
        range.end      = float(highValue);
        range.interval = float(delta);
        const Real currentValue = STR::toReal(value);
        floatParameterObject = float(currentValue);
    }

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Ensures that all audio parameter ranges correspond to the
 * underlying parameter map.
 *
 * @param[in] audioParameterList  list of audio parameter objects
 * @param[in] parameterMap        map with parameter data
 */
static void _AudioProcessorParameter_updateRanges
                (IN _AudioProcessorParameterArray& audioParameterList,
                 IN SoXEffectParameterMap& parameterMap)
{
    Logging_trace(">>");

    for (_AudioProcessorParameter* parameterObject : audioParameterList) {
        String parameterName =
            parameterObject->getName(1000).toStdString();
        _AudioProcessorParameter_updateRange(*parameterObject,
                                             parameterName,
                                             parameterMap);
    }
    
    Logging_trace("<<");
}

/*--------------------*/

/**
 * Updates value of juce parameter object <C>parameterObject</C> named
 * <C>parameterName</C> to <C>value</C> using data taken from
 * <C>parameterMap</C>.
 *
 * @param[inout] parameterObject  juce audio parameter object
 * @param[in]    parameterName    name of audio parameter in map
 * @param[in]    parameterMap     map of all audio parameters to
 *                                associated attributes
 * @param[in]    value            new value for parameter
 */
static void
_AudioProcessorParameter_updateValue
        (INOUT _AudioProcessorParameter* parameterObject,
         IN String& parameterName,
         IN SoXEffectParameterMap& parameterMap,
         IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    const SoXEffectParameterKind kind =
        parameterMap.kind(parameterName);

    if (kind == SoXEffectParameterKind::realKind
        || kind == SoXEffectParameterKind::intKind) {
        Real realValue = STR::toReal(value);

        if (kind == SoXEffectParameterKind::intKind) {
            realValue = Real::round(realValue);
        }

        juce::AudioParameterFloat& numericParameterObject =
            TOREFERENCE<juce::AudioParameterFloat>(parameterObject);
        numericParameterObject = float(realValue);
    } else if (kind == SoXEffectParameterKind::enumKind) {
        StringList enumValueList;
        parameterMap.valueRangeEnum(parameterName, enumValueList);
        const int valueIndex =
            int(Integer::maximum(0, enumValueList.position(value)));
        juce::AudioParameterChoice& choiceParameterObject =
            TOREFERENCE<juce::AudioParameterChoice>(parameterObject);
        choiceParameterObject = int(valueIndex);
    }


    Logging_trace("<<");
}

/*--------------------*/

/**
 * Updates values of audio parameters in <C>audioParameterList</C> by
 * key-value map <C>dictionary</C> underlying parameter map.
 *
 * @param[in] audioParameterList  list of audio parameter objects
 * @param[in] dictionary          map with parameter values
 * @param[in] parameterMap        map of all audio parameters to
 *                                associated attributes
 */
static void _AudioProcessorParameter_updateValues
                (IN _AudioProcessorParameterArray& audioParameterList,
                 IN Dictionary& dictionary,
                 IN SoXEffectParameterMap& parameterMap)
{
    Logging_trace1(">>: %1", dictionary.toString());

    for (_AudioProcessorParameter* parameterObject : audioParameterList) {
        const String parameterName =
            parameterObject->getName(1000).toStdString();

        if (dictionary.contains(parameterName)) {
            String value = dictionary.at(parameterName);
            _AudioProcessorParameter_updateValue(parameterObject,
                                                 parameterName,
                                                 parameterMap,
                                                 value);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Constructs juce parameter object from <C>parameterName</C> with
 * data taken from <C>parameterMap</C>.
 *
 * @param[in] parameterName  name of audio parameter in map
 * @param[in] parameterMap   map of all audio parameters to
 *                           associated attributes
 * @return  juce audio parameter object
 */
static _AudioProcessorParameter*
_AudioProcessorParameter_create (IN String& parameterName,
                                 IN SoXEffectParameterMap& parameterMap)
{
    Logging_trace1(">>: %1", parameterName);

    const SoXEffectParameterKind kind =
        parameterMap.kind(parameterName);
    const String value = parameterMap.value(parameterName);

    juce::NormalisableRange<float> range;
    const juce::String juceParameterName = juce::String(parameterName);
    const juce::ParameterID juceParameterID = juceParameterName;
    _AudioProcessorParameter* parameterObject = nullptr;

    if (kind == SoXEffectParameterKind::realKind
        || kind == SoXEffectParameterKind::intKind) {
        parameterObject =
            new juce::AudioParameterFloat(juceParameterID,
                                          juceParameterName,
                                          range, 1.0);
    } else if (kind == SoXEffectParameterKind::enumKind) {
        StringList enumValueList;
        parameterMap.valueRangeEnum(parameterName, enumValueList);
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
            .withMeta((bool) isMetaParameter);
        parameterObject =
            new juce::AudioParameterChoice(juceParameterID,
                                           juceParameterName,
                                           juceChoiceList,
                                           int(parameterIndex),
                                           attributes);
    }

    const Boolean parameterObjectIsSet = (parameterObject != nullptr);

    if (parameterObjectIsSet) {
        _AudioProcessorParameter_updateRange(*parameterObject,
                                            parameterName,
                                            parameterMap);
    }

    Logging_trace1("<<: %1", (parameterObjectIsSet ? "OKAY" : "NULL"));
    return parameterObject;
}

/*============================================================*/

namespace SoXPlugins::ViewAndController {

    /** a listener for effect parameters */
    struct _EffectParameterListener
        : juce::AudioProcessorParameter::Listener {

        /** the associated processor for this listener */
        SoXAudioProcessor* audioProcessor;

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
        SoXAudioEffect* effect{nullptr};

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
    /* no gesture support for now */
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

    /* find associated JUCE parameter object */
    const _AudioProcessorParameter* parameterObject =
        audioProcessor->getParameters()[int(parameterIndex)];
    const String parameterName =
        parameterObject->getName(1000).toStdString();
    Logging_trace1("--: parameterName = %1", parameterName);
    const SoXEffectParameterMap& parameterMap =
        audioProcessor->effectParameterMap();

    if (!parameterMap.contains(parameterName)) {
        Logging_trace("--: parameterName is unknown");
    } else {
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
            Integer elementCount{int(enumValueList.size())};
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
            audioProcessor->setValue(parameterName, value);
        }
    }

    Logging_trace("<<");
}

/*============================================================*/

/*--------------------*/
/* internal routines  */
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
    String result = title + STR::newlineString;

    for (Natural i = 0;  i < parameterNameList.size();  i++) {
        const String parameterName = parameterNameList[i];
        const SoXEffectParameterKind kind =
            parameterMap.kind(parameterName);
        String parameterValue = parameterMap.value(parameterName);

        if (kind == SoXEffectParameterKind::enumKind) {
            STR::prepend(parameterValue, STR::doubleQuoteCharacter);
            STR::append(parameterValue,  STR::doubleQuoteCharacter);
        }

        /* write each entry as a line */
        result +=
            (parameterName + " = " + parameterValue + STR::newlineString);
    }

    Logging_trace1("<<: %1", String_normalize(result));
    return result;
}

/*--------------------*/

/**
 * Sets all parameter with keys in <C>dictionary</C> to associated
 * values for audio processor given as <C>object</C> and notifies
 * observers
 *
 * @param[in] object           descriptor object receiving notification
 * @param[in] dictionary       map from parameter names to new values
 * @param[in] valueChangeKind  maximum kind of value change
 */
static void _dispatchValueChange (INOUT Object object,
                                  IN Dictionary& dictionary,
                                  IN SoXValueChangeKind valueChangeKind)
{
    Logging_trace2(">>: dictionary = %1, changeKind = %2",
                   dictionary.toString(),
                   SoXValueChangeKind_toString(valueChangeKind));

    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(object);
    SoXAudioProcessor& audioProcessor =
        TOREFERENCE<SoXAudioProcessor>(descriptor.listener.audioProcessor);
    const SoXEffectParameterMap& parameterMap =
        audioProcessor.effectParameterMap();
    const _AudioProcessorParameterArray& apParameterList =
        audioProcessor.getParameters();

    if (valueChangeKind == SoXValueChangeKind::globalChange) {
        _AudioProcessorParameter_updateRanges(apParameterList,
                                              parameterMap);
    }
    
    _AudioProcessorParameter_updateValues (apParameterList,
                                           dictionary, parameterMap);

    /* notify all observers about change */
    _SoXAudioEditorPtrSet observerSet = descriptor.observerSet;

    for (SoXAudioEditor* editor : observerSet) {
        if (editor != nullptr) {
            editor->notifyAboutChange(dictionary, valueChangeKind);
        }
    }

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
    const StringList lineList =
        StringList::makeBySplit(st, STR::newlineString);

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
            String parameterName = STR::strip(partList[0]);
            String value = STR::strip(partList[1]);
            SoXEffectParameterMap::splitParameterName(parameterName,
                                                      labelName,
                                                      pageNumber);

            if (value.length() >= 2
                && STR::firstCharacter(value) == STR::doubleQuoteCharacter
                && STR::lastCharacter(value) == STR::doubleQuoteCharacter) {
                value = value.substr(1, value.length() - 2);
            }

            /* the recalculation is forced when the value is
               outside a page or the last value of the sequence */
            const Boolean isOnSamePage =
                (previousPageNumber == pageNumber);
            const Boolean recalculationIsForced =
              (i == lastIndex || !isOnSamePage);

            /* map to the canonical name */
            String canonicalParameterName;
            
            if (parameterMap.isAcceptableName(parameterName,
                                              canonicalParameterName)) {
                parameterName = canonicalParameterName;

                /* make sure that the value in parameter map does not
                   match the new value */
                if (!parameterMap.isAllowedValue(parameterName, value)) {
                    value = parameterMap.value(parameterName);
                }

                parameterMap.invalidateValue(parameterName);
                audioProcessor->setValue(parameterName, value);
            }
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
static Real _readTime (IN juce::AudioPlayHead* playHead)
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
    return Integer(Real::round(stretchedValue));
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

    descriptor.listener.audioProcessor = this;

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
    const SoXAudioEffect& effect =
        TOREFERENCE<SoXAudioEffect>(descriptor.effect);
    return double(effect.tailLength());
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
    destData.setSize(int(sizeInBytes));
    destData.copyFrom(st.c_str(), 0, int(sizeInBytes));

    Logging_trace1("<<: %1", String_normalize(st));
}

/*--------------------*/

void SoXAudioProcessor::setStateInformation (const void* data,
                                             int sizeInBytes)
{
    Logging_trace(">>");

    const _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect& effect =
        TOREFERENCE<SoXAudioEffect>(descriptor.effect);

    /* restores state of audio processor from <data> */
    String st(static_cast<char*>(const_cast<void*>(data)),
              sizeInBytes);
    _readKeyValueMapString(this, effectParameterMap(), st);
    effect.recalculate();
    effect.setParameterValidity(true);

    Logging_trace2("<<: data = %1, processor = %2",
                   String_normalize(st), effect.toString());
}

/*--------------------*/
/* parameter map      */
/*--------------------*/

SoXEffectParameterMap& SoXAudioProcessor::effectParameterMap () const
{
    const _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    const SoXAudioEffect& effect =
        TOREFERENCE<SoXAudioEffect>(descriptor.effect);
    return effect.effectParameterMap();
}

/*--------------------*/

void SoXAudioProcessor::setValue (IN String& parameterName,
                                  IN String& value)
{
    Logging_trace2(">>: parameterName = %1, value = %2",
                   parameterName, value);

    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    const SoXEffectParameterMap& parameterMap = effectParameterMap();

    if (!parameterMap.contains(parameterName)) {
        Logging_trace1("--: bad parameter - %1", parameterName);
    } else {
        /* update effect and parameter map */
        SoXAudioEffect& effect =
            TOREFERENCE<SoXAudioEffect>(descriptor.effect);
        effect.setValue(parameterName, value);
        effect.recalculate();
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
        _AudioProcessorParameter* parameterObject =
            _AudioProcessorParameter_create(parameterName, parameterMap);

        if (parameterObject != nullptr) {
            addParameter(parameterObject);
            Natural parameterIndex = parameterObject->getParameterIndex();
            descriptor.parameterNameToIndexMap.set(parameterName,
                                                   parameterIndex);
            parameterObject->addListener(&descriptor.listener);
        }
    }

    descriptor.effect->setValueChangeHandler(&descriptor,
                                             _dispatchValueChange);

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
/* event handling     */
/*--------------------*/

void SoXAudioProcessor::prepareToPlay (double dataPointRate, int)
{
    Logging_trace1(">>: dataPointRate = %1",
                   TOSTRING(Real{dataPointRate}));

    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect& effect =
        TOREFERENCE<SoXAudioEffect>(descriptor.effect);

    if (!effect.hasValidParameters()) {
        effect.setDefaultValues();
        effect.setParameterValidity(true);
    }

    effect.prepareToPlay(dataPointRate);

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::releaseResources ()
{
    Logging_trace(">>");
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect& effect =
        TOREFERENCE<SoXAudioEffect>(descriptor.effect);
    effect.releaseResources();
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer&)
{
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect& effect =
        TOREFERENCE<SoXAudioEffect>(descriptor.effect);
    const Real dataPointRate = getSampleRate();

    /* check whether data point rate has changed; if yes, assume that
       playback has started */
    if (dataPointRate != effect.dataPointRate()) {
        effect.prepareToPlay(dataPointRate);
    }
    
    const Natural channelCount = getTotalNumInputChannels();
    const Natural outputChannelCount = getTotalNumOutputChannels();
    const Natural dataPointCount{buffer.getNumSamples()};
    
    /* in case we have more outputs than inputs, this code clears any
       output channels that didn't contain input data */
    for (Natural i = channelCount;  i < outputChannelCount;  i++) {
        buffer.clear(int(i), 0, int(dataPointCount));
    }

    AudioDataPointListVector audioDataPointBuffer{};

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        const float* inputPtr = buffer.getReadPointer(int(channel));
        AudioDataPointList dataPointList{dataPointCount};
        Array_convert(dataPointList.asArray(), inputPtr, dataPointCount);
        audioDataPointBuffer.append(dataPointList);
    }

    const Real currentTimePosition = _readTime(getPlayHead());
    effect.processBlock(currentTimePosition, audioDataPointBuffer);

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        float* outputPtr = buffer.getWritePointer(int(channel));
        AudioDataPointList& dataPointList = audioDataPointBuffer[channel];
        Array_convert(outputPtr, dataPointList.asArray(), dataPointCount);
    }
}
