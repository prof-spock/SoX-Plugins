/*====================*/

#include <iostream>
#include <fstream>

#include "GlobalMacros.h"
#include "Logging.h"
#include "Natural.h"
#include "Real.h"
#include "SoXAudioEffect.h"
#include "SoXCompander_AudioEffect.h"
#include "SoXPhaserAndTremolo_AudioEffect.h"
#include "SoXReverb_AudioEffect.h"
#include "StringUtil.h"

/*====================*/

using std::cout;

using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::CommonAudio::SoXAudioSample;
using SoXPlugins::CommonAudio::SoXAudioSampleBuffer;
using SoXPlugins::Effects::SoXCompander::SoXCompander_AudioEffect;
using SoXPlugins::Effects::SoXPhaserAndTremolo
         ::SoXPhaserAndTremolo_AudioEffect;
using SoXPlugins::Effects::SoXReverb::SoXReverb_AudioEffect;

/*====================*/

/*-----------*/
/* CONSTANTS */
/*-----------*/

const Natural _blocksPerSecond = 100;
const Natural _channelCount    = 2;
const Natural _repetitionCount = Natural{20} * _blocksPerSecond;

/* effect names */
const String _effectName_compander = "COMPANDER";
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
void _adaptBuffer (INOUT SoXAudioSampleBuffer& buffer) {
    SoXAudioSampleList& leftInputList = buffer[0];
    leftInputList[0] =   0.6;
    leftInputList[1] =  -0.3;
    leftInputList[2] =   0.25;
    leftInputList[3] =   0.4;
    leftInputList[4] =  -0.6;
    leftInputList.setLength(5);
    SoXAudioSampleList& rightInputList = buffer[1];
    rightInputList[0] =  0.3;
    rightInputList[1] = -0.6;
    rightInputList[2] =  0.4;
    rightInputList[3] =  0.25;
    rightInputList[4] =  0.5;
    rightInputList.setLength(5);
}

/*--------------------*/

void _copyBuffer (IN SoXAudioSampleBuffer& srcBuffer,
                  OUT SoXAudioSampleBuffer& destBuffer) {
    Natural channelCount = srcBuffer.length();
    Natural sampleCount = srcBuffer[0].length();
    destBuffer.clear();

    for (Natural channel = 0;  channel < channelCount;
         channel++) {
        const SoXAudioSampleList& srcList = srcBuffer[channel];
        SoXAudioSampleList destList{sampleCount};

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
void _fillBuffer (OUT SoXAudioSampleBuffer& buffer,
                  IN Integer sampleRate) {
    Logging_trace1(">>: sampleRate = %1", TOSTRING(sampleRate));

    Natural bufferLength = Natural{(size_t) sampleRate} / _blocksPerSecond;
    buffer.clear();

    for (Natural channel = 0;  channel < _channelCount;
         channel++) {
        SoXAudioSampleList inputList{bufferLength};

        for (Natural i = 0;  i < bufferLength;  i++) {
            SoXAudioSample s = Real::sin(Real::twoPi * Real(i)
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
        audioEffect->setValue("0#BANDCOUNT", "1", true);
        audioEffect->setValue("-1#BANDINDEX", "1", true);
        audioEffect->setValue("1#Attack [s]", "0.02", true);
        audioEffect->setValue("1#Decay [s]", "0.15", true);
        audioEffect->setValue("1#Knee [dB]", "4.0", true);
        audioEffect->setValue("1#Threshold [dB]", "-60.0", true);
        audioEffect->setValue("1#Ratio", "1.5", true);
        audioEffect->setValue("1#Gain [dB]", "4.5", true);
        audioEffect->setValue("1#Top Frequency [Hz]", "25000.0", false);
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

SoXAudioEffect* _makeNewEffect (IN String& audioEffectKind) {
    Logging_trace1(">>: %1", audioEffectKind);

    SoXAudioEffect* audioEffect = nullptr;

    if (audioEffectKind == _effectName_compander) {
        audioEffect = new SoXCompander_AudioEffect{};
    } else if (audioEffectKind == _effectName_reverb) {
        audioEffect = new SoXReverb_AudioEffect{};
    } else if (audioEffectKind == _effectName_tremolo) {
        audioEffect = new SoXPhaserAndTremolo_AudioEffect{};
    }

    _initializeEffect(audioEffectKind, audioEffect);

    Logging_trace1("<<: %1", audioEffect->toString());
    return audioEffect;
}

/*--------------------*/

/**
 * Performs a multisecond test run for a processor with given
 * <audioEffectKind> using samples from <waveFormBuffer> assuming a
 * sample rate of <sampleRate>
 */
void _runForEffect (IN String& audioEffectKind,
                    INOUT SoXAudioSampleBuffer& waveFormBuffer,
                    IN Integer sampleRate) {
    Logging_trace1(">>: kind = %1", audioEffectKind);

    SoXAudioEffect* audioEffect = _makeNewEffect(audioEffectKind);
    Logging_trace("--: AFTER INITIALIZATION");
    _writeToOutputFile("EFFECT AFTER INITIALIZATION", *audioEffect);
    Real timePosition = 0.0;
    Real increment = Real{1} / Real{_blocksPerSecond};
    Natural repetitionCount = _repetitionCount;
    SoXAudioSampleBuffer buffer{};

    if (audioEffectKind == _effectName_compander) {
        _adaptBuffer(waveFormBuffer);
        repetitionCount = 1;
    }

    _copyBuffer(waveFormBuffer, buffer);
    audioEffect->prepareToPlay((int) sampleRate);
    _writeToOutputFile("EFFECT AFTER PREPARATION", *audioEffect);
    Logging_trace("--: AFTER PREPARATION");

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
    SoXAudioSampleBuffer waveFormBuffer{};

    if (argc > 1 && argv[1][0] == 'C') {
        effectName = _effectName_compander;
    } else if (argc > 1 && argv[1][0] == 'T') {
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
