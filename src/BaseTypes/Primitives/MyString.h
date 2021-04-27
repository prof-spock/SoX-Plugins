/**
 * @file
 * The <C>String</C> specification defines a standard string type.
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
