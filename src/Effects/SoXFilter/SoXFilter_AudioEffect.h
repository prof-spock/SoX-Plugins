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

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Real.h"
#include "MyString.h"
#include "SoXAudioEffect.h"
#include "SoXAudioValueChangeKind.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::CommonAudio::SoXAudioEffect;
using SoXPlugins::CommonAudio::SoXAudioSampleBuffer;
using SoXPlugins::CommonAudio::SoXAudioValueChangeKind;

/*====================*/

namespace SoXPlugins::Effects::SoXFilter {

    /**
     * A <C>SoXFilter_AudioEffect</C> object models several SoX biquad
     * filters and combines <B>allpass</B>, <B>band</B>,
     * <B>bandpass</B>, <B>bandreject</B>, <B>bass</B>, <B>biquad</B>,
     * <B>equalizer</B>, <B>highpass</B>, <B>lowpass</B> and
     * <B>treble</B> into a single plugin.
     */
    struct SoXFilter_AudioEffect : public SoXAudioEffect {

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        SoXFilter_AudioEffect ();

        /*-----------------------*/
        /* string representation */
        /*-----------------------*/

        String toString () const override;

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

        void prepareToPlay (IN Real sampleRate)
            override;

        /*--------------------*/

        void processBlock (IN Real timePosition,
                           INOUT SoXAudioSampleBuffer& buffer)
            override;

        /*--------------------*/
        /*--------------------*/

        protected:

            String _effectDescriptorToString () const
                override;

            /*--------------------*/

            SoXAudioValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value,
                               IN bool recalculationIsSuppressed)
                override;

    };

};
