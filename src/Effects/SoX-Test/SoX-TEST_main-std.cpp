/**
 * @file
 * The <C>SoX-TEST</C> module implements a simple command-line test
 * program for the more complex SoX effects <B>reverb</B>,
 * <B>phaser/tremolo</B> and <B>compander</C>; there is no GUI
 * involved, only the engines are checked with standardized
 * parameters.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include <iostream>
#include <fstream>

#include "Logging.h"
#include "SoXCompander_AudioEffect.h"
#include "SoXFilter_AudioEffect.h"
#include "SoXPhaserAndTremolo_AudioEffect.h"
#include "SoXReverb_AudioEffect.h"

/*--------------------*/

using std::cout;

using Audio::AudioSample;
using SoXPlugins::Effects::SoXCompander::SoXCompander_AudioEffect;
using SoXPlugins::Effects::SoXFilter::SoXFilter_AudioEffect;
using SoXPlugins::Effects::SoXPhaserAndTremolo
         ::SoXPhaserAndTremolo_AudioEffect;
using SoXPlugins::Effects::SoXReverb::SoXReverb_AudioEffect;

/*====================*/

/*-----------*/
/* CONSTANTS */
/*-----------*/

const Natural _blocksPerSecond = 100;
const Natural _channelCount    = 2;

/* effect names */
const String _effectName_compander = "COMPANDER";
const String _effectName_filter    = "FILTER";
const String _effectName_reverb    = "REVERB";
const String _effectName_tremolo   = "TREMOLO";

/* output file for debugging */
std::ofstream _outputFile;

/*---------*/
/* TYPES */
/*---------*/

void _writeToOutputFile (String title,
                         SoXAudioEffect& audioEffect)
{
    _outputFile << "====================" << "\n";
    _outputFile << title << "\n";
    _outputFile << "====================" << "\n";
    _outputFile << audioEffect.toString() << "\n";
}

/*--------------------*/

/**
  * Puts five samples onto left and right channel in <buffer>
  */
void _adaptBuffer (INOUT AudioSampleListVector& buffer) {
    AudioSampleList& leftInputList = buffer[0];
    leftInputList[0] =   0.6;
    leftInputList[1] =  -0.3;
    leftInputList[2] =   0.25;
    leftInputList[3] =   0.4;
    leftInputList[4] =  -0.6;
    leftInputList.setLength(5);
    AudioSampleList& rightInputList = buffer[1];
    rightInputList[0] =  0.3;
    rightInputList[1] = -0.6;
    rightInputList[2] =  0.4;
    rightInputList[3] =  0.25;
    rightInputList[4] =  0.5;
    rightInputList.setLength(5);
}

/*--------------------*/

void _copyBuffer (IN AudioSampleListVector& srcBuffer,
                  OUT AudioSampleListVector& destBuffer) {
    Natural channelCount = srcBuffer.length();
    Natural sampleCount = srcBuffer[0].length();
    destBuffer.clear();

    for (Natural channel = 0;  channel < channelCount;
         channel++) {
        const AudioSampleList& srcList = srcBuffer[channel];
        AudioSampleList destList{sampleCount};

        for (Natural i = 0;  i < sampleCount;  i++) {
            destList[i] = srcList[i];
        }

        destBuffer.append(destList);
    }

}

/*--------------------*/

/**
  * Constructs a sine wave with 441Hz for <sampleRate> in <buffer>
  */
void _fillBuffer (OUT AudioSampleListVector& buffer,
                  IN Integer sampleRate) {
    Logging_trace1(">>: sampleRate = %1", TOSTRING(sampleRate));

    Natural bufferLength = Natural{(size_t) sampleRate} / _blocksPerSecond;
    buffer.clear();

    for (Natural channel = 0;  channel < _channelCount;
         channel++) {
        AudioSampleList inputList{bufferLength};

        for (Natural i = 0;  i < bufferLength;  i++) {
            AudioSample s = Real::sin(Real::twoPi * Real(i)
                                         / Real{bufferLength});
            inputList[i] = s;
        }

        buffer.append(inputList);
    }

    Logging_trace("<<");
}

/*--------------------*/

void _initializeEffect (IN String audioEffectKind,
                        INOUT SoXAudioEffect* audioEffect) {
    Logging_trace1(">>: %1", audioEffectKind);

    if (audioEffectKind == _effectName_compander) {
        audioEffect->setValue("0#BANDCOUNT", "4", true);
        audioEffect->setValue("-1#BANDINDEX", "1", true);
        audioEffect->setValue("1#Attack [s]", "0.03", true);
        audioEffect->setValue("1#Decay [s]", "0.15", true);
        audioEffect->setValue("1#Knee [dB]", "6.0", true);
        audioEffect->setValue("1#Threshold [dB]", "-18.0", true);
        audioEffect->setValue("1#Ratio", "6", true);
        audioEffect->setValue("1#Gain [dB]", "4", true);
        audioEffect->setValue("1#Top Frequency [Hz]", "300.0", false);
        audioEffect->setValue("2#Attack [s]", "0.03", true);
        audioEffect->setValue("2#Decay [s]", "0.15", true);
        audioEffect->setValue("2#Knee [dB]", "6.0", true);
        audioEffect->setValue("2#Threshold [dB]", "-18.0", true);
        audioEffect->setValue("2#Ratio", "6", true);
        audioEffect->setValue("2#Gain [dB]", "-7", true);
        audioEffect->setValue("2#Top Frequency [Hz]", "1500.0", false);
        audioEffect->setValue("3#Attack [s]", "0.04", true);
        audioEffect->setValue("3#Decay [s]", "0.15", true);
        audioEffect->setValue("3#Knee [dB]", "6.0", true);
        audioEffect->setValue("3#Threshold [dB]", "-20.0", true);
        audioEffect->setValue("3#Ratio", "20", true);
        audioEffect->setValue("3#Gain [dB]", "10", true);
        audioEffect->setValue("3#Top Frequency [Hz]", "4000.0", false);
        audioEffect->setValue("4#Attack [s]", "0.30", true);
        audioEffect->setValue("4#Decay [s]", "0.50", true);
        audioEffect->setValue("4#Knee [dB]", "6.0", true);
        audioEffect->setValue("4#Threshold [dB]", "-60.0", true);
        audioEffect->setValue("4#Ratio", "1.05", true);
        audioEffect->setValue("4#Gain [dB]", "0", true);
        audioEffect->setValue("4#Top Frequency [Hz]", "25000.0", false);
    } else if (audioEffectKind == _effectName_filter) {
        audioEffect->setValue("Filter Kind", "Equalizer", true);
        audioEffect->setValue("Frequency [Hz]", "1000", true);
        audioEffect->setValue("Bandwidth", "2", true);
        audioEffect->setValue("Bandwidth Unit", "Octave(s)", true);
        audioEffect->setValue("Eq. Gain [dB]", "5", false);
    } else if (audioEffectKind == _effectName_reverb) {
        audioEffect->setValue("isWetOnly?", "false", true);
        audioEffect->setValue("Reverberance [%]", "50", true);
        audioEffect->setValue("HF Damping [%]", "50", true);
        audioEffect->setValue("Room Scale [%]", "100", true);
        audioEffect->setValue("Stereo Depth [%]", "100", true);
        audioEffect->setValue("Pre Delay [ms]", "0", true);
        audioEffect->setValue("Wet Gain [dB]", "0", false);
    } else if (audioEffectKind == _effectName_tremolo) {
        audioEffect->setValue("Effect Kind", "Tremolo", true);
        audioEffect->setValue("Depth [%]", "50", true);
        audioEffect->setValue("Modulation [Hz]", "1", false);
    }

    Logging_trace("<<");
}

/*--------------------*/

SoXAudioEffect* _makeNewEffect (IN String& audioEffectKind,
                                INOUT Natural& testLengthInSeconds) {
    Logging_trace1(">>: %1", audioEffectKind);

    SoXAudioEffect* audioEffect = nullptr;

    testLengthInSeconds = 50000;
    
    if (audioEffectKind == _effectName_compander) {
        audioEffect = new SoXCompander_AudioEffect{};
    } else if (audioEffectKind == _effectName_filter) {
        audioEffect = new SoXFilter_AudioEffect{};
    } else if (audioEffectKind == _effectName_reverb) {
        testLengthInSeconds = 50;
        audioEffect = new SoXReverb_AudioEffect{};
    } else if (audioEffectKind == _effectName_tremolo) {
        audioEffect = new SoXPhaserAndTremolo_AudioEffect{};
    }

    _initializeEffect(audioEffectKind, audioEffect);

    Logging_trace2("<<: testLength = %1s, effect = %2",
                   testLengthInSeconds, audioEffect->toString());
    return audioEffect;
}

/*--------------------*/

/**
 * Performs a multisecond test run for a processor with given
 * <audioEffectKind> using samples from <waveFormBuffer> assuming a
 * sample rate of <sampleRate>
 */
void _runForEffect (IN String& audioEffectKind,
                    INOUT AudioSampleListVector& waveFormBuffer,
                    IN Integer sampleRate) {
    Logging_trace1(">>: kind = %1", audioEffectKind);

    Natural testLengthInSeconds;
    SoXAudioEffect* audioEffect =
        _makeNewEffect(audioEffectKind, testLengthInSeconds);
    Logging_trace("--: AFTER INITIALIZATION");
    _writeToOutputFile("EFFECT AFTER INITIALIZATION", *audioEffect);
    Real timePosition = 0.0;
    Real increment = Real{1.0} / Real{_blocksPerSecond};
    AudioSampleListVector buffer{};

    if (audioEffectKind == _effectName_compander) {
        _adaptBuffer(waveFormBuffer);
    }

    _copyBuffer(waveFormBuffer, buffer);
    audioEffect->prepareToPlay(sampleRate);
    _writeToOutputFile("EFFECT AFTER PREPARATION", *audioEffect);
    Logging_trace("--: AFTER PREPARATION");

    const Natural repetitionCount = testLengthInSeconds * _blocksPerSecond;
    for (Natural i = 0;  i < repetitionCount;  i++) {
        audioEffect->processBlock(timePosition, buffer);
        timePosition += increment;
    }

    _writeToOutputFile("EFFECT AFTER PROCESSING", *audioEffect);
    delete audioEffect;

    Logging_trace("<<");
}

/*--------------------*/
/*--------------------*/

int main (int argc, char* argv[]) {
    Logging_initialize();
    Logging_setFileName("C:/temp/logs/SoXTest.log", false);
    Logging_setIgnoredFunctionNamePrefix("SoXPlugins.");
    Logging_setTracingWithTime(true, 2);
    Logging_trace(">>");
    _outputFile.open("C:/temp/logs/SoXTest.txt");

    const Integer sampleRate = 44100;
    String effectName;
    AudioSampleListVector waveFormBuffer{};
    const Character effectCharacter = (argc < 2 ? ' ' : argv[1][0]);

    if (effectCharacter  == 'C') {
        effectName = _effectName_compander;
    } else if (effectCharacter == 'F') {
        effectName = _effectName_filter;
    } else if (effectCharacter == 'T') {
        effectName = _effectName_tremolo;
    } else {
        effectName = _effectName_reverb;
    }

    Logging_trace1("--: effectName = %1", effectName);
    _fillBuffer(waveFormBuffer, sampleRate);
    _runForEffect(effectName, waveFormBuffer, sampleRate);
    _outputFile.close();

    Logging_trace("<<");
    Logging_finalize();
    return 0;
}
