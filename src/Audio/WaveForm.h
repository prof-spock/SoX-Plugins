/**
 * @file
 * The <C>WaveForm</C> specification defines common services for
 * (LFO) wave forms.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-06
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "Boolean.h"
#include "Natural.h"
#include "Object.h"
#include "Radians.h"
#include "RealList.h"

/*--------------------*/

using BaseTypes::Primitives::Boolean;
using BaseTypes::Primitives::Natural;
using BaseTypes::Primitives::Object;
using BaseTypes::Primitives::Radians;
using BaseTypes::Containers::RealList;

/*====================*/

namespace Audio {

    /**
     * A <C>WaveFormIteratorState</C> object holds the state of an
     * iterator on the current waveform.
     */
    using WaveFormIteratorState = Natural;

    /*--------------------*/

    /**
     * The <C>WaveFormKind</C> is an enumeration type for the
     * supported kinds of wave forms: sine and triangle wave.
     */
    enum class WaveFormKind { sine, triangle };

    /*--------------------*/

    /**
     * A <C>WaveForm</C> object provides common services for (LFO)
     * wave forms.
     */
    struct WaveForm {

        /**
         * Creates empty waveform
         */
        WaveForm ();

        /*--------------------*/

        /**
         * Deletes waveform
         */
        ~WaveForm ();

        /*--------------------*/

        /**
         * Returns string representation of waveform.
         *
         * @return string representation
         */
        String toString() const;

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        /**
         * Updates waveform to <C>length</C> base points with a wave
         * form given by <C>kind</C>; wave form has
         * <C>minimumValue</C> and <C>maximumValue</C> as bounds;
         * <C>phase</C> specifies the offset in the waveform and
         * <C>hasIntegerElements</C> tells whether data has to be
         * rounded to integer
         *
         * @param[in] length            the count of base points
         * @param[in] kind              the kind (sine or triangle)
         * @param[in] minimumValue      the minimum y-value in waveform
         * @param[in] maximumValue      the maximum y-value in waveform
         * @param[in] phase             the initial phase (in radians)
         * @param[in] hasIntegerValues  tells whether waveform only may have
         *                              integer values as y-values
         */
        void set (IN Real length, IN WaveFormKind kind,
                  IN Real minimumValue, IN Real maximumValue,
                  IN Radians phase, IN Boolean hasIntegerValues);

        /*--------------------*/
        /* property access    */
        /*--------------------*/

        /**
         * Returns current value of waveform.
         *
         * @return value of waveform at current pointer
         */
        Real current () const;

        /*--------------------*/

        /**
         * Gets state of iteration in wave form
         * 
         * @return  state of iterator in wave form
         */
        WaveFormIteratorState state () const;

        /*--------------------*/
        /* property change    */
        /*--------------------*/

        /**
         * Resets access to wave form to its beginning
         */
        void reset ();

        /*--------------------*/

        /**
         * Sets state of iteration in wave form.
         *
         * @param[in] state  new state of wave form
         */
        void setState (IN WaveFormIteratorState state);

        /*--------------------*/

        /**
         * Advances access to wave form to next sample position
         */
        void advance ();

        /*--------------------*/
        /* time lock service  */
        /*--------------------*/

        /**
         * Calculates time locked delta phase for position
         * <C>currentTime</C> from <C>timeOffset</C> and
         * <C>frequency</C>; this gives the phase shift within a wave
         * form oscillating with <C>frequency</C> being at phase 0 for
         * <C>timeOffset</C> for the time given by <C>currentTime</C>,
         * e.g. for a frequency of 0.25Hz when <C>currentTime</C> is
         * <C>timeOffset</C>+1s, the phase shift is pi/4
         *
         * @param[in] frequency    frequency of wave form
         * @param[in] timeOffset   time offset of wave form
         * @param[in] currentTime  current time for phase to be calculated
         * @return phase in radians
         */
        static Radians phaseByTime (IN Real frequency,
                                    IN Real timeOffset,
                                    IN Real currentTime);

        /*--------------------*/
        /*--------------------*/

        protected:

            /** an untyped pointer to the internal data */
            Object _internalData;

    };

}
