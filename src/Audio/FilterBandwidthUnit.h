/**
 * @file
 * The <C>FilterBandwidthUnit</C> specification and body provides
 * the enumeration type for the different units of a IIR filter.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*====================*/

namespace Audio {

    /**
     * The <C>FilterBandwidthUnit</C> type is the enumeration of
     * the different units of a IIR filter.
     */
    enum class FilterBandwidthUnit : int {
        frequency, octaves, quality, butterworth, slope
    };

}
