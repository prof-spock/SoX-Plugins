/**
 * @file
 * The <C>SoXOverdrive_AudioEffect</C> specification defines the SoX
 * <B>overdrive</B> plugin distorting an input signal with a tanh
 * curve.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "SoXAudioEffect.h"

/*--------------------*/

using SoXPlugins::Effects::SoXAudioEffect;

/*====================*/

namespace SoXPlugins::Effects::SoXOverdrive {

    /**
     * A <C>SoXOverdrive_AudioEffect</C> object models the SoX
     * overdrive plugin amplifying the input signal by a gain amount
     * with tanh distortion and adapting the colour by asymmetric
     * shaping
     */
    struct SoXOverdrive_AudioEffect : public SoXAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXOverdrive_AudioEffect ();

        /*--------------------*/

        ~SoXOverdrive_AudioEffect ();

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

            SoXParameterValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value,
                               IN Boolean recalculationIsForced)
                override;

    };

}
