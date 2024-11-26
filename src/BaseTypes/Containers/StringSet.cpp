/**
 * @file
 * The <C>StringSet</C> body implements sets of string values
 * with no duplicates and only simple containment access and the
 * ability to add and remove elements.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

/*=========*/
/* IMPORTS */
/*=========*/

#include "StringSet.h"

/*====================*/

String BaseTypes::Containers::_stringSetTypeName ()
{
    return "StringSet";
}
