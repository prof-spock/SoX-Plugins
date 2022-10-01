/**
 * @file
 * The <C>WaveForm</C> body implements common services for (LFO)
 * wave forms.  The algorithms here are based on implementation ideas
 * from Carsten Borchardt and Rob Sewell in SoX file "synth.c".
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*=========*/
/* IMPORTS */
/*=========*/

#include "WaveForm.h"

#include <cmath>
#include "Assertion.h"
#include "Logging.h"

/*--------------------*/

using Audio::WaveForm;
using Audio::WaveFormIteratorState;
using Audio::WaveFormKind;

/*============================================================*/

namespace Audio {

    /** <C>_WaveTable</C> is a local alias name for a real list */
    using  _WaveTable = RealList;

    static Boolean _isInitialized = false;
    static _WaveTable _sineWaveTable;
    static _WaveTable _triangleWaveTable;

    /*--------------------*/

    /**
     * A <C>_WaveFormDescriptor</C> object contains the waveform
     * information and a cursor to it; the cursor position is
     * calculated by a linear function with start offset, step count
     * and increment instead of blindly adding up the increment: this
     * leads to no rounding runoff errors compared to the 
     * implementation.
     */
    struct _WaveFormDescriptor {
        /** the kind of waveform (sine or triangle) */
        WaveFormKind kind;

        /** the increment for stepping in the waveform */
        Real increment;

        /** the first position in the waveform */
        Real firstPosition;

        /** the number of steps in the waveform */
        Natural stepCount;

        /** the current (cursor) position in the waveform */
        Real position;

        /** the minimum value in the waveform */
        Real minimumValue;

        /** the maximum value in the waveform */
        Real maximumValue;

        /** the information whether only integer values may be used */
        Boolean hasIntegerValues;

        /** the count of points in the waveform */
        Natural waveTableLength;

        /** the data buffer for the waveform */
        _WaveTable* buffer;
    };

    /*--------------------*/
    /* internal routines  */
    /*--------------------*/

    /**
     * Returns string representation of <C>kind</C>.
     *
     * @param[in] kind  wave form kind
     * @return string representation of kind
     */
    static String _waveFormKind (IN WaveFormKind kind)
    {
        return String(kind == WaveFormKind::sine
                      ? "sine" : "triangle");
    }

    /*--------------------*/

    /**
     * Finds value in <C>waveTable</C> at real <C>position</C> by a linear
     * interpolation
     *
     * @param[in] waveTable  the list of samples in wave form
     * @param[in] position   the position in the wave form
     */
    static Real _getWavetableValueAtPosition (IN _WaveTable* waveTable,
                                              IN Real position)
    {
        const Natural waveTableLength = waveTable->length();
        Assertion_pre(position >= 0.0, "position must be non-negative");
        Assertion_pre(waveTableLength > 0, "wave table must be non-empty");

        const Natural indexA = (Natural) position % waveTableLength;
        const Natural indexB = (indexA + 1) % waveTableLength;
        const Real valueA = waveTable->at(indexA);
        const Real valueB = waveTable->at(indexB);
        const Real fPart = position.fractionalPart();
        const Real value = valueA * (Real{1.0} - fPart) + valueB * fPart;
        return value;
    }

    /*--------------------*/

    /**
     * Initializes <C>waveTable</C> to be of <C>kind</C> having
     * <C>length</C> base points, the range is [0, +1] for all wave forms
     *
     * @param[inout] waveTable  the list of samples in wave form
     * @param[in] kind          the wave form kind
     * @param[in] length        the number of sampling points
     */
    static void _initializeWaveTable (INOUT _WaveTable& waveTable,
                                      IN WaveFormKind kind,
                                      IN Natural length)
    {
        Logging_trace2(">>: kind = %1, length = %2",
                       _waveFormKind(kind), TOSTRING(length));

        waveTable.setLength(length);
        const Real delta = Real::twoPi / (Real) length;
        const Real one{1.0};
        const Real two{2.0};
        const Real four{4.0};

        for (Natural i = 0;  i < length;  i++) {
            const Real x{i};
            Real y = 0.0;

            if (kind == WaveFormKind::sine) {
                y = (Real::sin(x * delta) + one) / two;
            } else {
                const Integer quadrant{x * four / Real{length}};
                y = x * two / Real{length};
                y = (quadrant == 0 ? y + 0.5
                     : (quadrant == 3 ? y - 1.5 : Real{1.5} - y));
            }

            waveTable[i] = y;
        }
    }

    /*--------------------*/

    /**
     * Calculates position from settings in <C>descriptor</C>.
     *
     * @param[in] descriptor  the descriptor for the wave form
     */
    static Real _position (IN _WaveFormDescriptor* descriptor)
    {
        Real p = (descriptor->firstPosition
                  + Real{descriptor->stepCount} * descriptor->increment);
        return Real::mod(p, Real{descriptor->waveTableLength});
    }

}

/*============================================================*/

/*--------------------*/
/* setup              */
/*--------------------*/

WaveForm::WaveForm ()
{
    Logging_trace(">>");

    if (!_isInitialized) {
        // inititalize static variables

        // for a triangle wave with linear interpolation four
        // equidistant sampling points are enough
        const Natural _sineWaveTableLength     = 10000;
        const Natural _triangleWaveTableLength = 4;

        _initializeWaveTable(_sineWaveTable,
                             WaveFormKind::sine,
                             _sineWaveTableLength);
        _initializeWaveTable(_triangleWaveTable,
                             WaveFormKind::triangle,
                             _triangleWaveTableLength);
        _isInitialized = true;
    }

    _WaveFormDescriptor* descriptor = new _WaveFormDescriptor{};
    _internalData = descriptor;
    set(1000.0, WaveFormKind::sine, 0.0, 1.0, 0.0, false);

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/

WaveForm::~WaveForm ()
{
    Logging_trace(">>");
    _WaveFormDescriptor* descriptor =
        (_WaveFormDescriptor*) _internalData;
    delete descriptor;
    Logging_trace("<<");
}

/*--------------------*/

String WaveForm::toString() const {
    const _WaveFormDescriptor* descriptor =
        static_cast<_WaveFormDescriptor*>(_internalData);

    String result = "WaveForm(";
    result += "kind = " + _waveFormKind(descriptor->kind);
    result += ", firstPosition = " + TOSTRING(descriptor->firstPosition);
    result += ", increment = " + TOSTRING(descriptor->increment);
    result += ", stepCount = " + TOSTRING(descriptor->stepCount);
    result += ", position = " + TOSTRING(descriptor->position);
    result += ", minimumValue = " + TOSTRING(descriptor->minimumValue);
    result += ", maximumValue = " + TOSTRING(descriptor->maximumValue);
    result += (", hasIntegerValues = "
               + TOSTRING(descriptor->hasIntegerValues));
    result += (", waveTableLength = "
               + TOSTRING(descriptor->waveTableLength));
    result += ", buffer = " + descriptor->buffer->toString();
    result += ")";

    return result;
}

/*--------------------*/

void WaveForm::set (Real length, WaveFormKind kind,
                    Real minimumValue, Real maximumValue,
                    Radians phase, Boolean hasIntegerValues)
{
    Logging_trace6(">>: length = %1, kind = %2,"
                   " minimumValue = %3, maximumValue = %4,"
                   " phase = %5, hasIntegerValues = %6",
                   TOSTRING(length), _waveFormKind(kind),
                   TOSTRING(minimumValue), TOSTRING(maximumValue),
                   TOSTRING(phase), TOSTRING(hasIntegerValues));
    Assertion_pre(length > 0.0, "length must be positive");

    _WaveFormDescriptor* descriptor =
        static_cast<_WaveFormDescriptor*>(_internalData);

    _WaveTable* buffer = (kind == WaveFormKind::sine
                          ? &_sineWaveTable
                          : &_triangleWaveTable);
    const Natural waveTableLength = buffer->length();
    const Real firstPosition =
        Real::mod((Real) waveTableLength * phase / Real::twoPi,
                  (Real) waveTableLength);

    descriptor->kind             = kind;
    descriptor->firstPosition    = firstPosition;
    descriptor->increment        = (Real) waveTableLength / length;
    descriptor->stepCount        = 0;
    descriptor->position         = firstPosition;
    descriptor->minimumValue     = minimumValue;
    descriptor->maximumValue     = maximumValue;
    descriptor->hasIntegerValues = hasIntegerValues;
    descriptor->waveTableLength  = waveTableLength;
    descriptor->buffer           = buffer;

    Logging_trace1("<<: %1", toString());
}

/*--------------------*/
/* property access    */
/*--------------------*/

Real WaveForm::current () const
{
    const _WaveFormDescriptor* descriptor =
        static_cast<_WaveFormDescriptor*>(_internalData);
    const Real minimumValue = descriptor->minimumValue;
    const Boolean hasIntegerValues = descriptor->hasIntegerValues;
    const Real scalingFactor = descriptor->maximumValue - minimumValue;
    Real value = _getWavetableValueAtPosition(descriptor->buffer,
                                              descriptor->position);
    value = minimumValue + value * scalingFactor;

    //  does not distribute integer values equally over their
    // range: the maximum value only gets 50% coverage of all the
    // other values
    value = (hasIntegerValues ? value.round() : value);
    return value;
}

/*--------------------*/

WaveFormIteratorState WaveForm::state () const
{
    const _WaveFormDescriptor* descriptor =
        static_cast<_WaveFormDescriptor*>(_internalData);
    return descriptor->stepCount;
}

/*--------------------*/
/* property change    */
/*--------------------*/

void WaveForm::reset ()
{
    _WaveFormDescriptor* descriptor =
        static_cast<_WaveFormDescriptor*>(_internalData);
    descriptor->position  = descriptor->firstPosition;
    descriptor->stepCount = 0;
}

/*--------------------*/

void WaveForm::setState (WaveFormIteratorState state)
{
    _WaveFormDescriptor* descriptor =
        static_cast<_WaveFormDescriptor*>(_internalData);
    const Natural stepCount = state;
    descriptor->stepCount = stepCount;
    descriptor->position  = _position(descriptor);
}

/*--------------------*/

void WaveForm::advance ()
{
    _WaveFormDescriptor* descriptor =
        static_cast<_WaveFormDescriptor*>(_internalData);
    descriptor->stepCount++;
    descriptor->position = _position(descriptor);
}

/*--------------------*/
/* time lock service  */
/*--------------------*/

Radians WaveForm::phaseByTime (IN Real frequency,
                               IN Real timeOffset,
                               IN Real currentTime)
{
    Logging_trace3(">>: frequency = %1, timeOffset = %2,"
                   " currentTime = %3",
                   TOSTRING(frequency), TOSTRING(timeOffset),
                   TOSTRING(currentTime));

    Radians result;
    const Real deltaTime = currentTime - timeOffset;
    result = Real::fractionalPart(deltaTime * frequency) * Real::twoPi;
    result = result.mod(Real::twoPi);

    Logging_trace1("<<: %1", TOSTRING(result));
    return result;
}
