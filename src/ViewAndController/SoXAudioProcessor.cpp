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

/** abbreviated form of function name */
#define expand StringUtil::expand
/** abbreviated form of function name */
#define replace StringUtil::replace

/** a set of audio editor pointers */
typedef GenericSet<SoXAudioEditor*> _SoXAudioEditorPtrSet;

/** a mapping from string to natural */
typedef GenericMap<String, Natural> StringToNaturalMap;

/*============================================================*/

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
                  expand("value must be in unit interval- %1",
                         TOSTRING(unitIntervalValue)));
    return lowValue + unitIntervalValue * (highValue - lowValue);
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
                  expand("value must be in unit interval- %1",
                         TOSTRING(unitIntervalValue)));
    const Real stretchedValue =
        _stretchToRealInterval(unitIntervalValue, lowValue, highValue);
    return (Integer) Real::round(stretchedValue);
}

/*============================================================*/

namespace SoXPlugins::ViewAndController {

    /** the double quote character */
    static const char quoteCharacter = '"';

    /** number of decimal places for reals in serialized form */
    static const Natural decimalPlaceCount = 5;

    /*--------------------*/

    /** a listener for effect parameters */
    struct _EffectParameterListener
        : juce::AudioProcessorParameter::Listener {

        /** the associated processor for this listener */
        SoXAudioProcessor* processor;

        /*--------------------*/

        void parameterValueChanged (int paramIndex,
                                    float fValue) override;

        /*--------------------*/

        void parameterGestureChanged (int, bool) override;

    };

    /*--------------------*/

    void
    _EffectParameterListener::parameterValueChanged (int paramIndex,
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

    /*--------------------*/

    void _EffectParameterListener::parameterGestureChanged (int, bool)
    {
    }

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

        const juce::String juceParameterName = juce::String(parameterName);
        const juce::ParameterID juceParameterID = juceParameterName;
        juce::AudioProcessorParameter* result = NULL;

        if (kind == SoXEffectParameterKind::realKind) {
            Real lowValue, highValue, delta;
            effectParameterMap.valueRangeReal(parameterName,
                                             lowValue, highValue, delta);
            juce::NormalisableRange<float> range((float) lowValue,
                                                 (float) highValue,
                                                 (float) delta);
            const Real defaultValue = StringUtil::toReal(value);
            result =
                new juce::AudioParameterFloat(juceParameterID,
                                              juceParameterName,
                                              range,
                                              (float) defaultValue, "");
        } else if (kind == SoXEffectParameterKind::intKind) {
            Integer lowValue, highValue, delta;
            effectParameterMap.valueRangeInt(parameterName,
                                            lowValue, highValue, delta);
            const Integer defaultValue = StringUtil::toInteger(value);
            result =
                new juce::AudioParameterInt(juceParameterID,
                                            juceParameterName,
                                            (int) lowValue, (int) highValue,
                                            (int) defaultValue);
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

            result =
                new juce::AudioParameterChoice(juceParameterID,
                                               juceParameterName,
                                               juceChoiceList,
                                               (int) parameterIndex);
        }

        Logging_trace1("<<: %1", (result == NULL ? "NULL" : "OKAY"));
        return result;
    }

    /*--------------------*/

    /**
     * Replaces all newline in <C>st</C> by another character and returns
     * result.
     *
     * @param[in] st  string to be normalized
     * @return string with newlines removed
     */
    static String _normalize (IN String& st)
    {
        String result = st;
        replace(result, "\n", "§");
        return result;
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
                parameterValue =
                    quoteCharacter + parameterValue + quoteCharacter;
            } else if (kind == SoXEffectParameterKind::realKind) {
                const Real r = StringUtil::toReal(parameterValue);
                parameterValue = r.toString();
            }

            // write each entry as a line
            result += (parameterName + " = " + parameterValue + "\n");
        }

        Logging_trace1("<<: %1", _normalize(result));
        return result;
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
    static void
    _readKeyValueMapString (INOUT SoXAudioProcessor* audioProcessor,
                            INOUT SoXEffectParameterMap& parameterMap,
                            IN String& st)
    {
        Logging_trace1(">>: st = %1", st);

        // read the data as written by the string conversion
        const StringList lineList = StringList::makeBySplit(st, "\n");

        // throw away empty last line and title => first index is 1, last
        // index is lineCount - 1
        const Natural firstIndex = 1;
        const Natural lastIndex  = lineList.size() - 2;
        Natural previousPageNumber = Natural::maximumValue();

        for (Natural i = firstIndex; i <= lastIndex; i++) {
            const String line = lineList[i];
            const StringList partList = StringList::makeBySplit(line, "=");

            if (partList.size() == 2) {
                Natural pageNumber;
                String labelName;
                const String parameterName = StringUtil::strip(partList[0]);
                String value = StringUtil::strip(partList[1]);
                SoXEffectParameterMap::splitParameterName(parameterName,
                                                         labelName,
                                                         pageNumber);

                if (value.length() >= 2
                    && value[0] == quoteCharacter
                    && StringUtil::lastChar(value) == quoteCharacter) {
                    value = value.substr(1, value.length() - 2);
                }

                // the recalculation is forced when the value is
                // outside a page or the last value of the sequence
                const Boolean isOnSamePage =
                    (previousPageNumber == pageNumber);
                const Boolean recalculationIsForced =
                  (i == lastIndex || !isOnSamePage);

                // make sure that the value in parameter map does not
                // match the new value
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
    static
    Real _readTime (juce::AudioPlayHead* playHead)
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
     * Update juce parameter object <C>parameter</C> named
     * <C>parameterName</C> to <C>value</C> using data taken from
     * <C>effectParameterMap</C>.
     *
     * @param[inout] parameter           juce audio parameter object
     * @param[in]    parameterName       name of audio parameter in map
     * @param[in]    effectParameterMap  map of all audio parameters to
     *                                   associated attributes
     * @param[in]    value               new value for parameter
     */
    static void
    _updateAudioProcessorParameter
            (INOUT juce::AudioProcessorParameter* parameter,
             IN String& parameterName,
             IN SoXEffectParameterMap& effectParameterMap,
             IN String& value)
    {
        Logging_trace2(">>: parameterName = %1, value = %2",
                       parameterName, value);

        const SoXEffectParameterKind kind =
            effectParameterMap.kind(parameterName);

        if (kind == SoXEffectParameterKind::realKind) {
            const float realValue = (float) StringUtil::toReal(value);
            juce::AudioParameterFloat& fParameter =
                TOREFERENCE<juce::AudioParameterFloat>(parameter);
            fParameter = realValue;
        } else if (kind == SoXEffectParameterKind::intKind) {
            const int intValue = (int) StringUtil::toInteger(value);
            juce::AudioParameterInt& iParameter =
                TOREFERENCE<juce::AudioParameterInt>(parameter);
            iParameter = intValue;
        } else if (kind == SoXEffectParameterKind::enumKind) {
            StringList enumValueList;
            effectParameterMap.valueRangeEnum(parameterName, enumValueList);
            int valueIndex =
                (int) Integer::maximum(0,
                                       enumValueList.position(value));
            juce::AudioParameterChoice& cParameter =
                TOREFERENCE<juce::AudioParameterChoice>(parameter);
            cParameter = (int) valueIndex;
        }

        
        Logging_trace("<<");
    }
    
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
    return 0.0;
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
    // does not apply
}

/*--------------------*/

const juce::String SoXAudioProcessor::getProgramName (int)
{
    return "default";
}

/*--------------------*/

void SoXAudioProcessor::changeProgramName (int, const juce::String&)
{
    // does not apply
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

    // stores state of audio processor in <destData>
    const String title = getName().toStdString();
    const String st = _convertMapToString(effectParameterMap(), title);
    const Natural sizeInBytes = st.size();
    destData.setSize((int) sizeInBytes);
    destData.copyFrom(st.c_str(), 0, (int) sizeInBytes);

    Logging_trace1("<<: %1", _normalize(st));
}

/*--------------------*/

void SoXAudioProcessor::setStateInformation (const void* data,
                                             int sizeInBytes)
{
    Logging_trace(">>");

    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect* effect = descriptor.effect;

    // restores state of audio processor from <data>
    String st((char *) data, sizeInBytes);
    _readKeyValueMapString(this, effectParameterMap(), st);
    effect->setParameterValidity(true);

    Logging_trace2("<<: data = %1, processor = %2",
                   _normalize(st), effect->toString());
}

/*--------------------*/
/* parameter map      */
/*--------------------*/

SoXEffectParameterMap& SoXAudioProcessor::effectParameterMap () const
{
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEffect* effect = descriptor.effect;
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
        const String oldValue = parameterMap.value(parameterName);

        if (value != oldValue) {
            // update effect and parameter map
            SoXAudioEffect* effect = descriptor.effect;
            const SoXParameterValueChangeKind changeKind =
                effect->setValue(parameterName, value,
                                 recalculationIsForced);

            // notify listeners
            SoXParameterValueChangeKind parameterChange = 
                SoXParameterValueChangeKind::parameterChange;
        
            if (changeKind != parameterChange) {
                _notifyObserversAboutChange(changeKind, parameterName);
            }

            _notifyObserversAboutChange(parameterChange, parameterName);

            // update juce parameter object
            Natural parameterIndex =
                descriptor.parameterNameToIndexMap.at(parameterName);
            juce::AudioProcessorParameter* parameter =
                getParameters()[(int) parameterIndex];
            _updateAudioProcessorParameter(parameter,
                                           parameterName,
                                           parameterMap,
                                           value);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::setValues (IN Dictionary& dictionary)
{
    Logging_trace1(">>: %1", dictionary.toString());

    Natural count = dictionary.size();

    for (auto& entry : dictionary) {
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
    descriptor.effect = (SoXAudioEffect*) effect;

    const SoXEffectParameterMap& parameterMap = effectParameterMap();
    Logging_trace1("--: parameterMap = %1", parameterMap.toString());
    const StringList parameterNameList = parameterMap.parameterNameList();

    for (const String& parameterName : parameterNameList) {
        juce::AudioProcessorParameter* parameter =
            _constructParameterObject(parameterName, parameterMap);

        if (parameter != NULL) {
            addParameter(parameter);
            Natural parameterIndex = parameter->getParameterIndex();
            descriptor.parameterNameToIndexMap.set(parameterName,
                                                   parameterIndex);
            parameter->addListener(&descriptor.listener);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/
/* observer mgmt      */
/*--------------------*/

void SoXAudioProcessor::registerObserver (INOUT Object observer)
{
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEditor* editor = (SoXAudioEditor*) observer;
    descriptor.observerSet.add(editor);
}

/*--------------------*/

void SoXAudioProcessor::unregisterObserver (INOUT Object observer)
{
    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    SoXAudioEditor* editor = (SoXAudioEditor*) observer;
    descriptor.observerSet.remove(editor);
}

/*--------------------*/

void
SoXAudioProcessor::_notifyObserversAboutChange
                       (IN SoXParameterValueChangeKind kind,
                        IN String& parameterName)
{
    Logging_trace2(">>: kind = %1, parameterName = %2",
                   SoXParameterValueChangeKind_toString(kind),
                   parameterName);

    _SoXAudioProcessorDescriptor& descriptor =
        TOREFERENCE<_SoXAudioProcessorDescriptor>(_descriptor);
    _SoXAudioEditorPtrSet observerSet = descriptor.observerSet;

    for (SoXAudioEditor* editor : observerSet) {
        if (editor != nullptr) {
            editor->notifyAboutChange(kind, parameterName);
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
    const Natural sampleCount = (Natural) buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any
    // output channels that didn't contain input data
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
