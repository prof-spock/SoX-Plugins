/**
 * @file
 * The <C>SoXEffectParameterMap</C> specification defines a map of
 * string keys and associated values of <C>SoXEffectParameterKind</C>;
 * those values can be set and queried: this provides an
 * interpretative coupling between audio processors and audio editors.
 *
 * Entries can be set to active or inactive to provide a complete list
 * of parameters to audio processors.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "Dictionary.h"
#include "Real.h"
#include "StringSet.h"

/*--------------------*/

using BaseTypes::Containers::Dictionary;
using BaseTypes::Containers::StringSet;
using BaseTypes::Primitives::Integer;
using BaseTypes::Primitives::Natural;
using BaseTypes::Primitives::Real;

/*====================*/

namespace SoXPlugins::Helpers {

    /**
     * The <C>SoXEffectParameterKind</C> is an enumeration type for
     * the kind of parameter for an audio processor parameter.
     */
    enum class SoXEffectParameterKind {
        realKind, intKind, enumKind, unknownKind
    };

    /*--------------------*/

    /**
     * Converts <C>kind</C> to a string.
     *
     * @param[in] kind  parameter kind to be converted
     * @return string representation of <C>kind</C>
     */
    String effectParameterKindToString (IN SoXEffectParameterKind& kind);

    /*====================*/

    /**
     * A <C>SoXEffectParameterMap</C> object is a map of string keys
     * and associated values of <C>SoXEffectParameterKind</C>; those
     * values can be set and queried: this provides an interpretative
     * coupling between audio processors and audio editors.
     */
    struct SoXEffectParameterMap {

        /** a string specifying an unknown value as a mapping
         * entry */
        static const String unknownValue;

        /** a string specifying the separator between widget page
         * and widget name */
        static const String widgetPageSeparator;

        /*--------------------*/
        /*--------------------*/

        /**
         * Makes an empty parameter mapping
         */
        SoXEffectParameterMap ();

        /*--------------------*/

        /**
         * Destroys the audio parameter map
         */
        ~SoXEffectParameterMap ();

        /*--------------------*/

        /**
         * Returns printable representation of parameter map.
         *
         * @return printable representation of map
         */
        String toString () const;

        /*--------------------*/
        /* reset              */
        /*--------------------*/

        /**
         * Clears all audio parameter entries.
         */
        void clear();

        /*--------------------*/
        /* property queries   */
        /*--------------------*/

        /**
         * Tells whether <C>parameterName</C> occurs in list of
         * parameter names.
         *
         * @return information whether parameter name is contained
         *         in map
         */
        Boolean contains (IN String& parameterName) const;

        /*--------------------*/

        /**
         * Returns the list of all parameter names in this audio
         * map
         *
         * @return list of parameter names
         */
        StringList parameterNameList () const;

        /*--------------------*/

        /**
         * Returns the mapping of all parameter names in this
         * audio map to their values
         *
         * @return  mapping from parameter names to current values (as
         *          a string)
         */
        Dictionary parameterNameToValueMap () const;

        /*--------------------*/

        /**
         * Returns the parameter kind of <C>parameterName</C>; if
         * parameter name is not in allowed list, unknown is
         * returned
         *
         * @param[in] parameterName  name of parameter to be checked
         *                           for kind
         * @return associated kind for <C>parameterName</C>
         */
        SoXEffectParameterKind kind (IN String& parameterName)
            const;

        /*--------------------*/

        /**
         * Gets list of enumeration values for parameter with
         * <C>parameterName</C> of parameter kind "enumeration"
         * into <C>result</C>; returns empty list when parameter
         * is not an enumeration parameter
         *
         * @param[in]  parameterName  name of parameter queried
         *                            for values
         * @param[out] result         list of allowed enumeration
         *                            values
         */
        void valueRangeEnum (IN String& parameterName,
                             OUT StringList& result) const;

        /*--------------------*/

        /**
         * Gets range of integer values for parameter with
         * <C>parameterName</C> of parameter kind "integer" into
         * <C>lowValue</C>, <C>highValue</C> and <C>delta</C>; returns
         * zero values when parameter is not an integer parameter
         *
         * @param[in]  parameterName  name of parameter queried for
         *                            values
         * @param[out] lowValue       minimum allowed value for
         *                            parameter
         * @param[out] highValue      maximum allowed value for
         *                            parameter
         * @param[out] delta          increment for parameter
         */
        void valueRangeInt (IN String& parameterName,
                            OUT Integer& lowValue,
                            OUT Integer& highValue,
                            OUT Integer& delta) const;

        /*--------------------*/

        /**
         * Gets range of real values for parameter with
         * <C>parameterName</C> of parameter kind "Real" into
         * <C>lowValue</C>, <C>highValue</C> and <C>delta</C>;
         * returns zero values when parameter is not a real
         * parameter
         *
         * @param[in]  parameterName  name of parameter queried for
         *                            values
         * @param[out] lowValue       minimum allowed value for
         *                            parameter
         * @param[out] highValue      maximum allowed value for
         *                            parameter
         * @param[out] delta          increment for parameter
         */
        void valueRangeReal (IN String& parameterName,
                             OUT Real& lowValue,
                             OUT Real& highValue,
                             OUT Real& delta) const;

        /*--------------------*/

        /**
         * Tells whether <C>value</C> is okay for
         * <C>parameterName</C>.
         *
         * @param[in] parameterName  name of parameter queried for
         *                           values
         * @param[in] value          value to be checked
         * @return  information whether value is okay for parameter
         */
        Boolean isAllowedValue (IN String& parameterName,
                                IN String& value) const;

        /*--------------------*/

        /**
         * Adapts <C>value</C> for <C>parameterName</C> to allowed
         * range and tells whether value was okay
         *
         * @param[in]    parameterName  name of parameter queried for
         *                              values
         * @param[inout] value          value for parameter to be
         *                              adapted
         * @return  information whether value is okay
         */
        Boolean adaptValueEnum (IN String& parameterName,
                                INOUT String& value) const;

        /*--------------------*/

        /**
         * Adapts <C>value</C> for <C>parameterName</C> to allowed
         * range and tells whether value was okay
         *
         * @param[in]    parameterName  name of parameter queried for
         *                              values
         * @param[inout] value          value for parameter to be
         *                              adapted
         * @return  information whether value is okay
         */
        Boolean adaptValueInt (IN String& parameterName,
                               INOUT Integer& value) const;

        /*--------------------*/

        /**
         * Adapts <C>value</C> for <C>parameterName</C> to allowed
         * range and tells whether value was okay
         *
         * @param[in]    parameterName  name of parameter queried for
         *                              values
         * @param[inout] value          value for parameter to be
         *                              adapted
         * @return  information whether value is okay
         */
        Boolean adaptValueReal (IN String& parameterName,
                                INOUT Real& value) const;


        /*--------------------*/

        /**
         * Tells whether <C>value</C> for <C>parameterName</C> is
         * different to stored value.
         *
         * @param[in] parameterName   name of affected parameter
         * @param[in] value           value to be stored in parameter
         */
        Boolean valueIsDifferent (IN String& parameterName,
                                  IN String value) const;

        /*--------------------*/
        /* active state       */
        /*--------------------*/

        /**
         * Sets parameter named <C>parameterName</C> to either
         * active or inactive depending on <C>isActive</C>; per
         * default every parameter is initially active
         *
         * @param[in] parameterName  name of parameter to be set
         *                           active or inactive
         * @param[in] isActive       information whether parameter
         *                           shall be active
         */
        void setActiveness (IN String& parameterName,
                            IN Boolean isActive);

        /*--------------------*/

        /**
         * Tells whether parameter named <C>parameterName</C> is
         * active or inactive.
         *
         * @param[in] parameterName  name of parameter to be checked
         *                           for activeness
         * @return information whether parameter is active
         */
        Boolean isActive (IN String& parameterName) const;

        /*--------------------*/

        /**
         * Sets all parameters for names in <C>parameterNameList</C>
         * to either "active" or "inactive" depending on
         * <C>isActive</C>.
         *
         * @param[in] parameterNameList  list of parameter names to
         *                               be set to active or inactive
         * @param[in] isActive           information whether parameters
         *                               shall be set to active
         */
        void setActivenessForNameList (IN StringList parameterNameList,
                                       IN Boolean isActive);

        /*--------------------*/

        /**
         * Sets all parameters in parameterMap on pages past
         * <C>lastPageIndex</C> to "inactive" and before to
         * "active".
         *
         * @param[in] lastPageIndex  index of last active page
         */
        void changeActivenessByPage (IN Natural lastPageIndex);

        /*--------------------*/
        /* data access/change */
        /*--------------------*/

        /**
         * Sets parameter named <C>parameterName</C> to
         * <C>value</C>.  If value has wrong kind, it is ignored.
         *
         * @param[in] parameterName  name of parameter to be set
         * @param[in] value          new value for parameter
         */
        void setValue (IN String& parameterName, IN String& value);

        /*--------------------*/

        /**
         * Sets parameter named <C>parameterName</C> to an unknown
         * value.
         *
         * @param[in] parameterName  name of parameter to be
         *                           invalidated
         */
        void invalidateValue (IN String& parameterName);

        /*--------------------*/

        /**
         * Gets parameter value for parameter with
         * <C>parameterName</C> converted to a string.  If there
         * is none, an unknown value is returned
         *
         * @param[in] parameterName  name of parameter to be
         *                           queried for a value
         * @return  value for given parameter (or "unknown")
         */
        String value (IN String& parameterName) const;

        /*--------------------*/
        /* kind change        */
        /*--------------------*/

        /**
         * Adds parameter named <C>parameterName</C> to internal
         * list and sets its kind to <C>intParameter</C>; upon
         * success any associated value is cleared
         *
         * @param[in] parameterName  name of parameter to be
         *                           defined
         * @param[in] lowValue       minimum allowed value for
         *                           parameter
         * @param[in] highValue      maximum allowed value for
         *                           parameter
         * @param[in] delta          increment for parameter
         */
        void setKindInt (IN String& parameterName,
                         IN Integer lowValue,
                         IN Integer highValue,
                         IN Integer delta);

        /*--------------------*/

        /**
         * Adds parameter named <C>parameterName</C> to internal
         * list and sets its kind to <C>intParameter</C> and its
         * value to <C>value</C>
         *
         * @param[in] parameterName  name of parameter to be
         *                           defined
         * @param[in] lowValue       minimum allowed value for
         *                           parameter
         * @param[in] highValue      maximum allowed value for
         *                           parameter
         * @param[in] delta          increment for parameter
         * @param[in] value          current value for parameter
         */
        void setKindAndValueInt (IN String& parameterName,
                                 IN Integer lowValue,
                                 IN Integer highValue,
                                 IN Integer delta,
                                 IN Integer value);

        /*--------------------*/

        /**
         * Adds parameter named <C>parameterName</C> to internal
         * list and sets its kind to <C>realParameter</C>; upon
         * success any associated value is cleared
         *
         * @param[in] parameterName  name of parameter to be
         *                           defined
         * @param[in] lowValue       minimum allowed value for
         *                           parameter
         * @param[in] highValue      maximum allowed value for
         *                           parameter
         * @param[in] delta          increment for parameter
         */
        void setKindReal (IN String& parameterName,
                          IN Real lowValue,
                          IN Real highValue,
                          IN Real delta);

        /*--------------------*/

        /**
         * Adds parameter named <C>parameterName</C> to internal
         * list and sets its kind to <C>realParameter</C> and its
         * value to <C>value</C>
         *
         * @param[in] parameterName  name of parameter to be
         *                           defined
         * @param[in] lowValue       minimum allowed value for
         *                           parameter
         * @param[in] highValue      maximum allowed value for
         *                           parameter
         * @param[in] delta          increment for parameter
         * @param[in] value          current value for parameter
         */
        void setKindAndValueReal (IN String& parameterName,
                                  IN Real lowValue,
                                  IN Real highValue,
                                  IN Real delta,
                                  IN Real value);

        /*--------------------*/

        /**
         * Adds parameter named <C>parameterName</C> to internal
         * list and sets its kind to <C>enumParameter</C> with
         * values given by <C>valueList</C>; upon success any
         * associated value is cleared
         *
         * @param[in] parameterName  name of parameter to be defined
         * @param[in] valueList      list of allowed enumeration
         *                           values for parameter
         */
        void setKindEnum (IN String& parameterName,
                          IN StringList& valueList);

        /*--------------------*/

        /**
         * Adds parameter named <C>parameterName</C> to internal
         * list and sets its kind to <C>enumParameter</C> with
         * values given by <C>valueList</C> and its value to
         * <C>value</C>
         *
         * @param[in] parameterName  name of parameter to be defined
         * @param[in] valueList      list of allowed enumeration
         *                           values for parameter
         * @param[in] value          current value for parameter
         */
        void setKindAndValueEnum (IN String& parameterName,
                                  IN StringList& valueList,
                                  IN String& value);

        /*---------------------------*/
        /* parameter names for pages */
        /*---------------------------*/

        /**
         * Makes effective parameter name from
         * <C>parameterName</C> and <C>pageIndex</C>.
         *
         * @param[in] parameterName           original parameter name
         * @param[in] pageIndex               the page index for this
         *                                    parameter
         * @return adapted parameter name
         */
        static
        String pagedParameterName (IN String parameterName,
                                   IN Natural pageIndex);

        /*--------------------*/

        /**
         * Splits <C>parameterName</C> into <C>pageIndex</C> and
         * <C>effectiveParameterName</C>.
         *
         * @param[in]  parameterName           parameter name containing
         *                                     page index
         * @param[out] effectiveParameterName  the effective parameter
         *                                     name without page index
         * @param[out] pageIndex               the page index for this
         *                                     parameter
         */
        static
        void splitParameterName (IN String parameterName,
                                 OUT String& effectiveParameterName,
                                 OUT Natural& pageIndex);

        /*--------------------*/

        /**
         * Splits <C>parameterName</C> into <C>pageIndex</C>,
         * <C>effectiveParameterName</C> and
         * <C>nominalPageIndex</C>.
         *
         * @param[in]  parameterName           parameter name containing
         *                                     page index
         * @param[out] effectiveParameterName  the effective parameter
         *                                     name without page index
         * @param[out] pageIndex               the page index for this
         *                                     parameter
         * @param[out] nominalPageIndex        the nominal page index
         *                                     for this parameter
         */
        static
        void splitParameterName (IN String parameterName,
                                 OUT String& effectiveParameterName,
                                 OUT Natural& pageIndex,
                                 OUT Integer& nominalPageIndex);

        /*--------------------*/

        /**
         * Tells whether parameter given by <C>parameterName</C>
         * is the page selector.
         *
         * @param[in] parameterName  name of parameter to be checked
         * @return  tells whether this parameter is the page selector
         */
        static Boolean isPageSelector (IN String parameterName);

        /*--------------------*/
        /*--------------------*/

        private:

            /** the name list of the parameters */
            StringList _parameterNameList;

            /** the mapping from parameter name to associated
             * values */
            Dictionary _parameterNameToValueMap;

            /** the mapping from parameter name to value range
             *  (encoded as a string) */
            Dictionary _parameterNameToValueRangeMap;

            /** the mapping from parameter name to audio parameter
             *  kind */
            GenericMap<String, SoXEffectParameterKind> _parameterNameToKindMap;

            /** the set of parameter names considered active */
            StringSet _activeParameterNameSet;

    };

}
