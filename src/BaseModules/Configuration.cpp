/**
 * @file
 * The <C>Configuration</C> body implements configuration
 * values (like e.g. the maximum element count used when logging
 * lists).
 *
 * @author Dr. Thomas Tensi
 * @date   2026-08
 */

/*====================*/

/*=========*/
/* IMPORTS */
/*=========*/

#include "Configuration.h"

/*--------------------*/

using BaseModules::Configuration;

/*====================*/

static Natural _presentedListElementCount = Natural::maximumValue();

/*--------------------*/
/* property access    */
/*--------------------*/

Natural Configuration::presentedListElementCount ()
{
    return _presentedListElementCount;
}

/*--------------------*/
/* property change    */
/*--------------------*/

void Configuration::setPresentedListElementCount (IN Natural count)
{
    _presentedListElementCount = count;
}
