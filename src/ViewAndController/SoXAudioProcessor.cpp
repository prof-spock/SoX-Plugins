/**
 * @file
 * The <C>SoXAudioProcessor</C> body implements an audio effect
 * wrapper for a plugin responsible for the communication to the
 * enclosing plugin host.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#include "JuceLibrary.h"
#include "Natural.h"
#include "SoXAudioEditor.h"
#include "SoXAudioProcessor.h"
#include "StringUtil.h"
#include "Logging.h"

using SoXPlugins::BaseTypes::Primitives::Integer;
using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::CommonAudio::SoXAudioParameterKind;
using SoXPlugins::CommonAudio::SoXAudioParameterMap;
using SoXPlugins::CommonAudio::SoXAudioSampleBuffer;
using SoXPlugins::ViewAndController::SoXAudioEditor;
using SoXPlugins::ViewAndController::_SoXAudioEditorPtrSet;
using SoXPlugins::ViewAndController::SoXAudioProcessor;

namespace StringUtil = SoXPlugins::BaseTypes::StringUtil;
using StringUtil::expand;
using StringUtil::replace;

/*============================================================*/

namespace SoXPlugins::ViewAndController {

    /** the double quote character */
    static const char quoteCharacter = '"';

    /** number of decimal places for reals in serialized form */
    static const Natural decimalPlaceCount = 4;

    /*--------------------*/
    /* internal routines  */
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
    static String _convertMapToString (IN SoXAudioParameterMap& parameterMap,
                                       IN String& title)
    {
        Logging_trace2(">>: map = %1, title = %2",
                       parameterMap.toString(), title);

        const StringList parameterNameList = parameterMap.parameterNameList();
        String result = title + "\n";

        for (Natural i = 0;  i < parameterNameList.size();  i++) {
            const String parameterName = parameterNameList[i];
            const SoXAudioParameterKind kind =
                parameterMap.kind(parameterName);
            String parameterValue = parameterMap.value(parameterName);

            if (kind == SoXAudioParameterKind::enumKind) {
                parameterValue =
                    quoteCharacter + parameterValue + quoteCharacter;
            } else if (kind == SoXAudioParameterKind::realKind) {
                // round to <decimalPlaceCount> decimal places
                const Real r = Real::round(StringUtil::toReal(parameterValue),
                                           decimalPlaceCount);
                parameterValue = TOSTRING(r);
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
                            INOUT SoXAudioParameterMap& parameterMap,
                            IN String& st)
    {
        Logging_trace1(">>: st = %1", st);

        // read the data as written by the string conversion
        const StringList lineList = StringList::makeBySplit(st, "\n");

        // throw away empty last line and title => first index is 1, last
        // index is lineCount - 1
        const Natural firstIndex = 1;
        const Natural lastIndex  = lineList.size() - 2;

        for (Natural i = firstIndex; i <= lastIndex; i++) {
            const String line = lineList[i];
            const StringList partList = StringList::makeBySplit(line, "=");

            if (partList.size() == 2) {
                const String parameterName = StringUtil::strip(partList[0]);
                String value = StringUtil::strip(partList[1]);

                if (value.length() >= 2
                    && value[0] == quoteCharacter
                    && StringUtil::lastChar(value) == quoteCharacter) {
                    value = value.substr(1, value.length() - 2);
                }

                // make sure that the value in parameter map does not
                // match the new value
                const bool recalculationIsSuppressed = (i < lastIndex);

                if (!parameterMap.isAllowedValue(parameterName, value)) {
                    value = parameterMap.value(parameterName);
                }

                parameterMap.invalidateValue(parameterName);
                audioProcessor->setValue(parameterName, value,
                                         recalculationIsSuppressed);
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
            juce::AudioPlayHead::CurrentPositionInfo positionInfo;
            playHead->getCurrentPosition(positionInfo);
            currentTime = positionInfo.timeInSeconds;
        }

        return currentTime;
    }

};

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

SoXAudioProcessor::SoXAudioProcessor ()
     : juce::AudioProcessor(BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
       _effect{NULL}
{
    Logging_trace(">>");
    Logging_trace("<<");
}

/*--------------------*/

SoXAudioProcessor::~SoXAudioProcessor ()
{
    Logging_trace(">>");
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
    return true;
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
    const String st = _convertMapToString(audioParameterMap(), title);
    const Natural sizeInBytes = st.size();
    destData.setSize((int) sizeInBytes);
    destData.copyFrom(st.c_str(), 0, (int) sizeInBytes);

    Logging_trace1("<<: %1", _normalize(st));
}

/*--------------------*/

void SoXAudioProcessor::setStateInformation (IN void* data,
                                             int sizeInBytes)
{
    Logging_trace(">>");

    // restores state of audio processor from <data>
    String st((char *) data, sizeInBytes);
    _readKeyValueMapString(this, audioParameterMap(), st);
    _effect->setParameterValidity(true);

    Logging_trace2("<<: data = %1, processor = %2",
                   _normalize(st), _effect->toString());
}

/*--------------------*/
/* parameter map      */
/*--------------------*/

SoXAudioParameterMap& SoXAudioProcessor::audioParameterMap () const
{
    return _effect->audioParameterMap();
}

/*--------------------*/

void SoXAudioProcessor::setValue (IN String& parameterName,
                                  IN String& value,
                                  IN bool recalculationIsSuppressed)
{
    Logging_trace3(">>: parameterName = %1, value = %2,"
                   " recalcIsSuppressed = %3",
                   parameterName, value,
                   TOSTRING(recalculationIsSuppressed));

    const SoXAudioValueChangeKind changeKind =
        _effect->setValue(parameterName, value, recalculationIsSuppressed);

    if (changeKind != SoXAudioValueChangeKind::parameterChange) {
        _notifyObserversAboutChange(changeKind, parameterName);
    }

    _notifyObserversAboutChange(SoXAudioValueChangeKind::parameterChange,
                                parameterName);

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
        const bool recalculationIsSuppressed = (--count > 0);
        setValue(parameterName, value, recalculationIsSuppressed);
    }

    Logging_trace("<<");
}

/*--------------------*/
/* observer mgmt      */
/*--------------------*/

void SoXAudioProcessor::registerObserver (INOUT SoXAudioEditor* observer)
{
    _observerSet.insert(observer);
}

/*--------------------*/

void SoXAudioProcessor::unregisterObserver (INOUT SoXAudioEditor* observer)
{
    _observerSet.erase(observer);
}

/*--------------------*/

void
SoXAudioProcessor::_notifyObserversAboutChange
                       (IN SoXAudioValueChangeKind kind,
                        IN String& parameterName)
{
    Logging_trace2(">>: kind = %1, parameterName = %2",
                   SoXAudioValueChangeKind_toString(kind), parameterName);

    for (SoXAudioEditor* editor : _observerSet) {
        if (editor != nullptr) {
            editor->notifyAboutChange(kind, parameterName);
        }
    }

    Logging_trace("<<");
}

/*--------------------*/
/* event handling     */
/*--------------------*/

void SoXAudioProcessor::prepareToPlay (IN double sampleRate, IN int)
{
    Logging_trace1(">>: sampleRate = %1", TOSTRING(sampleRate));

    if (!_effect->hasValidParameters()) {
        _effect->setDefaultValues();
        _effect->setParameterValidity(true);
    }

    _effect->prepareToPlay(sampleRate);

    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::releaseResources ()
{
    Logging_trace(">>");
    _effect->releaseResources();
    Logging_trace("<<");
}

/*--------------------*/

void SoXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer& midiMessages)
{
    // processes a block of float samples by internally
    // calling the double routine
    juce::AudioBuffer<double> sampleBuffer;
    sampleBuffer.makeCopyOf(buffer);
    processBlock(sampleBuffer, midiMessages);
    buffer.makeCopyOf(sampleBuffer);
}

/*--------------------*/

void SoXAudioProcessor::processBlock (juce::AudioBuffer<double>& buffer,
                                      juce::MidiBuffer&)
{
    const Natural channelCount = getTotalNumInputChannels();
    const Natural outputChannelCount = getTotalNumOutputChannels();
    const Natural sampleCount = (Natural) buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any
    // output channels that didn't contain input data
    for (Natural i = channelCount;  i < outputChannelCount;  i++) {
        buffer.clear((int) i, 0, (int) sampleCount);
    }

    SoXAudioSampleBuffer audioSampleBuffer{};

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        const double* inputPtr  =
            buffer.getReadPointer((int) channel);
        SoXAudioSampleList sampleList{sampleCount};

        for (SoXAudioSample& audioSample : sampleList) {
            audioSample = *inputPtr++;
        }

        audioSampleBuffer.append(sampleList);
    }

    const Real currentTimePosition = _readTime(getPlayHead());
    _effect->processBlock(currentTimePosition, audioSampleBuffer);

    for (Natural channel = 0;  channel < channelCount;  channel++) {
        SoXAudioSample* outputPtr =
            (Real*) buffer.getWritePointer((int) channel);
        SoXAudioSampleList& sampleList = audioSampleBuffer[channel];

        for (SoXAudioSample& audioSample : sampleList) {
            *outputPtr++ = audioSample;
        }
    }
}
