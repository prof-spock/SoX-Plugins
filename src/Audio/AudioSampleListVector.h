/**
 * @file
 * The <C>AudioSampleListVector</C> specification defines the
 * communication buffer structure for the block processing of audio
 * effects; it is a list of audio sample lists containing the input
 * samples and finally the (processed) output samples
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "AudioSampleList.h"

/*====================*/

namespace Audio {

    /**
     * A <C>AudioSampleListVector</C> object is a buffer of audio
     * sample lists containing the input samples and the (processed)
     * output samples serving as the communication buffer structure
     * for the block processing of audio effects.
     */
    struct AudioSampleListVector
        : public GenericSequence<AudioSampleList,
                                 AudioSampleList::toString> {

        /**
         * Returns printable representation of buffer.
         *
         * @return string representation of buffer
         */
        String toString () const;

        /*--------------------*/

        /**
         * Sets all entries in sample buffer to 0
         */
        void setToZero ();
    };

}

/*============================================================*/

#ifndef DEBUG
    /* production code is inlined */
    #include "AudioSampleListVector.cpp-inc"
#endif
