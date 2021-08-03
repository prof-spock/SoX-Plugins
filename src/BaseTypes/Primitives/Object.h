/**
 * @file
 * The <C>Object</C> specification and body provides the root object
 * type.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
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
    using Object = void*;

    /*--------------------*/

    /**
     * The <C>ObjectSet</C> type represents sets of (pointer) objects.
     */
    using ObjectSet = std::set<Object>;

}
