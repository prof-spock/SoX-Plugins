/**
 * @file
 * The <C>Object</C> specification and body provides the root object
 * type.
 */

#pragma once

/*====================*/

#include <set>

/*====================*/

namespace SoXPlugins::BaseTypes::Primitives {

    /**
     * The <C>Object</C> type is the root class for all pointer
     * objects.
     */
    typedef void* Object;

    /*--------------------*/

    /**
     * The <C>ObjectSet</C> type represents sets of (pointer) objects.
     */
    typedef std::set<Object> ObjectSet;

};
