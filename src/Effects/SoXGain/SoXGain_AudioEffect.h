/**
 * @file
 * The <C>SoXGain_AudioEffect</C> specification defines the SoX
 * <B>gain</B> plugin amplifying or attenuating the input signal by a
 * specific dB amount.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-07
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

namespace SoXPlugins::Effects::SoXGain {

    /**
     * A <C>SoXGain_AudioEffect</C> object models the SoX <B>gain</B>
     * plugin amplifying or attenuating the input signal by a specific
     * dB amount.
     */
    struct SoXGain_AudioEffect : public SoXAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXGain_AudioEffect ();

        /*--------------------*/

        ~SoXGain_AudioEffect () override;

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
