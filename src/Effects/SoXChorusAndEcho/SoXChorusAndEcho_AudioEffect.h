/**
 * @file
 * The <C>SoXChorusAndEcho_AudioEffect</C> specification defines the SoX
 * <B>echo</B> and <B>echos</B> audio effects; the difference between
 * them is that <B>echo</B> has parallel taps all fed by the input
 * signal while <B>echos</B> has sequential taps.
 *
 * @author Dr. Thomas Tensi
 * @date   2024-12
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "Real.h"
#include "SoXAudioEffect.h"
#include "MyString.h"

/*--------------------*/

using BaseTypes::Primitives::Real;
using SoXPlugins::Effects::SoXAudioEffect;

/*====================*/

namespace SoXPlugins::Effects::SoXChorusAndEcho {

    /**
     * A <C>SoXChorusAndEcho_AudioEffect</C> object models the SoX <B>echo</B>
     * and <B>echos</B> audio effects; the difference between them is
     * that <B>echo</B> has parallel taps all fed by the input signal
     * while <B>echos</B> has sequential taps.
     */
    struct SoXChorusAndEcho_AudioEffect : public SoXAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXChorusAndEcho_AudioEffect ();

        /*--------------------*/

        ~SoXChorusAndEcho_AudioEffect () override;

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

        void prepareToPlay (IN Real sampleRate)
            override;

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
