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

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        String name () const override;

        /*--------------------*/
        /* parameter change   */
        /*--------------------*/

        void setDefaultValues () override;

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

    };

}
