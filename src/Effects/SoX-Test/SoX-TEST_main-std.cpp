/**
 * @file
 * The <C>SoX-TEST</C> module implements a simple command-line test
 * program for all SoX effects <B>reverb</B>,
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
#include "OperatingSystem.h"
#include "SoXChorusAndEcho_AudioEffect.h"
#include "SoXCompander_AudioEffect.h"
#include "SoXFilter_AudioEffect.h"
#include "SoXFlangerPhaserAndTremolo_AudioEffect.h"
#include "SoXGain_AudioEffect.h"
#include "SoXOverdrive_AudioEffect.h"
#include "SoXReverb_AudioEffect.h"

/*--------------------*/

using std::cout;

using Audio::AudioDataPoint;
using BaseModules::OperatingSystem;
using SoXPlugins::Effects::SoXCompander::SoXCompander_AudioEffect;
using SoXPlugins::Effects::SoXChorusAndEcho
         ::SoXChorusAndEcho_AudioEffect;
using SoXPlugins::Effects::SoXFilter::SoXFilter_AudioEffect;
using SoXPlugins::Effects::SoXFlangerPhaserAndTremolo
         ::SoXFlangerPhaserAndTremolo_AudioEffect;
using SoXPlugins::Effects::SoXGain::SoXGain_AudioEffect;
using SoXPlugins::Effects::SoXOverdrive::SoXOverdrive_AudioEffect;
using SoXPlugins::Effects::SoXReverb::SoXReverb_AudioEffect;

/*====================*/

/*-----------*/
/* CONSTANTS */
/*-----------*/

const Natural _blocksPerSecond = 100;
const Natural _channelCount    = 2;

/* effect names */
const String _effectName_chorus     = "CHORUS";
const String _effectName_filter     = "FILTER";
const String _effectName_flanger    = "FLANGER";
const String _effectName_gain       = "GAIN";
const String _effectName_mcompander = "MCOMPANDER";
const String _effectName_overdrive  = "OVERDRIVE";
const String _effectName_phaser     = "PHASER";
const String _effectName_reverb     = "REVERB";
const String _effectName_tremolo    = "TREMOLO";

/** mapping from single character to name of effect */
static const Dictionary _characterToEffectNameMap =
    Dictionary::makeFromString(
            "C -> "  +  _effectName_chorus
        + ", F -> "  + _effectName_filter
        + ", G -> "  + _effectName_gain
        + ", L -> "  + _effectName_flanger
        + ", M -> "  + _effectName_mcompander
        + ", O -> "  + _effectName_overdrive
        + ", P -> "  + _effectName_phaser
        + ", R -> "  + _effectName_reverb
        + ", T -> "  + _effectName_tremolo
    );

/* output file for debugging */
std::ofstream _outputFile;

/*--------------------*/

static void _writeToOutputFile (IN String& title,
                         IN SoXAudioEffect& audioEffect)
{
    _outputFile << "====================" << "\n";
    _outputFile << title << "\n";
    _outputFile << "====================" << "\n";
    _outputFile << audioEffect.toString() << "\n";
}

/*--------------------*/

/**
  * Puts five data points onto left and right channel in <buffer>
  */
static void _adaptBuffer (INOUT AudioDataPointListVector& buffer)
{
    AudioDataPointList& leftInputList = buffer[0];
    leftInputList[0] =   0.6;
    leftInputList[1] =  -0.3;
    leftInputList[2] =   0.25;
    leftInputList[3] =   0.4;
    leftInputList[4] =  -0.6;
    leftInputList.setLength(5);
    AudioDataPointList& rightInputList = buffer[1];
    rightInputList[0] =  0.3;
    rightInputList[1] = -0.6;
    rightInputList[2] =  0.4;
    rightInputList[3] =  0.25;
    rightInputList[4] =  0.5;
    rightInputList.setLength(5);
}

/*--------------------*/

static void _copyBuffer (IN AudioDataPointListVector& srcBuffer,
                         OUT AudioDataPointListVector& destBuffer)
{
    Natural channelCount   = srcBuffer.length();
    Natural dataPointCount = srcBuffer[0].length();
    destBuffer.clear();

    for (Natural channel = 0;  channel < channelCount;
         channel++) {
        const AudioDataPointList& srcList = srcBuffer[channel];
        AudioDataPointList destList{dataPointCount};

        for (Natural i = 0;  i < dataPointCount;  i++) {
            destList[i] = srcList[i];
        }

        destBuffer.append(destList);
    }

}

/*--------------------*/

/**
 * Constructs a sine wave with 441Hz for <C>dataPointRate</C> in
 * <C>buffer</C>.
 *
 * @param[out] buffer         list vector of data points
 * @param[in]  dataPointRate  data point rate (in Hz)
 */
static void _fillBuffer (OUT AudioDataPointListVector& buffer,
                         IN Integer dataPointRate)
{
    Logging_trace1(">>: dataPointRate = %1", TOSTRING(dataPointRate));

    Natural bufferLength = Natural(dataPointRate / _blocksPerSecond);
    buffer.clear();

    for (Natural channel = 0;  channel < _channelCount;
         channel++) {
        AudioDataPointList inputList{bufferLength};

        for (Natural i = 0;  i < bufferLength;  i++) {
            AudioDataPoint s = Real::sin(Real::twoPi * Real(i)
                                         / Real{bufferLength});
            inputList[i] = s;
        }

        buffer.append(inputList);
    }

    Logging_trace("<<");
}

/*-----------------------*/
/* effect initialization */
/*-----------------------*/

/**
 * Initializes <C>audioEffect</C> for a chorus.
 * 
 */
static void _initializeEffectForChorus (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("Effect Kind",      "Chorus");

    audioEffect->setValue("In Gain",          "1.0");
    audioEffect->setValue("Out Gain",         "0.4");
    audioEffect->setValue("Time Offset [s]",  "0");

    audioEffect->setValue("-2#Stage Count",   "3");
    audioEffect->setValue("-1#Stage Index",   "1");

    audioEffect->setValue("1#Delay [ms]",     "50");
    audioEffect->setValue("1#Decay",          "0.75");
    audioEffect->setValue("1#Frequency [Hz]", "1.25");
    audioEffect->setValue("1#Depth [ms]",     "1");
    audioEffect->setValue("1#Waveform",       "Sine");

    audioEffect->setValue("2#Delay [ms]",     "75");
    audioEffect->setValue("2#Decay",          "0.5");
    audioEffect->setValue("2#Frequency [Hz]", "2.5");
    audioEffect->setValue("2#Depth [ms]",     "2");
    audioEffect->setValue("2#Waveform",       "Triangle");

    audioEffect->setValue("3#Delay [ms]",     "100");
    audioEffect->setValue("3#Decay",          "0.25");
    audioEffect->setValue("3#Frequency [Hz]", "5");
    audioEffect->setValue("3#Depth [ms]",     "4");
    audioEffect->setValue("3#Waveform",       "Sine");

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Initializes <C>audioEffect</C> for a filter.
 * 
 */
static void _initializeEffectForFilter (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("Filter Kind",    "Equalizer");
    audioEffect->setValue("Frequency [Hz]", "520");
    audioEffect->setValue("Bandwidth",      "2");
    audioEffect->setValue("Bandwidth Unit", "Octave(s)");
    audioEffect->setValue("Eq. Gain [dB]",  "-3");

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Initializes <C>audioEffect</C> for a flanger.
 * 
 */
static void _initializeEffectForFlanger (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("Effect Kind",      "Flanger");
    audioEffect->setValue("Delay [ms]",       "10.0");
    audioEffect->setValue("Depth [ms]",       "5.0");
    audioEffect->setValue("Regeneration [%]", "30");
    audioEffect->setValue("Width [%]",        "55");
    audioEffect->setValue("Modulation [Hz]",  "0.5");
    audioEffect->setValue("Waveform",         "Sine");
    audioEffect->setValue("Time Offset [s]",  "0.0");

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Initializes <C>audioEffect</C> for a gain.
 * 
 */
static void _initializeEffectForGain (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("Gain [dB]", "-7.5");
    
    Logging_trace("<<");
}

/*--------------------*/

/**
 * Initializes <C>audioEffect</C> for a multiband compander.
 * 
 */
static void _initializeEffectForMCompand (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("-2#Band Count",        "2");
    audioEffect->setValue("-1#Band Index",        "1");

    audioEffect->setValue("1#Attack [s]",         "0.02");
    audioEffect->setValue("1#Decay [s]",          "0.15");
    audioEffect->setValue("1#Knee [dB]",          "4.0");
    audioEffect->setValue("1#Threshold [dB]",     "-60.0");
    audioEffect->setValue("1#Ratio",              "1.5");
    audioEffect->setValue("1#Gain [dB]",          "4.5");
    audioEffect->setValue("1#Top Frequency [Hz]", "1000.0");

    audioEffect->setValue("2#Attack [s]",         "0.15");
    audioEffect->setValue("2#Decay [s]",          "0.4");
    audioEffect->setValue("2#Knee [dB]",          "0");
    audioEffect->setValue("2#Threshold [dB]",     "-20.0");
    audioEffect->setValue("2#Ratio",              "2");
    audioEffect->setValue("2#Gain [dB]",          "-2");
    audioEffect->setValue("2#Top Frequency [Hz]", "25000.0");

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Initializes <C>audioEffect</C> for a overdrive.
 * 
 */
static void _initializeEffectForOverdrive (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("Gain [dB]", "3");
    audioEffect->setValue("Colour",    "40");

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Initializes <C>audioEffect</C> for a phaser.
 * 
 */
static void _initializeEffectForPhaser (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("Effect Kind",     "Phaser");
    audioEffect->setValue("In Gain",         "0.6");
    audioEffect->setValue("Out Gain",        "0.66");
    audioEffect->setValue("Delay [ms]",      "3");
    audioEffect->setValue("Regeneration",    "0.4");
    audioEffect->setValue("Modulation [Hz]", "0.5");
    audioEffect->setValue("Waveform",        "Triangle");
    audioEffect->setValue("Interp. Order",   "0");
    audioEffect->setValue("Time Offset [s]", "0");

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Initializes <C>audioEffect</C> for a reverb.
 * 
 */
static void _initializeEffectForReverb (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("Is Wet Only?",     "false");
    audioEffect->setValue("Reverberance [%]", "60");
    audioEffect->setValue("HF Damping [%]",   "22");
    audioEffect->setValue("Room Scale [%]",   "87.5");
    audioEffect->setValue("Stereo Depth [%]", "34.88");
    audioEffect->setValue("Predelay [ms]",    "20");
    audioEffect->setValue("Wet Gain [dB]",    "-3");

    Logging_trace("<<");
}

/*--------------------*/

/**
 * Initializes <C>audioEffect</C> for a tremolo.
 * 
 */
static void _initializeEffectForTremolo (INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace(">>");

    audioEffect->setValue("Effect Kind",     "Tremolo");
    audioEffect->setValue("Modulation [Hz]", "0.395");
    audioEffect->setValue("Depth [%]",       "94.67");
    audioEffect->setValue("Time Offset [s]", "0");

    Logging_trace("<<");
}

/*--------------------*/

static void _initializeEffect (IN String& audioEffectKind,
                               INOUT SoXAudioEffect* audioEffect)
{
    Logging_trace1(">>: %1", audioEffectKind);

    if (audioEffectKind == _effectName_chorus) {
        _initializeEffectForChorus(audioEffect);
    } else if (audioEffectKind == _effectName_filter) {
        _initializeEffectForFilter(audioEffect);
    } else if (audioEffectKind == _effectName_flanger) {
        _initializeEffectForFlanger(audioEffect);
    } else if (audioEffectKind == _effectName_gain) {
        _initializeEffectForGain(audioEffect);
    } else if (audioEffectKind == _effectName_mcompander) {
        _initializeEffectForMCompand(audioEffect);
    } else if (audioEffectKind == _effectName_overdrive) {
        _initializeEffectForOverdrive(audioEffect);
    } else if (audioEffectKind == _effectName_phaser) {
        _initializeEffectForPhaser(audioEffect);
    } else if (audioEffectKind == _effectName_reverb) {
        _initializeEffectForReverb(audioEffect);
    } else if (audioEffectKind == _effectName_tremolo) {
        _initializeEffectForTremolo(audioEffect);
    }

    audioEffect->recalculate();

    Logging_trace1("<<: %1", audioEffect.toString());
}

/*--------------------*/

static SoXAudioEffect* _makeNewEffect (IN String& audioEffectKind)
{
    Logging_trace1(">>: %1", audioEffectKind);

    SoXAudioEffect* audioEffect = nullptr;

    if (audioEffectKind == _effectName_chorus) {
        audioEffect = new SoXChorusAndEcho_AudioEffect{};
    } else if (audioEffectKind == _effectName_filter) {
        audioEffect = new SoXFilter_AudioEffect{};
    } else if (audioEffectKind == _effectName_flanger) {
        audioEffect = new SoXFlangerPhaserAndTremolo_AudioEffect{};
    } else if (audioEffectKind == _effectName_gain) {
        audioEffect = new SoXGain_AudioEffect{};
    } else if (audioEffectKind == _effectName_mcompander) {
        audioEffect = new SoXCompander_AudioEffect{};
    } else if (audioEffectKind == _effectName_overdrive) {
        audioEffect = new SoXOverdrive_AudioEffect{};
    } else if (audioEffectKind == _effectName_phaser) {
        audioEffect = new SoXFlangerPhaserAndTremolo_AudioEffect{};
    } else if (audioEffectKind == _effectName_reverb) {
        audioEffect = new SoXReverb_AudioEffect{};
    } else if (audioEffectKind == _effectName_tremolo) {
        audioEffect = new SoXFlangerPhaserAndTremolo_AudioEffect{};
    } else {
        Logging_traceError("--: bad audio effect kind");
    }

    String effectAsString;

    if (audioEffect == nullptr) {
        effectAsString = "NULL";
    } else {
        _initializeEffect(audioEffectKind, audioEffect);
        effectAsString = (!Logging_isActive
                          ? audioEffectKind
                          : audioEffect->toString());
    }

    Logging_trace1("<<: effect = %1", effectAsString);
    return audioEffect;
}

/*--------------------*/

/**
 * Performs a multisecond test run for a processor with given
 * <C>audioEffectKind</C> using data points from <C>waveFormBuffer</C>
 * assuming a data point rate of <C>dataPointRate</C>
 * 
 * @param[in]  audioEffectKind  kind of audio effect to be tested
 * @param[in]  waveFormBuffer   buffer of data points for effect
 * @param[in]  dataPointRate    data point rate (in Hz)
 */
static void _runForEffect (IN String& audioEffectKind,
                           INOUT AudioDataPointListVector& waveFormBuffer,
                           IN Integer dataPointRate) {
    Logging_trace1(">>: kind = %1", audioEffectKind);

    const Natural testLengthInSeconds = 1;
    SoXAudioEffect* audioEffect = _makeNewEffect(audioEffectKind);
    Logging_trace("--: AFTER INITIALIZATION");
    // _writeToOutputFile("EFFECT AFTER INITIALIZATION", *audioEffect);
    Real timePosition = 0.0;
    Real increment = Real{1.0} / Real{_blocksPerSecond};
    AudioDataPointListVector buffer{};

    if (audioEffectKind == _effectName_mcompander) {
        _adaptBuffer(waveFormBuffer);
    }

    _copyBuffer(waveFormBuffer, buffer);
    audioEffect->prepareToPlay(dataPointRate);
    // _writeToOutputFile("EFFECT AFTER PREPARATION", *audioEffect);
    Logging_trace("--: AFTER PREPARATION");

    if (audioEffectKind == _effectName_reverb) {
        SoXReverb_AudioEffect* reverb =
            static_cast<SoXReverb_AudioEffect*>(audioEffect);
        Real tailLength = reverb->tailLength();
    }

    const Natural repetitionCount = testLengthInSeconds * _blocksPerSecond;
    for (Natural i = 0;  i < repetitionCount;  i++) {
        audioEffect->processBlock(timePosition, buffer);
        timePosition += increment;
    }

    // _writeToOutputFile("EFFECT AFTER PROCESSING", *audioEffect);
    delete audioEffect;

    Logging_trace("<<");
}

/*--------------------*/
/*--------------------*/

int main (int argc, char* argv[])
{
    Logging_initialize();
    String temporaryDirectoryPath =
        OperatingSystem::temporaryDirectoryPath();
    Logging_setFileName(temporaryDirectoryPath + "/SoXTest.log", false);
    Logging_setIgnoredFunctionNamePrefix("SoXPlugins.");
    Logging_setTracingWithTime(true, 2);
    Logging_trace(">>");
    _outputFile.open(temporaryDirectoryPath + "/SoXTest.txt");

    const Integer dataPointRate = 44100;
    String effectName;
    AudioDataPointListVector waveFormBuffer{};
    const String effectCharacter =
        (argc < 2 ? " " : String{argv[1]}.substr(0, 1));

    if (!_characterToEffectNameMap.contains(effectCharacter)) {
        Logging_trace1("--: bad effect character '%s'",
                       effectCharacter);
    } else {
        effectName = _characterToEffectNameMap.at(effectCharacter);
        Logging_trace1("--: effectName = %1", effectName);
        _fillBuffer(waveFormBuffer, dataPointRate);
        _runForEffect(effectName, waveFormBuffer, dataPointRate);
        _outputFile.close();
    }

    Logging_trace("<<");
    Logging_finalize();
    return 0;
}
