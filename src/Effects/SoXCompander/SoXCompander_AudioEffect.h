/**
 * @file
 * The <C>SoXCompander_AudioEffect</C> specification defines the SoX
 * <B>compand</B> and <B>mcompand</B> audio effects; a simple
 * compander is just seen as a multiband compander with a single band.
 *
 * @author Dr. Thomas Tensi
 * @date   2021-01
 */

#pragma once

/*====================*/

#include "Real.h"
#include "SoXAudioEffect.h"
#include "MyString.h"

/*====================*/

using BaseTypes::Primitives::Real;
using SoXPlugins::Effects::SoXAudioEffect;

/*====================*/

namespace SoXPlugins::Effects::SoXCompander {

    /**
     * A <C>SoXCompander_AudioEffect</C> object models the SoX
     * <B>compand</B> and <B>mcompand</B> audio effects; a simple
     * compander is just seen as a multiband compander with a single
     * band.
     */
    struct SoXCompander_AudioEffect : public SoXAudioEffect {

        /*---------------------*/
        /* setup & destruction */
        /*---------------------*/

        SoXCompander_AudioEffect ();

        /*--------------------*/

        ~SoXCompander_AudioEffect () override;

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

        void recalculate () override;

        /*--------------------*/
        /*--------------------*/

        protected:

            /*-----------------------*/
            /* string representation */
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
