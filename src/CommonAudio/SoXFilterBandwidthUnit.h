/**
 * @file
 * The <C>SoxFilterBandwidthUnit</C> specification and body provides
 * the enumeration type for the different units of a IIR filter.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

namespace SoXPlugins::CommonAudio {

    /**
     * The <C>SoxFilterBandwidthUnit</C> type is the enumeration of
     * the different units of a IIR filter.
     */
    enum class SoXFilterBandwidthUnit : int {
        frequency, octaves, quality, butterworth, slope
    };

};
