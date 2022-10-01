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

        String name () const override;

        /*--------------------*/
        /* parameter change   */
        /*--------------------*/

        void setDefaultValues () override;

        /*--------------------*/
        /* event handling     */
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

            SoXAudioValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value,
                               IN Boolean recalculationIsSuppressed)
                override;

    };

}
