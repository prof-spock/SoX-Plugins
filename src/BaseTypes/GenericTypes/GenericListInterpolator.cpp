/**
 * @file
 * The <C>GenericListInterpolator</C> body implements a wrapper
 * object doing an interpolation on a list of values.
 *
 * @author Dr. Thomas Tensi
 * @date   2025-04
 */

/*====================*/

/*=========*/
/* IMPORTS */
/*=========*/

#include "GenericListInterpolator.h"

/*====================*/

namespace BaseTypes::GenericTypes {

    String pointLocationToString (IN ListInterpolatorPointLocation p)
    {
        String result = 
            (p == ListInterpolatorPointLocation::atBegin
             ? "atBegin"
             : (p == ListInterpolatorPointLocation::atCenter
                ? "atCenter"
                : "atEnd"));
        return result;
    }

}
