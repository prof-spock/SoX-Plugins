/**
 * @file
 * The <C>StringSet</C> specification defines sets of string values
 * with no duplicates and only simple containment access and the
 * ability to add and remove elements.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "GenericSet.h"
#include "StringList.h"

/*--------------------*/

using BaseTypes::GenericTypes::GenericSet;

/*====================*/

namespace BaseTypes::Containers {

    /**
     * Returns name of string set type
     *
     * @return type name
     */
    String _stringSetTypeName ();
    
    /*--------------------*/

    /**
     * A <C>StringSet</C> object is a set of string values with no
     * duplicates and only simple containment access and the ability
     * to add and remove elements.
     */
    struct StringSet
        : public GenericSet<String,
                            String_toString,
                            _stringSetTypeName > {

        /*--------------------*/
        /* constructors       */
        /*--------------------*/

        /**
         * Makes set from list of elements <C>list</C>.
         *
         * @param list  list of string elements to be included in set
         * @return  set with data from <C>list</C>
         */
        static StringSet makeFromList (IN StringList& list)
        {
            StringSet result = StringSet();

            for (const String& element : list) {
                result.add(element);
            }

            return result;
        }

    };

}
