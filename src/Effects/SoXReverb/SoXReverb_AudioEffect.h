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

        void recalculate () override;

        /*--------------------*/
        /*--------------------*/

        protected:

            /*-----------------------*/
            /* type conversion       */
            /*-----------------------*/

            String _effectDescriptorToString () const
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

    };

}
