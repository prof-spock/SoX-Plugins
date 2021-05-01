/**
 * @file
 * The <C>SoXGain_AudioEffect</C> specification defines the SoX
 * <B>gain</B> plugin amplifying or attenuating the input signal by a
 * specific dB amount.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-07
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

namespace SoXPlugins::Effects::SoXGain {

    /**
     * A <C>SoXGain_AudioEffect</C> object models the SoX <B>gain</B>
     * plugin amplifying or attenuating the input signal by a specific
     * dB amount.
     */
    struct SoXGain_AudioEffect : public SoXAudioEffect {

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        SoXGain_AudioEffect ();

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
