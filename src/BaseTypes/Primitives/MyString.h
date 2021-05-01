/**
 * @file
 * The <C>String</C> specification defines a standard string type.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include <string>
#include "GlobalMacros.h"

/*====================*/

namespace SoXPlugins::BaseTypes::Primitives {

    /**
     * The <C>String</C> type represents (Unicode) character
     * sequences.
    */
    typedef std::string String;

    /*--------------------*/

    /**
    * Returns simple printable string representation for <C>st</C>.
    * 
    * @param[in] st  string to be converted
    * @return string representation
    */
    String String_toString (IN String& st);

};
