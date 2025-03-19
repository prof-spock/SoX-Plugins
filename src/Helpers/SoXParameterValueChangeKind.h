/**
 * @file
 * The <C>SoXParameterValueChangeKind</C> specification defines a
 * parameter change kind enumeration for SoX plugins.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

namespace SoXPlugins::Helpers {

    /** kind of change in a single parameter value to be reported to
     * observers with its consequences (e.g. a global change of all
     * parameter data) */
    enum class SoXParameterValueChangeKind : short {

        /** the global change in all parameters */
        globalChange = 1,

        /** the page change in the parameter set */
        pageChange = 2,

        /** the change of the page count in the parameter set */
        pageCountChange = 3,

        /** a simple change of a single parameter in the parameter
         * set */
        parameterChange = 4,

        /** no significant */
        noChange = 5

    };

}

/*--------------------*/

/**
 * Returns the string representation of <C>kind</C>.
 *
 * @param kind  audio value change kind
 * @return string representation
 */
#define SoXParameterValueChangeKind_toString(kind) \
    String((short)(kind) == 1 ? "globalChange" \
           : ((short)(kind) == 2 ? "pageChange" \
              : ((short)(kind) == 3 ? "pageCountChange" \
                 : ((short)(kind) == 4 ? "parameterChange"      \
                    : "noChange"))))
