/**
 * @file
 * The <C>String</C> body implements a standard string type.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#include "MyString.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::String;

/*====================*/

namespace SoXPlugins::BaseTypes::Primitives {

    String String_toString (IN String& st)
    {
        return "'" + st + "'";
    }

}
