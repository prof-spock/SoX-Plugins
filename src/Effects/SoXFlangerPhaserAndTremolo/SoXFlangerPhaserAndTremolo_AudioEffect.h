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

#include "SoXCombinedAudioEffect.h"

/*--------------------*/

using SoXPlugins::Effects::SoXCombinedAudioEffect;

/*====================*/

namespace SoXPlugins::Effects::SoXFlangerPhaserAndTremolo {

    /**
     * A <C>SoXFlangerPhaserAndTremolo_AudioEffect</C> object models
     * the SoX <B>flanger</B>, <B>phaser</B> and <B>tremolo</B>
     * modulation effects that have similar parameters and hence are
     * combined into a single plugin.
     */
    struct SoXFlangerPhaserAndTremolo_AudioEffect
        : public SoXCombinedAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXFlangerPhaserAndTremolo_AudioEffect ();

        /*--------------------*/

        ~SoXFlangerPhaserAndTremolo_AudioEffect () override;

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        Boolean isTimeVariant () const override;

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

        void recalculate () override;

        /*--------------------*/
        /*--------------------*/

        protected:

            /*-----------------------*/
            /* string representation */
            /*-----------------------*/

            String _effectDescriptorToString () const
                override;

            /*-----------------------*/

            String _internalState () const
                override;

            /*--------------------*/
            /* parameter map      */
            /*--------------------*/

            SoXValueChangeKind
            _setValueInternal (IN String& parameterName,
                               IN String& value)
                override;

            /*--------------------*/
            /* event handling     */
            /*--------------------*/

            void
            _applyForFrame (IN AudioDataPointList& inputDataPointList,
                            OUT AudioDataPointList& outputDataPointList)
                override;

            /*--------------------*/

            void _handleDataPointRateChange (IN Real dataPointRate)
                override;

            /*--------------------*/

            void _updateEffectForKind (IN String& effectKindString)
                override;

    };

}
