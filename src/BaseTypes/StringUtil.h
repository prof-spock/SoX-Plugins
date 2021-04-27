/**
 * @file
 * The <C>StringUtil</C> specification defines several string routines
 * not included in the standard C++ library (like, for example, prefix
 * calculation, whitespace stripping etc).
 *
 * @author Dr. Thomas Tensi
 */

#pragma once

/*====================*/

#include "GlobalMacros.h"
#include "Natural.h"
#include "Integer.h"
#include "Real.h"
#include "MyString.h"
#include "StringList.h"

/*====================*/

using SoXPlugins::BaseTypes::Primitives::Integer;
using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::Real;
using SoXPlugins::BaseTypes::Primitives::String;
using SoXPlugins::BaseTypes::Containers::StringList;

/*====================*/

/**
 * The <C>StringUtil</C> package provides several string routines not
 * included in the standard C++ library (like, for example, prefix
 * calculation, whitespace stripping etc).
 */
namespace SoXPlugins::BaseTypes::StringUtil {

    /**
     * Returns character at <C>position</C> in <C>st</C>.
     *
     * @param[in] st        the string to be indexed
     * @param[in] position  the character index (starting at 0)
     * @return  character at position
     */
    char at (IN String& st, IN Natural position);

    /*--------------------*/

    /**
     * Tells whether <C>st</C> ends with <C>prefix</C>.
     *
     * @param[in] st      the string to be checked for suffix
     * @param[in] suffix  the string to be checked for being a suffix
     * @return  information whether st ends with suffix
     */
    bool endsWith (IN String& st, IN String& suffix);

    /*--------------------*/

    /**
     * Expands parameter %i by <C>replacementi</C> in <C>st</C>
     * and returns expanded version
     *
     * @param st            the string with parameters
     * @param replacement01  replacement string for %1
     * @param replacement02  replacement string for %2
     * @param replacement03  replacement string for %3
     * @param replacement04  replacement string for %4
     * @param replacement05  replacement string for %5
     * @param replacement06  replacement string for %6
     * @param replacement07  replacement string for %7
     * @param replacement08  replacement string for %8
     * @param replacement09  replacement string for %9
     * @param replacement10  replacement string for %A
     * @return  expanded version of st
     */
    String expand (IN String& st,
                   IN String& replacement01 = "",
                   IN String& replacement02 = "",
                   IN String& replacement03 = "",
                   IN String& replacement04 = "",
                   IN String& replacement05 = "",
                   IN String& replacement06 = "",
                   IN String& replacement07 = "",
                   IN String& replacement08 = "",
                   IN String& replacement09 = "",
                   IN String& replacement10 = "");

    /*--------------------*/

    /**
     * Tells whether <C>st</C> represents an integer.
     *
     * @param st  the string with an integer value
     * @return  information whether string represents an integer or
     *          not
     */
    bool isInt (IN String& st);

    /*--------------------*/

    /**
     * Tells whether <C>st</C> represents an natural.
     *
     * @param st  the string with a natural value
     * @return  information whether string represents a natural or
     *          not
     */
    bool isNatural (IN String& st);

    /*--------------------*/

    /**
     * Tells whether <C>st</C> represents a real value.
     *
     * @param st  the string with a real value
     * @return  information whether string represents a real or not
     */
    bool isReal (IN String& st);

    /*--------------------*/

    /**
     * Gives last character of <C>st</C>
     */
    char lastChar (IN String& st);

    /*--------------------*/

    /**
     * Returns prefix with at most <C>count</C> characters of
     * <C>st</C>.
     *
     * @param[in] st     the string to be cut
     * @param[in] count  number of characters to extract from front
     * @return  prefix with at most <C>count</C> characters
     */
    String prefix (IN String& st, IN Natural count);

    /*--------------------*/

    /**
     * Replaces all occurrences of <C>pattern</C> in <C>st</C> by
     * <C>replacement</C> (without rescanning replaced parts).
     *
     * @param[inout] st        the string with all patterns to be
     *                         replaced by replacement
     * @param[in] pattern      literal pattern to be searched for
     * @param[in] replacement  replacement string
     */
    void replace (INOUT String& st,
                  IN String& pattern,
                  IN String& replacement);

    /*--------------------*/

    /**
     * Splits <C>st</C> at <C>separator</C> and sets <C>prefix</C>
     * and <C>suffix</C>.  If separator is not found, returns
     * false.
     *
     * @param[in] st         the string to be split
     * @param[in] separator  the separation string
     * @param[out] prefix    the part before the separator
     * @param[out] suffix    the part after the separator
     * @return  information whether st contains separator
     */
    bool splitAt (IN String& st, IN String& separator,
                  OUT String& prefix, OUT String& suffix);

    /*--------------------*/

    /**
     * Tells whether <C>st</C> starts with <C>prefix</C>.
     *
     * @param[in] st      the string to be checked for prefix
     * @param[in] prefix  the string to be checked for being prefix
     * @return  information whether st starts with prefix
     */
    bool startsWith (IN String& st, IN String& prefix);

    /*--------------------*/

    /**
     * Removes white space from beginning and end of <C>st</C>.
     *
     * @param st  the string with whitespace to be replaced
     * @return  sanitized version of string
     */
    String strip (IN String& st);

    /*--------------------*/

    /**
     * Returns substring of <C>st</C> starting at <C>firstIndex</C>
     * with at most <C>count</C> characters.
     *
     * @param[in] st          the string to be cut
     * @param[in] firstIndex  the first position in st (starting at 0)
     * @param[in] count       number of characters to extract
     * @return  substring from <C>firstIndex</C> with at most
     *          <C>count</C> characters
     */
    String substring (IN String& st,
                      IN Natural firstIndex,
                      IN Natural count = Natural::maximumValue());

    /*--------------------*/

    /**
     * Converts <C>st</C> to an int; if conversion fails,
     * <C>defaultValue</C> is returned
     *
     * @param st            the string to be converted to integer
     * @param defaultValue  the value to be used for bad conversion
     * @return  integer value of string or default
     */
    Integer toInteger (IN String& st,
                       IN Integer defaultValue = Integer::maximumValue());

    /*--------------------*/

    /**
     * Converts <C>st</C> to a natural; if conversion fails, maximum
     * natural is returned
     *
     * @param st  the string to be converted to natural
     * @return  natural value of string or maximum natural
     */
    Natural toNatural (IN String& st);

    /*--------------------*/

    /**
     * Converts <C>st</C> to a natural; if conversion fails,
     * <C>defaultValue</C> is returned
     *
     * @param st            the string to be converted to natural
     * @param defaultValue  the value to be used for bad conversion
     * @return  natural value of string or default
     */
    Natural toNatural (IN String& st, IN Natural defaultValue);

    /*--------------------*/

    /**
     * Converts <C>st</C> to a real value; if conversion fails,
     * NaN is returned
     *
     * @param st  the string to be converted to real
     * @return  real value of string or NaN
     */
    Real toReal (IN String& st);

    /*--------------------*/

    /**
     * Converts <C>st</C> to a real value; if conversion fails,
     * <C>defaultValue</C> is returned
     *
     * @param st            the string to be converted to real
     * @param defaultValue  the value to be used for bad conversion
     * @return  real value of string or default
     */
    Real toReal (IN String& st, IN Real defaultValue);

    /*--------------------*/

    /**
     * Converts bool value <C>b</C> to string.
     *
     * @param b  boolean value to be converted
     * @return  string representation of bool
     */
    String toString (IN bool& b);

    /*--------------------*/

    /**
     * Converts real value <C>r</C> to string.
     *
     * @param r  real value to be converted
     * @return  string representation of real
     */
    String toString (IN Real& r);

    /*--------------------*/

    /**
     * Converts integer <C>i</C> to string.
     *
     * @param i             integer value to be converted
     * @return  string representation of integer
     */
    String toString (IN Integer& i);

    /*--------------------*/

    /**
     * Converts integer <C>i</C> to string with precision and
     * padding information.
     *
     * @param i             integer value to be converted
     * @param precision     minimum number of valid digits
     * @param padCharacter  character to use for left padding
     * @return  string representation of integer
     */
    String toString (IN Integer& i,
                     IN Natural precision,
                     IN String padCharacter = "0");

    /*--------------------*/

    /**
     * Converts natural value <C>n</C> to string.
     *
     * @param n  natural value to be converted
     * @return  string representation of natural
     */
    String toString (IN Natural& n);

    /*--------------------*/

    /**
     * Converts natural value <C>n</C> to string with precision
     * and padding information.
     *
     * @param n  natural value to be converted
     * @param precision     minimum number of valid digits
     * @param padCharacter  character to use for left padding
     * @return  string representation of natural
     */
    String toString (IN Natural& n,
                     IN Natural precision,
                     IN String padCharacter = "0");

    /*--------------------*/

    /**
     * Converts string <C>st</C> to string literal representation.
     *
     * @param st  the string to be converted to a printable string
     * @return  printable value of string
     */
    String toPrintableString (IN String& st);

    /*--------------------*/
    /* GENERIC FUNCTIONS  */
    /*--------------------*/

    /** the separator between key an value in a map */
    static const String keyValueSeparator = " -> ";

    /*--------------------*/

    /**
     * Returns string representation of list <C>list</C> with type
     * name <C>typeName</C> where elements are comma-separated.
     *
     * @param typeName  name of list type
     * @param list      list of strings
     * @return  single string representation of list
     */
    inline String _combineListElements (IN String& typeName,
                                        IN StringList& list)
    {
        const String separator = ", ";
        return typeName + "(" + list.join(separator) + ")";
    }

    /*--------------------*/

    /**
     * Converts list <C>list</C> to linear string representation
     * prefixed by <C>typeName</C> assuming an explicit conversion
     * from element type to string
     *
     * @param typeName  name of list type
     * @param list      list of strings
     * @return  single string representation of list
     */
    template<typename List_T,
             typename Element_T,
             String elementToStringProc(IN Element_T&)>
    String listToStringExplicit (IN String& typeName,
                                 IN List_T& list)
    {
        StringList printableStringList;

        for (Element_T element : list) {
            printableStringList.append(elementToStringProc(element));
        }

        return _combineListElements(typeName, printableStringList);
    }

    /*--------------------*/

    /**
     * Converts list <C>list</C> to linear string representation
     * prefixed by <C>typeName</C> assuming an object operation
     *
     * @param typeName  name of list type
     * @param list      list of strings
     * @return  single string representation of list
     */
    template<typename List_T, typename Element_T>
    String listToStringImplicit (IN String& typeName,
                                 IN List_T& list)
    {
        StringList printableStringList;

        for (Element_T element : list) {
            printableStringList.append(element.toString());
        }

        return _combineListElements(typeName, printableStringList);
    }

    /*--------------------*/

    /**
     * Converts mapping <C>map</C> to linear string
     * representation prefixed by <C>typeName</C>; assumes key
     * is a string and value has some print function
     *
     * @param typeName  name of list type
     * @param map       map to be converted to string
     *                  representation
     * @return  string representation of map
     */
    template<typename Map_T, typename Value_T,
             String valueToStringProc(IN Value_T&)>
    String stringMapToStringExplicit (IN String& typeName,
                                      IN Map_T& map)
    {
        StringList printableStringList;

        for (auto iterator = map.cbegin();
             iterator != map.cend();  iterator++) {
            const String key   = toPrintableString(iterator->first);
            const String value = valueToStringProc(iterator->second);
            printableStringList
                .append(key + keyValueSeparator + value);
        }

        return _combineListElements(typeName, printableStringList);
    }

};

/*--------------------*/

/** a simple shortcut for the generic toString function in the
 * package */
#define TOSTRING SoXPlugins::BaseTypes::StringUtil::toString
