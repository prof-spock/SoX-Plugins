/**
 * @file
 * The <C>SoXOverdrive_AudioEffect</C> specification defines the SoX
 * <B>overdrive</B> plugin distorting an input signal with a tanh
 * curve.
 *
 * @author Dr. Thomas Tensi
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

namespace SoXPlugins::Effects::SoXOverdrive {

    /**
     * A <C>SoXOverdrive_AudioEffect</C> object models the SoX
     * overdrive plugin amplifying the input signal by a gain amount
     * with tanh distortion and adapting the colour by asymmetric
     * shaping
     */
    struct SoXOverdrive_AudioEffect : public SoXAudioEffect {

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        SoXOverdrive_AudioEffect ();

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
