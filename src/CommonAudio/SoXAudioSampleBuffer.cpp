/**
 * @file
 * The <C>SoXAudioSampleBuffer</C> body implements the communication
 * buffer structure for the block processing of SoX audio effects; it
 * is a buffer of audio sample lists containing the input samples and
 * the (processed) output samples
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#include "SoXAudioSampleBuffer.h"

/*====================*/

using SoXPlugins::CommonAudio::SoXAudioSampleBuffer;

/*====================*/

String SoXAudioSampleBuffer::toString () const
{
    return _toString("SoXAudioSampleBuffer");
}

/*--------------------*/

void SoXAudioSampleBuffer::setToZero ()
{
    for (SoXAudioSampleList& list : *this) {
        list.setToZero();
    }
}
