/**
 * @file
 * The <C>SoXAudioSampleList</C> body implements a simple list of
 * audio samples.
 */

/*====================*/

#include "SoXAudioSampleList.h"

/*====================*/

using SoXPlugins::CommonAudio::SoXAudioSampleList;

/*====================*/

String SoXAudioSampleList::toString () const
{
    return _toString("SoXAudioSampleList");
}

/*--------------------*/

String SoXAudioSampleList::toString (IN SoXAudioSampleList& list)
{
    return list.toString();
}

/*--------------------*/

void SoXAudioSampleList::setToZero ()
{
    for (SoXAudioSample& sample : *this) {
        sample = 0.0;
    }
}
