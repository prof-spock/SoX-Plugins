/**
 * @file
 * The <C>SoXPhaserAndTremolo_AudioEffect</C> specification defines
 * the SoX <B>phaser</B> and <B>tremolo</B> modulation effects that
 * have similar parameters and hence are combined into a single
 * plugin.
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

namespace SoXPlugins::Effects::SoXPhaserAndTremolo {

    /**
     * A <C>SoXPhaserAndTremolo_AudioEffect</C> object models the SoX
     * <B>phaser</B> and <B>tremolo</B> modulation effects that have
     * similar parameters and hence are combined into a single plugin.
     */
    struct SoXPhaserAndTremolo_AudioEffect : public SoXAudioEffect {

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        SoXPhaserAndTremolo_AudioEffect ();

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
