/**
 * @file
 * The <C>SoXAudioSampleQueue</C> body implements a ring buffer for
 * audio samples with left and right shifts and access to first and
 * last elements in queue <I>(this is the formal CPP file used when
 * not doing inlining in production code)</I>.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#include "SoXAudioSampleQueue.h"

#ifdef DEBUG
    // module implementation contains functions
    #include "SoXAudioSampleQueue.cpp-inc"
#endif
