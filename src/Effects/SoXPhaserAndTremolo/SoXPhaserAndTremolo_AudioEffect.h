/**
 * @file
 * The <C>SoXPhaserAndTremolo_AudioEffect</C> specification defines
 * the SoX <B>phaser</B> and <B>tremolo</B> modulation effects that
 * have similar parameters and hence are combined into a single
 * plugin.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-09
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

namespace SoXPlugins::Effects::SoXPhaserAndTremolo {

    /**
     * A <C>SoXPhaserAndTremolo_AudioEffect</C> object models the SoX
     * <B>phaser</B> and <B>tremolo</B> modulation effects that have
     * similar parameters and hence are combined into a single plugin.
     */
    struct SoXPhaserAndTremolo_AudioEffect : public SoXAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXPhaserAndTremolo_AudioEffect ();

        /*--------------------*/

        ~SoXPhaserAndTremolo_AudioEffect ();

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
