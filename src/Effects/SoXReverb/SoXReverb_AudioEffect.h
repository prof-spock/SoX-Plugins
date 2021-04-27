/**
 * @file
 * The <C>SoXReverb_AudioEffect</C> specification defines the SoX
 * <B>reverb</B> plugin as a simple reverb based on freeverb.
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

using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::CommonAudio::SoXAudioEffect;
using SoXPlugins::CommonAudio::SoXAudioSampleBuffer;
using SoXPlugins::CommonAudio::SoXAudioValueChangeKind;

/*====================*/

namespace SoXPlugins::Effects::SoXReverb {

    /**
     * A <C>SoXReverb_AudioEffect</C> object models the SoX
     * <B>reverb</B> plugin as a simple reverb implementing the
     * Freeverb algorithm.
     */
    struct SoXReverb_AudioEffect  : public SoXAudioEffect {

        /*--------------------*/
        /* setup              */
        /*--------------------*/

        SoXReverb_AudioEffect ();

        /*-----------------------*/
        /* string representation */
        /*-----------------------*/

        String toString () const override;

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        String name () const;

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
