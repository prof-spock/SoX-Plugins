/**
 * @file
 * The <C>AudioDataPointRingBuffer</C> body implements a ring buffer for
 * audio data points with left and right shifts and access to first
 * and last elements in ring buffer <I>(this is the formal CPP file
 * used when not doing inlining in production code)</I>.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "AudioDataPointRingBuffer.h"

/*====================*/

#ifdef DEBUG
    /* module implementation contains functions */
    #include "AudioDataPointRingBuffer.cpp-inc"
#endif
