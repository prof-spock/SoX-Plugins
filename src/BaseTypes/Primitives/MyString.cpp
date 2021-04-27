/**
 * @file
 * The <C>String</C> body implements a standard string type.
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
