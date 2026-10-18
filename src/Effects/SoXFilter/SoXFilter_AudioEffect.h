/**
 * @file
 * The <C>SoXFilter_AudioEffect</C> specification defines a universal
 * SoX <B>filter</B> plugin representing different IIR filters:
 * <B>allpass</B>, <B>band</B>, <B>bandpass</B>, <B>bandreject</B>,
 * <B>bass</B>, <B>biquad</B>, <B>equalizer</B>, <B>highpass</B>,
 * <B>lowpass</B> and <B>treble</B>.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-11
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXCombinedAudioEffect.h"

/*--------------------*/

using SoXPlugins::Effects::SoXCombinedAudioEffect;

/*====================*/

namespace SoXPlugins::Effects::SoXFilter {

    /**
     * A <C>SoXFilter_AudioEffect</C> object models several SoX biquad
     * filters and combines <B>allpass</B>, <B>band</B>,
     * <B>bandpass</B>, <B>bandreject</B>, <B>bass</B>, <B>biquad</B>,
     * <B>equalizer</B>, <B>highpass</B>, <B>lowpass</B> and
     * <B>treble</B> into a single plugin.
     */
    struct SoXFilter_AudioEffect : public SoXCombinedAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXFilter_AudioEffect ();

        /*--------------------*/

        ~SoXFilter_AudioEffect () override;

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        String name () const override;

        /*--------------------*/
        /* parameter change   */
        /*--------------------*/

        void setDefaultValues () override;

        /*--------------------*/
        /* event handling     */
        /*--------------------*/

        void recalculate () override;

        /*--------------------*/
        /*--------------------*/

        protected:

            /*-----------------------*/
            /* string representation */
            /*-----------------------*/

            String _effectDescriptorToString () const
                override;

            /*--------------------*/
            /* parameter map      */
            /*--------------------*/

            SoXValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value)
                override;

            /*--------------------*/
            /* event handling     */
            /*--------------------*/

            void
            _applyForFrame (IN AudioDataPointList& inputDataPointList,
                            OUT AudioDataPointList& outputDataPointList)
                override;

            /*--------------------*/

            void _handleDataPointRateChange (IN Real dataPointRate)
                override;

            /*--------------------*/

            void _updateEffectForKind (IN String& effectKindString)
                override;

    };

}
