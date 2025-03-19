/**
 * @file
 * The <C>SoXFlangerPhaserAndTremolo_AudioEffect</C> specification
 * defines the SoX <B>flanger</B>, <B>phaser</B> and <B>tremolo</B>
 * modulation effects that have similar parameters and hence are
 * combined into a single plugin.
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

namespace SoXPlugins::Effects::SoXFlangerPhaserAndTremolo {

    /**
     * A <C>SoXFlangerPhaserAndTremolo_AudioEffect</C> object models
     * the SoX <B>flanger</B>, <B>phaser</B> and <B>tremolo</B>
     * modulation effects that have similar parameters and hence are
     * combined into a single plugin.
     */
    struct SoXFlangerPhaserAndTremolo_AudioEffect : public SoXAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXFlangerPhaserAndTremolo_AudioEffect ();

        /*--------------------*/

        ~SoXFlangerPhaserAndTremolo_AudioEffect () override;

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
