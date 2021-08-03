/**
 * @file
 * The <C>SoxAudioSample</C> specification and body provides the
 * definition of the audio sample (a real value).
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include "Real.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Real;

/*====================*/

namespace SoXPlugins::CommonAudio {

    /** type for an audio sample */
    using SoXAudioSample = Real;

}
