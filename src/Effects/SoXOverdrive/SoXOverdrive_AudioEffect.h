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

        ~SoXOverdrive_AudioEffect () override;

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

    };

}
