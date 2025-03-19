/**
 * @file
 * The <C>SoXReverb_AudioEffect</C> specification defines the SoX
 * <B>reverb</B> plugin as a simple reverb based on freeverb.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-10
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "Boolean.h"
#include "SoXAudioEffect.h"

/*--------------------*/

using BaseTypes::Primitives::Boolean;
using SoXPlugins::Effects::SoXAudioEffect;

/*====================*/

namespace SoXPlugins::Effects::SoXReverb {

    /**
     * A <C>SoXReverb_AudioEffect</C> object models the SoX
     * <B>reverb</B> plugin as a simple reverb implementing the
     * Freeverb algorithm.
     */
    struct SoXReverb_AudioEffect : public SoXAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXReverb_AudioEffect ();

        /*--------------------*/

        ~SoXReverb_AudioEffect () override;

        /*-----------------------*/
        /* string representation */
        /*-----------------------*/

        String toString () const override;

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        String name () const override;

        /*--------------------*/

        Real tailLength () const override;

        /*--------------------*/
        /* parameter change   */
        /*--------------------*/

        void setDefaultValues () override;

        /*--------------------*/
        /* event handling     */
        /*--------------------*/

        void prepareToPlay (IN Real sampleRate) override;

        /*--------------------*/

        void processBlock (IN Real timePosition,
                           INOUT AudioSampleListVector& buffer)
            override;

        /*--------------------*/
        /*--------------------*/

        protected:

            String _effectDescriptorToString () const
                override;

            /*--------------------*/

            SoXParameterValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value,
                               IN Boolean recalculationIsForced)
                override;

    };

}
