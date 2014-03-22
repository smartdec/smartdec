/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <cassert>
#include <memory> /* std::unique_ptr */
#include <utility> /* std::swap */
#include <type_traits> /* std::remove_const */

namespace nc {

/**
 * Base class for the elements of intrusive lists.
 *
 * \tparam Element class derived from ilist_item.
 */
template<class T>
class ilist_item {
    /** Pointer to the next element. */
    T *next_;

    /** Pointer to the previous element. */
    T *prev_;

    template<class X, class Y> friend class ilist;
    template<class X, class Y> friend class ilist_iterator;

protected:
    /**
     * Protected constructor.
     */
    ilist_item(): next_(NULL), prev_(NULL) {}
};

/**
 * Class containing pointers to the first and last element of a list.
 *
 * \tparam T Element type.
 */
template<class T>
class ilist_data {
public:
    /** Pointer to the first element. */
    T *front_;

    /** Pointer to the last element. */
    T *back_;
};

/**
 * Base class for an intrusive list iterator.
 *
 * \tparam T Element type as to be returned to the user.
 * \tparam U Element type as given to the container.
 */
template<class T, class U = T>
class ilist_iterator: public std::iterator<std::bidirectional_iterator_tag, T> {
    /** Pointer to the element the iterator points to. */
    T *element_;

    /** Reference to the data of the list being iterated. */
    const ilist_data<U> &list_;

    template<class X, class Y> friend class ilist_iterator;

public:
    /**
     * Constructor.
     *
     * \param list      Reference to the data of the list being iterated.
     * \param element   Pointer to the element this iterator points to.
     */
    explicit ilist_iterator(const ilist_data<U> &list, T *element = NULL):
        element_(element), list_(list)
    {}

    /**
     * Constructor from a non-const iterator.
     *
     * \param that Iterator to construct from.
     */
    ilist_iterator(const ilist_iterator<typename std::remove_const<T>::type, U> &that):
        element_(that.element_), list_(that.list_)
    {}

    /**
     * Prefix increment.
     */
    ilist_iterator &operator++() {
        if (element_ != NULL) {
            element_ = element_->next_;
        } else {
            element_ = list_.front_;
        }
        return *this;
    }

    /**
     * Postfix increment.
     */
    ilist_iterator operator++(int) {
        auto copy = *this;
        ++*this;
        return copy;
    }

    /**
     * Prefix decrement.
     */
    ilist_iterator &operator--() {
        if (element_ != NULL) {
            element_ = element_->prev_;
        } else {
            element_ = list_.back_;
        }
        return *this;
    }

    /**
     * Postfix decrement.
     */
    ilist_iterator operator--(int) {
        auto copy = *this;
        --*this;
        return copy;
    }

    /**
     * \returns True if *this and that point to the same element, false otherwise.
     */
    bool operator==(const ilist_iterator &that) const {
        return this->element_ == that.element_;
    }

    /**
     * \returns True if *this and that point to different elements, false otherwise.
     */
    bool operator!=(const ilist_iterator &that) const {
        return !(*this == that);
    }

    /**
     * \return Pointer to the element being pointed to.
     */
    T *operator->() const {
        return element_;
    }

    /**
     * \return Pointer to the element being pointed to.
     */
    T *operator*() const {
        return element_;
    }
};

/**
* Intrusive singly linked list owning its elements.
*
* \param T       Element type.
* \param Deleter Deleter type.
*/
template<class T, class Deleter = std::default_delete<T>>
class ilist: private ilist_data<T> {
    using ilist_data<T>::front_;
    using ilist_data<T>::back_;

public:
    /**
     * Type of the elements.
     */
    typedef T value_type;

    /**
     * Type of the deleter for the list elements.
     */
    typedef Deleter deleter_type;

    /**
     * Unique pointer type for the elements.
     */
    typedef std::unique_ptr<value_type, deleter_type> unique_ptr;

    /**
     * Iterator type.
     */
    typedef ilist_iterator<value_type, value_type> iterator;

    /**
     * Constant iterator type.
     */
    typedef ilist_iterator<const value_type, value_type> const_iterator;

    /**
     * Reverse iterator type.
     */
    typedef std::reverse_iterator<iterator> reverse_iterator;

    /**
     * Constant reverse iterator type.
     */
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    /**
     * Constructs an empty list.
     *
     * \param deleter Deleter used for the destruction of managed elements.
     */
    ilist(const deleter_type &deleter = deleter_type()):
        deleter_(deleter)
    {
        front_ = NULL;
        back_ = NULL;
    }

    /**
     * Copy-construction is forbidden.
     */
    ilist(const ilist &) = delete;

    /**
     * Move constructor.
     *
     * \param that The list to move from.
     */
    ilist(ilist &&that):
        deleter_(std::move(that.deleter_))
    {
        front_ = that.front_;
        back_ = that.back_;

        that.front_ = NULL;
        that.back_ = NULL;
    }

    /**
     * Destructor.
     *
     * Destroys all the elements in the list.
     */
    ~ilist() { clear(); }

    /**
     * Copy-assignment is forbidden.
     */
    ilist &operator=(const ilist &) = delete;

    /**
     * Move-assignment operator.
     *
     * \param that List to move from.
     */
    ilist &operator=(ilist &&that) {
        clear();
        swap(that);
        return *this;
    }

    /**
     * Swaps *this and that lists.
     *
     * \param that Another list.
     */
    void swap(ilist &that) {
        std::swap(front_, that.front_);
        std::swap(back_, that.back_);
        std::swap(deleter_, that.deleter_);
    }

    /**
     * \return Deleter used for the destruction of managed elements.
     */
    deleter_type &get_deleter() { return deleter_; }

    /**
     * \return Deleter used for the destruction of managed elements.
     */
    const deleter_type &get_deleter() const { return deleter_; }

    /**
     * \return Pointer to the first element. Will be NULL if the list is empty.
     */
    value_type *front() { return front_; }

    /**
     * \return Pointer to the first element. Will be NULL if the list is empty.
     */
    const value_type *front() const { return front_; }

    /**
     * \return Pointer to the last element. Will be NULL if the list is empty.
     */
    value_type *back() { return back_; }

    /**
     * \return Pointer to the last element. Will be NULL if the list is empty.
     */
    const value_type *back() const { return back_; }

    /**
     * \return True if the list is empty, false otherwise.
     */
    bool empty() const { return front() == NULL; }

    /**
     * Clears the list. Destroys all the elements.
     */
    void clear() {
        while (!empty()) {
            pop_front();
        }
    }

    /**
     * Removes given element from the list.
     *
     * \param element Valid pointer to an element in the list.
     *
     * \return Valid pointer to the deleted element.
     */
    unique_ptr erase(value_type *element) {
        assert(element != NULL);

        if (element == front_) {
            front_ = front_->next_;
        }
        if (element == back_) {
            back_ = back_->prev_;
        }

        element->next_ = NULL;
        element->prev_ = NULL;

        return unique_ptr(element);
    }

    /**
     * Removes the front element from the list.
     *
     * \return Valid pointer to the erased element.
     */
    unique_ptr pop_front() {
        return erase(front());
    }

    /**
     * Removes the back element from the list.
     *
     * \return Valid pointer to the erased element.
     */
    unique_ptr pop_back() {
        return erase(back());
    }

    /**
     * Inserts the element in the list at the given position.
     *
     * \param position Iterator identifying the position.
     * \param element Element to insert.
     */
    value_type *insert(const_iterator position, unique_ptr element) {
        assert(element);
        assert(element->next_ == NULL);
        assert(element->prev_ == NULL);

        if (position == begin()) {
            element->next_ = front_;
            front_ = element.get();
            if (back_ == NULL) {
                back_ = front_;
            }
        } else if (position == end()) {
            element->prev_ = back_;
            back_ = element.get();
            if (front_ == NULL) {
                front_ = back_;
            }
        } else {
            element->next_ = const_cast<value_type *>(*position);
            element->next_->prev_ = element.get();
            element->prev_ = position->prev_;
            element->prev_->next_ = element.get();
        }

        return element.release();
    }

    /**
     * Inserts given element to the front of the list.
     *
     * \param element Valid pointer to the element.
     */
    value_type *push_front(unique_ptr element) {
        return insert(begin(), std::move(element));
    }

    /**
     * Inserts given element to the back of the list.
     *
     * \param element Valid pointer to the element.
     */
    value_type *push_back(unique_ptr element) {
        return insert(end(), std::move(element));
    }

    /**
     * \param first First element of the range to be cut out.
     * \param last Next after the last element of the range to be cut out.
     *
     * \return List containing the range being cut out.
     */
    ilist cut_out(const_iterator first, const_iterator last) {
        ilist result;

        if (first == last) {
            return result;
        }

        result.front_ = const_cast<value_type *>(*first);
        result.back_ = const_cast<value_type *>(*--last);

        assert(result.front_ != NULL);
        assert(result.back_ != NULL);

        if (result.front_ == front_) {
            front_ = result.back_->next_;
            if (front_) {
                front_->prev_ = NULL;
            }
        }

        if (result.back_ == back_) {
            back_ = result.front_->prev_;
            if (back_) {
                back_->next_ = NULL;
            }
        }

        result.front_->prev_ = NULL;
        result.back_->next_ = NULL;

        return result;
    }

    /**
     * \return Iterator pointing to the first element.
     */
    iterator begin() { return iterator(*this, front()); }

    /**
     * \return Iterator pointing to the next after the last element.
     */
    iterator end() { return iterator(*this); }

    /**
     * \return Constant iterator to the first element.
     */
    const_iterator begin() const { return cbegin(); }

    /**
     * \return Constant iterator to the next after the last element.
     */
    const_iterator end() const { return cend(); }

    /**
     * Reverse iterator pointing to the last element.
     */
    reverse_iterator rbegin() { return reverse_iterator(end()); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    reverse_iterator rend() { return reverse_iterator(begin()); }

    /**
     * Reverse iterator pointing to the last element.
     */
    const_reverse_iterator rbegin() const { return crbegin(); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    const_reverse_iterator rend() const { return crend(); }

    /**
     * \return Constant iterator to the first element.
     */
    const_iterator cbegin() const { return const_iterator(*this, front()); }

    /**
     * \return Constant iterator to the next after the last element.
     */
    const_iterator cend() const { return const_iterator(*this); }

    /**
     * Reverse iterator pointing to the last element.
     */
    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    /**
     * \param element Valid pointer to an element of the list.
     *
     * \return Iterator pointing to the given element.
     */
    iterator get_iterator(value_type *element) {
        assert(element != NULL);
        return iterator(*this, element);
    }

    /**
     * \param element Valid pointer to an element of the list.
     *
     * \return Constant iterator pointing to the given element.
     */
    const_iterator get_iterator(const value_type *element) const {
        assert(element != NULL);
        return const_iterator(*this, element);
    }

private:
    /**
     * Deleter.
     */
    deleter_type deleter_;
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
