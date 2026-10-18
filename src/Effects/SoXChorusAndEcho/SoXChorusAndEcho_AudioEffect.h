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
#include "SoXCombinedAudioEffect.h"
#include "MyString.h"

/*--------------------*/

using BaseTypes::Primitives::Real;
using SoXPlugins::Effects::SoXCombinedAudioEffect;

/*====================*/

namespace SoXPlugins::Effects::SoXChorusAndEcho {

    /**
     * A <C>SoXChorusAndEcho_AudioEffect</C> object models the SoX <B>echo</B>
     * and <B>echos</B> audio effects; the difference between them is
     * that <B>echo</B> has parallel taps all fed by the input signal
     * while <B>echos</B> has sequential taps.
     */
    struct SoXChorusAndEcho_AudioEffect : public SoXCombinedAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXChorusAndEcho_AudioEffect ();

        /*--------------------*/

        ~SoXChorusAndEcho_AudioEffect () override;

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
