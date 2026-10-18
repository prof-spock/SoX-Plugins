/**
 * @file
 * The <C>SoXCombinedAudioEffect</C> specification defines the root
 * class for all SoX audio effects with multiple effect kinds (e.g.
 * the filter effect representing bass, bandreject etc.).
 *
 * @author Dr. Thomas Tensi
 * @date   2026-10
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

namespace SoXPlugins::Effects {

    /**
     * A <C>SoXCombinedAudioEffect</C> object is a generic stereo
     * audio effect with multiple effect kinds from SoX encapsulating
     * the common services of such an effect.
     */
    struct SoXCombinedAudioEffect : public SoXAudioEffect {

        protected:

            /*--------------------*/
            /* event handling     */
            /*--------------------*/

            /**
             * Change the internal effect settings (e.g. the parameter
             * map) based on <C>effectKind</C>.
             *
             * @param[in] effectKind  string representing effect kind
             */
            virtual void _updateEffectForKind (IN String& effectKind) = 0;

    };

}
