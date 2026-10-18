/**
 * @file
 * The <C>SoXValueChangeObserver</C> specification defines a parameter
 * change kind enumeration type and an observer type for value
 * changes.
 *
 * @author Dr. Thomas Tensi
 * @date   2026-08
 */

/*====================*/

#pragma once

/*=========*/
/* IMPORTS */
/*=========*/

#include "Boolean.h"
#include "Dictionary.h"
#include "Object.h"

/*--------------------*/

using BaseTypes::Primitives::Boolean;
using BaseTypes::Containers::Dictionary;
using BaseTypes::Primitives::Object;
using BaseTypes::Primitives::String;

/*====================*/

namespace SoXPlugins::Helpers {

    /** kind of change in a single parameter value to be reported to
     * observers with its consequences (e.g. a global change of all
     * parameter data) */
    enum class SoXValueChangeKind : short {

        /** no significant change */
        noChange = 1,

        /** a simple change of a normal parameter in the parameter
         * set */
        parameterChange = 2,

        /** a global change affecting all parameters */
        globalChange = 3
    };

    /*--------------------*/

    /**
     * Returns the string representation of <C>kind</C>.
     *
     * @param kind  audio value change kind
     * @return string representation
     */
    String SoXValueChangeKind_toString (IN SoXValueChangeKind kind);

    /*====================*/

    /** a callback function type for receiving a value change
     * notification where the value change handler object is given as
     * the object, followed by parameter name and parameter value */
    typedef void (*SoXValueChangeNotification)(INOUT Object,
                                               IN Dictionary&,
                                               IN SoXValueChangeKind);

    /*====================*/

    /**
     * A <C>SoXValueChangeObserver</C> is an object that observes
     * value changes in some underlying object.  It relies on an
     * associated value change notification function that is called
     * whenever a value change happens.
     */
    struct SoXValueChangeObserver {

        /*--------------------*/
        /* creation           */
        /*--------------------*/

        /**
         * Defines handler object and procedure for routing value
         * changes to <C>valueChangeHandler</C> and
         * <C>notificationProc</C>; if handler object is
         * <C>nullptr</C>, any value change is just processed locally,
         * otherwise it is assumed that handler acts on the underlying
         * object somehow (e.g. by changing other values)
         *
         * @param [inout] valueChangeHandler  object processing the
         *                                    value change
         * @param [inout] notificationProc    callback routine called
         *                                    for some value change
         */
        SoXValueChangeObserver
             (Object valueChangeHandler = nullptr,
              SoXValueChangeNotification notificationProc = nullptr);

        /*--------------------*/
        /* property change    */
        /*--------------------*/

        /**
         * Sets handler object and procedure for routing value changes
         * to <C>valueChangeHandler</C> and <C>notificationProc</C>;
         * if handler object is <C>nullptr</C>, any value change is
         * just processed locally, otherwise it is assumed that
         * handler acts on the underlying object somehow (e.g. by
         * changing other values)
         *
         * @param [inout] valueChangeHandler  object processing the
         *                                    value change
         * @param [inout] notificationProc    callback routine called
         *                                    for some value change
         */
        void set (Object valueChangeHandler,
                  SoXValueChangeNotification notificationProc);

        /*--------------------*/
        /* event handling     */
        /*--------------------*/

        /**
         * Tells that all parameter given by keys of <C>dictionary</C>
         * have new value given by the corresponding values.
         *
         * @param [in]  dictionary   mapping from parameter names
         *                           to new values
         * @param [in]  kind         maximum kind of value change
         */
        void notifyAboutValueChange (IN Dictionary& dictionary,
                                     IN SoXValueChangeKind kind);
        
        /*--------------------*/
        /*--------------------*/

        protected:

            /** object processing the value change */
            Object _valueChangeHandler = nullptr;

            /** callback routine called for some value change */
            SoXValueChangeNotification _notificationProc = nullptr;

    };
}
