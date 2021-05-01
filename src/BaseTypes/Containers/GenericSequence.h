/**
 * @file
 * The <C>GenericSequence</C> specification and body defines a
 * generic sequence type of values with zero-based arbitrary indexed
 * access to positions in the sequence.
 *
 * @author Dr. Thomas Tensi
 * @date   2020-08
 */

#pragma once

/*====================*/

#include <vector>

#include "GlobalMacros.h"
#include "Natural.h"
#include "Integer.h"
#include "MyString.h"

/*====================*/

using std::vector;
using SoXPlugins::BaseTypes::Primitives::Natural;
using SoXPlugins::BaseTypes::Primitives::Integer;
using SoXPlugins::BaseTypes::Primitives::String;

/*====================*/

namespace SoXPlugins::BaseTypes::Containers {

    /**
     * A <C>GenericSequence</C> is a generic sequence type of values
     * with zero-based arbitrary indexed access to positions in the
     * sequence.  Indexing starts at zero and is consecutive.
     * Sequences also allow duplicate elements.
     *
     * If the printing of the sequence shall be possible the
     * <C>elementToString</C> function must be provided.
     */
    template <typename T, String elementToString (IN T&) = nullptr>
    struct GenericSequence : public vector<T>
    {

        /**
         * Initializes list to an empty sequence.
         */
        GenericSequence ()
        {
        }

        /*--------------------*/

        /**
         * Initializes list to a sequence of length
         * <C>initialSize</C>.
         *
         * @param[in] initialSize  initial length of sequence
         */
        GenericSequence (IN Natural initialSize)
        {
            setLength(initialSize);
        }

        /*--------------------*/
        /* operators          */
        /*--------------------*/

        /**
         * Returns value reference to sequence at <C>position</C>.
         *
         * @param[in] position  index position
         * @return  sequence value reference at index position
         */
        T& operator [] (IN Natural position)
        {
            return vector<T>::at((size_t) position);
        }

        /*--------------------*/

        /**
         * Returns value in sequence at <C>position</C>.
         *
         * @param[in] position  index position
         * @return  sequence value at index position
         */
        const T& operator [] (IN Natural position) const
        {
            return vector<T>::at((size_t) position);
        }

        /*--------------------*/
        /* Methods            */
        /*--------------------*/

        /**
         * Returns value reference to sequence at <C>position</C>.
         *
         * @param[in] position  index position
         * @return  sequence value reference at index position
         */
        T& at (IN Natural position)
        {
            return vector<T>::at((size_t) position);
        }

        /*--------------------*/

        /**
         * Returns value in sequence at <C>position</C>.
         *
         * @param[in] position  index position
         * @return  sequence value at index position
         */
        const T& at (IN Natural position) const
        {
            return vector<T>::at((size_t) position);
        }

        /*--------------------*/

        /**
         * Appends <C>element</C> as last element.
         * @param[in] element  element to be appended to sequence
         */
        void append (IN T& element)
        {
            vector<T>::emplace_back(element);
        }

        /*--------------------*/

        /**
         * Tells whether current sequence contains <C>element</C>.
         *
         * @param[in] element  element to be searched in sequence
         * @return  information whether <C>element</C> occurs as element in
         *          sequence
         */
        bool contains (IN T& element) const
        {
            return (position(element) >= 0);
        }

        /*--------------------*/

        /**
         * Tells first position where sequence contains <C>element</C>;
         * returns -1 when not found
         *
         * @param[in] element  element to be searched in sequence
         * @return  first index position of <C>element</C> in sequence
         *          (starting with 0) or -1 when not found
         */
        Integer position (IN T& element) const
        {
            Integer result{-1};

            for (Natural i = 0;  i < this->size();  i++) {
                if (at(i) == element) {
                    result = Integer{(int) i};
                    break;
                }
            }

            return result;
        }

        /*--------------------*/

        /**
         * Returns the length of sequence.
         *
         * @return  length of sequence
         */
        Natural length () const
        {
            return Natural{vector<T>::size()};
        }
        
        /*--------------------*/

        /**
         * Sets element at <C>position</C> to <C>value</C>.
         *
         * @param[in] position  index position
         * @param[in] value     new value at position
         */
        void setAt (IN Natural position, IN T value)
        {
            at(position) = value;
        }

        /*--------------------*/

        /**
         * Resizes sequence to <C>newSize</C> elements.
         *
         * @param[in] newSize  new size of sequence
         */
        void setLength (IN Natural newSize)
        {
            vector<T>::resize((size_t) newSize);
        }

        /*--------------------*/

        /**
         * Resizes sequence to <C>newSize</C> elements and fills it
         * with <C>value</C>.
         *
         * @param[in] newSize  new size of sequence
         * @param[in] value    value to be filled into sequence
         */
        void setLength (IN Natural newSize, IN T value)
        {
            vector<T>::resize((size_t) newSize, value);
        }

        /*--------------------*/
        /*--------------------*/

        protected:

            /**
             * Converts sequence to linear string representation
             * prefixed by <C>nameOfType</C> assuming an explicit
             * conversion from element type to string
             *
             * @param nameOfType  name of sequence type
             * @return  single string representation of sequence
             */
            String _toString (IN String nameOfType) const
            {
                String result = "";

                for (const T& element : *this) {
                    result += (result.size() == 0 ? "" : ", ");
                    result += elementToString(element);
                }

                result = nameOfType + "(" + result + ")";
                return result;
            }

    };

};
