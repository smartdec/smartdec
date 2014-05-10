/* The file is part of Snowman decompiler.             */
/* See doc/licenses.txt for the licensing information. */

#pragma once

#include <nc/config.h>

#include <cassert>
#include <iterator>
#include <memory> /* std::unique_ptr */
#include <utility> /* std::swap */
#include <type_traits> /* std::remove_const */

#include <boost/noncopyable.hpp>

namespace nc {

/**
 * Base class for the elements of intrusive lists.
 *
 * \tparam T Derived class of list elements.
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
    ilist_item() noexcept: next_(NULL), prev_(NULL) {}

private:
    /**
     * Copying is forbidden.
     */
    ilist_item(const ilist_item &);

    /**
     * Copying is forbidden.
     */
    ilist_item &operator=(const ilist_item &);
};

/**
 * Class containing pointers to the first and last element of a list.
 *
 * \tparam T Type of elements.
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
 * \tparam T Element type as exposed to the user.
 * \tparam U Element type as specified in the container.
 */
template<class T, class U = T>
class ilist_iterator: public std::iterator<std::bidirectional_iterator_tag, T> {
    typedef std::iterator<std::bidirectional_iterator_tag, T> super;

public:
    typedef typename super::value_type value_type;
    typedef typename super::difference_type difference_type;
    typedef typename super::pointer pointer;
    typedef typename super::reference reference;

private:
    /** Pointer to the element the iterator points to. */
    pointer element_;

    /** Reference to the data of the list being iterated. */
    const ilist_data<U> *list_;

    template<class X, class Y> friend class ilist_iterator;

public:
    /**
     * Constructor.
     *
     * \param list      Reference to the data of the list being iterated.
     * \param element   Pointer to the element this iterator points to.
     */
    explicit ilist_iterator(const ilist_data<U> *list, pointer element = NULL) noexcept:
        element_(element), list_(list)
    {
        assert(list != NULL);
    }

    /**
     * Constructor from a non-const iterator.
     *
     * \param that Iterator to construct from.
     */
    ilist_iterator(const ilist_iterator<typename std::remove_const<T>::type, U> &that) noexcept:
        element_(that.element_), list_(that.list_)
    {}

    /**
     * Prefix increment.
     */
    ilist_iterator &operator++() noexcept {
        if (element_ != NULL) {
            element_ = element_->ilist_item<U>::next_;
        } else {
            element_ = list_->front_;
        }
        return *this;
    }

    /**
     * Postfix increment.
     */
    ilist_iterator operator++(int) noexcept {
        auto copy = *this;
        ++*this;
        return copy;
    }

    /**
     * Prefix decrement.
     */
    ilist_iterator &operator--() noexcept {
        if (element_ != NULL) {
            element_ = element_->ilist_item<U>::prev_;
        } else {
            element_ = list_->back_;
        }
        return *this;
    }

    /**
     * Postfix decrement.
     */
    ilist_iterator operator--(int) noexcept {
        auto copy = *this;
        --*this;
        return copy;
    }

    /**
     * \returns True if *this and that point to the same element, false otherwise.
     */
    bool operator==(const ilist_iterator &that) const noexcept {
        return this->element_ == that.element_;
    }

    /**
     * \returns True if *this and that point to different elements, false otherwise.
     */
    bool operator!=(const ilist_iterator &that) const noexcept {
        return !(*this == that);
    }

    /**
     * \return Pointer to the element being pointed to.
     */
    pointer operator->() const noexcept {
        return element_;
    }

    /**
     * \return Pointer to the element being pointed to.
     */
    pointer operator*() const noexcept {
        return element_;
    }
};

/**
* Intrusive doubly linked list owning its elements.
* Features stable iterators, insertion and deletion in O(1) time.
*
* \param T       Type of elements.
* \param Deleter Type of deleter for the list elements.
*/
template<class T, class Deleter = std::default_delete<T>>
class ilist: private ilist_data<T>, boost::noncopyable {
    using ilist_data<T>::front_;
    using ilist_data<T>::back_;

public:
    typedef T value_type;
    typedef Deleter deleter_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef std::unique_ptr<T, deleter_type> unique_ptr;
    typedef ilist_iterator<T, T> iterator;
    typedef ilist_iterator<const T, T> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef std::size_t size_type;

    /**
     * Constructs an empty list.
     *
     * \param deleter Deleter for the list elements.
     */
    ilist(const deleter_type &deleter = deleter_type()):
        deleter_(deleter)
    {
        front_ = NULL;
        back_ = NULL;
    }

    /**
     * Move constructor.
     *
     * \param that The list to move from.
     */
    template<class U>
    ilist(ilist<U, Deleter> &&that):
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
     * Move-assignment operator.
     *
     * \param that List to move from.
     */
    template<class U>
    ilist &operator=(ilist<U, Deleter> &&that) {
        clear();

        deleter_ = std::move(that.deleter_);
        front_ = that.front_;
        back_ = that.back_;

        that.front_ = NULL;
        that.back_ = NULL;

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
    deleter_type &get_deleter() noexcept { return deleter_; }

    /**
     * \return Deleter used for the destruction of managed elements.
     */
    const deleter_type &get_deleter() const noexcept { return deleter_; }

    /**
     * \return Pointer to the first element. Will be NULL if the list is empty.
     */
    pointer front() noexcept { return front_; }

    /**
     * \return Pointer to the first element. Will be NULL if the list is empty.
     */
    const_pointer front() const noexcept { return front_; }

    /**
     * \return Pointer to the last element. Will be NULL if the list is empty.
     */
    pointer back() noexcept { return back_; }

    /**
     * \return Pointer to the last element. Will be NULL if the list is empty.
     */
    const_pointer back() const noexcept { return back_; }

    /**
     * \return True if the list is empty, false otherwise.
     */
    bool empty() const noexcept { return front() == NULL; }

    /**
     * Removes and destroys all the elements in the list.
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
     * \return Valid pointer to the removed element.
     */
    unique_ptr erase(pointer element) {
        assert(element != NULL);

        if (element == front_) {
            front_ = front_->ilist_item<T>::next_;
        }
        if (element == back_) {
            back_ = back_->ilist_item<T>::prev_;
        }

        if (element->ilist_item<T>::prev_) {
            element->ilist_item<T>::prev_->ilist_item<T>::next_ = element->ilist_item<T>::next_;
        }
        if (element->ilist_item<T>::next_) {
            element->ilist_item<T>::next_->ilist_item<T>::prev_ = element->ilist_item<T>::prev_;
        }

        element->ilist_item<T>::next_ = NULL;
        element->ilist_item<T>::prev_ = NULL;

        return unique_ptr(element, deleter_);
    }

    /**
     * Removes the element at a given position from the list.
     *
     * \param iterator Iterator pointing to the given element.
     *
     * \return Valid pointer to the removed element.
     */
    unique_ptr erase(iterator iterator) {
        return erase(*iterator);
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
     *
     * \return Valid pointer to the inserted element.
     */
    pointer insert(const_iterator position, unique_ptr element) noexcept {
        assert(element != NULL);
        assert(element->ilist_item<T>::next_ == NULL);
        assert(element->ilist_item<T>::prev_ == NULL);

        element->ilist_item<T>::next_ = const_cast<pointer>(*position);
        element->ilist_item<T>::prev_ = const_cast<pointer>(*--position);

        if (element->ilist_item<T>::next_ != NULL) {
            element->ilist_item<T>::next_->ilist_item<T>::prev_ = element.get();
        } else {
            back_ = element.get();
        }

        if (element->ilist_item<T>::prev_ != NULL) {
            element->ilist_item<T>::prev_->ilist_item<T>::next_ = element.get();
        } else {
            front_ = element.get();
        }

        return element.release();
    }

    /**
     * Inserts given element to the front of the list.
     *
     * \param element Valid pointer to the element.
     *
     * \return Valid pointer to the inserted element.
     */
    pointer push_front(unique_ptr element) noexcept {
        return insert(begin(), std::move(element));
    }

    /**
     * Inserts given element to the back of the list.
     *
     * \param element Valid pointer to the element.
     *
     * \return Valid pointer to the inserted element.
     */
    pointer push_back(unique_ptr element) noexcept {
        return insert(end(), std::move(element));
    }

    /**
     * \param first First element of the range to be cut out.
     * \param last Next after the last element of the range to be cut out.
     *
     * \return List containing the range being cut out.
     */
    ilist cut_out(const_iterator first, const_iterator last) {
        ilist result(deleter_);

        if (first == last) {
            return result;
        }

        result.front_ = const_cast<pointer>(*first);
        result.back_ = const_cast<pointer>(*--last);

        assert(result.front_ != NULL);
        assert(result.back_ != NULL);

        if (result.front_ == front_) {
            front_ = result.back_->ilist_item<T>::next_;
            if (front_) {
                front_->ilist_item<T>::prev_ = NULL;
            }
        }

        if (result.back_ == back_) {
            back_ = result.front_->ilist_item<T>::prev_;
            if (back_) {
                back_->ilist_item<T>::next_ = NULL;
            }
        }

        if (result.front_->ilist_item<T>::prev_) {
            result.front_->ilist_item<T>::prev_->ilist_item<T>::next_ = result.back_->ilist_item<T>::next_;
        }
        if (result.back_->ilist_item<T>::next_) {
            result.back_->ilist_item<T>::next_->ilist_item<T>::prev_ = result.front_->ilist_item<T>::prev_;
        }

        result.front_->ilist_item<T>::prev_ = NULL;
        result.back_->ilist_item<T>::next_ = NULL;

        return result;
    }

    /**
     * \return Iterator pointing to the first element.
     */
    iterator begin() noexcept { return iterator(this, front()); }

    /**
     * \return Iterator pointing to the next after the last element.
     */
    iterator end() noexcept { return iterator(this); }

    /**
     * \return Constant iterator to the first element.
     */
    const_iterator begin() const noexcept { return cbegin(); }

    /**
     * \return Constant iterator to the next after the last element.
     */
    const_iterator end() const noexcept { return cend(); }

    /**
     * Reverse iterator pointing to the last element.
     */
    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }

    /**
     * Reverse iterator pointing to the last element.
     */
    const_reverse_iterator rbegin() const noexcept { return crbegin(); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    const_reverse_iterator rend() const noexcept { return crend(); }

    /**
     * \return Constant iterator to the first element.
     */
    const_iterator cbegin() const noexcept { return const_iterator(this, front()); }

    /**
     * \return Constant iterator to the next after the last element.
     */
    const_iterator cend() const noexcept { return const_iterator(this); }

    /**
     * Reverse iterator pointing to the last element.
     */
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }

    /**
     * Reverse iterator pointing to the element before the first one.
     */
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    /**
     * \param element Valid pointer to an element of the list.
     *
     * \return Iterator pointing to the given element.
     */
    iterator get_iterator(pointer element) noexcept {
        assert(element != NULL);
        return iterator(this, element);
    }

    /**
     * \param element Valid pointer to an element of the list.
     *
     * \return Constant iterator pointing to the given element.
     */
    const_iterator get_iterator(const_pointer element) const noexcept {
        assert(element != NULL);
        return const_iterator(this, element);
    }

    /**
     * \return Number of elements in the list.
     *
     * \warning This function takes O(N) time to compute.
     */
    size_type size() const noexcept {
        return std::distance(begin(), end());
    }

private:
    /**
     * Deleter for the list elements.
     */
    deleter_type deleter_;
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
