/**
 * @file
 * The <C>Dictionary</C> specification defines maps from string keys
 * to string values.
 */

#pragma once

/*====================*/

#include <map>

#include "GlobalMacros.h"
#include "MyString.h"
#include "StringList.h"

/*====================*/

using std::map;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::BaseTypes::Containers::StringList;

/*====================*/

namespace SoXPlugins::BaseTypes::Containers {

    /**
     * A <C>Dictionary</C> object maps string keys to string values.
     * Every key is associated with at most one value, where the key
     * identity is defined by standard string comparison.
     */
    struct Dictionary : public map<String,String> {

        /**
         * Makes an empty dictionary.
         */
        Dictionary ();

        /*--------------------*/

        /**
         * Destroys current dictionary.
         */
        ~Dictionary ();

        /*--------------------*/

        /**
         * Makes dictionary from list of strings <C>list</C>
         * taking even-indexed entries as keys and odd-indexed
         * entries as associated values.
         *
         * @param list  list of strings with keys at even and
         *              values at odd indexes
         * @return  dictionary with keys and values as given in
         *          <C>list</C>
         */
        static Dictionary makeFromList (IN StringList& list);

        /*--------------------*/

        /**
         * Returns printable representation of dictionary.
         *
         * @return string representation of dictionary
         */
        String toString () const;

        /*--------------------*/

        /**
         * Returns entry for <C>st</C> or otherwise
         * <C>defaultValue</C>.
         *
         * @param key           key within dictionary
         * @param defaultValue  value to use when key is not found
         * @return entry for <C>key</C> in dictionary
         */
        String atWithDefault (IN String& key,
                              IN String& defaultValue) const;

        /*--------------------*/

        /**
         * Tells whether dictionary has entry for <C>key</C>.
         *
         * @param key  key to be tested for existence within dictionary
         * @return information whether <C>key</C> is in dictionary
         */
        bool contains (IN String& key) const;

    };

};
