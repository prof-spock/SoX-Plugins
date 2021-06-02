/**
 * @file
 * The <C>SoXCompander_AudioEffect</C> specification defines the SoX
 * <B>compand</B> and <B>mcompand</B> audio effects; a simple
 * compander is just seen as a multiband compander with a single band.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Real.h"
#include "SoXAudioEffect.h"
#include "SoXAudioValueChangeKind.h"
#include "MyString.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::CommonAudio::SoXAudioEffect;
using SoXPlugins::CommonAudio::SoXAudioSampleBuffer;
using SoXPlugins::CommonAudio::SoXAudioValueChangeKind;

/*====================*/

namespace SoXPlugins::Effects::SoXCompander {

    /**
     * A <C>SoXCompander_AudioEffect</C> object models the SoX
     * <B>compand</B> and <B>mcompand</B> audio effects; a simple
     * compander is just seen as a multiband compander with a single
     * band.
     */
    struct SoXCompander_AudioEffect : public SoXAudioEffect {

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        SoXCompander_AudioEffect ();

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

        void prepareToPlay (Real sampleRate)
            override;

        /*--------------------*/

        void processBlock (Real timePosition,
                           SoXAudioSampleBuffer& buffer)
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
