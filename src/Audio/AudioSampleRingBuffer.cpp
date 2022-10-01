/**
 * @file
 * The <C>AudioSampleRingBuffer</C> body implements a ring buffer for
 * audio samples with left and right shifts and access to first and
 * last elements in ring buffer <I>(this is the formal CPP file used
 * when not doing inlining in production code)</I>.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "AudioSampleRingBuffer.h"

/*====================*/

#ifdef DEBUG
    /* module implementation contains functions */
    #include "AudioSampleRingBuffer.cpp-inc"
#endif
