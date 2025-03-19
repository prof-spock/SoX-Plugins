/**
 * @file
 * The <C>WaveForm</C> body implements common services for (LFO)
 * wave forms.  The algorithms here are based on implementation ideas
 * from Carsten Borchardt and Rob Sewell in SoX file
 * "synth.c". <I>(this is the formal CPP file used when not doing
 * inlining in production code)</I>
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "WaveForm.h"

/*====================*/

#ifdef DEBUG
    /* module implementation contains functions */
    #include "WaveForm.cpp-inc"
#endif

/*============================================================*/

using Audio::_WaveTable;

/*============================================================*/

/** number of base points for a sine wave */
const Natural _sineWaveTableLength     = 10000;

/** number of base points for a triangle wave(four is enough!) */
const Natural _triangleWaveTableLength = 4;

/*============================================================*/

static _WaveTable _makeWaveTable (IN WaveFormKind kind,
                                  IN Natural length);
                                  
/*============================================================*/

/** the reference table for a sine wave */
_WaveTable _sineWaveTable =
    _makeWaveTable(WaveFormKind::sine, _sineWaveTableLength);

/** the reference table for a triangle wave */
_WaveTable _triangleWaveTable =
    _makeWaveTable(WaveFormKind::triangle, _triangleWaveTableLength);

/*============================================================*/

/**
 * Constructs waveTable of <C>kind</C> having <C>length</C> base
 * points, the range is [0, +1] for all wave forms
 *
 * @param[in] kind    the wave form kind
 * @param[in] length  the number of base points
 * @return  new wave table
 */
static _WaveTable _makeWaveTable (IN WaveFormKind kind,
                                  IN Natural length)
{
    Logging_trace2(">>: kind = %1, length = %2",
                   waveFormKindToString(kind), TOSTRING(length));

    _WaveTable result;
    result.setLength(length);
    const Real delta = Real::twoPi / (Real) length;
    const Real four{4.0};

    for (Natural i = 0;  i < length;  i++) {
        const Real x{i};
        Real y = 0.0;

        if (kind == WaveFormKind::sine) {
            y = (Real::sin(x * delta) + Real::one) / Real::two;
        } else {
            const Integer quadrant{x * four / Real{length}};
            y = x * Real::two / Real{length};
            y = (quadrant == 0 ? y + 0.5
                 : (quadrant == 3 ? y - 1.5 : Real{1.5} - y));
        }

        result.set(i, y);
    }

    Logging_trace("<<");
    return result;
}
