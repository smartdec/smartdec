/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <algorithm>
#include <map>
#include <vector>

#include <boost/unordered_map.hpp>

#include <nc/common/Foreach.h>
#include <nc/common/Range.h> /* nc::find */

#include "TextRange.h"

namespace nc {
namespace gui {

/**
 * Template class for tracking position of objects.
 *
 * \tparam T type of objects, positions of which are tracked.
 */
template<class T>
class RangeTracker {
    /** Mapping from an object pointer to the range occupied by the object. */
    boost::unordered_map<T *, TextRange> object2range_;

    /**
     * Mappings from a range to the pointer to the object occupying this range.
     * Keys of each mapping are non-mutually-overlapping ranges.
     */
    std::vector<std::map<TextRange, T *>> range2object_;

    /** Vector containing pointers to objects and their center positions. */
    mutable std::vector<std::pair<int, T *>> center2object_;

    /** Flag whether the tracker has been modified since last call to autoSort(). */
    mutable bool modified_;

    public:

    /**
     * Constructor.
     */
    RangeTracker(): modified_(false) {}

    /**
     * Deletes all previously added ranges from the tracker.
     */
    void clear() {
        object2range_.clear();
        range2object_.clear();
        center2object_.clear();
        modified_ = false;
    }

    /**
     * Adds an object at given range.
     *
     * \param[in] object    Pointer to an object.
     * \param[in] range     Range.
     */
    void addRange(T *object, TextRange range) {
        object2range_[object] = range;
        center2object_.push_back(std::make_pair(range.center(), object));

        bool inserted = false;
        foreach (auto &map, range2object_) {
            if (!contains(map, range)) {
                map[range] = object;
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            range2object_.resize(range2object_.size() + 1);
            range2object_.back().insert(std::make_pair(range, object));
        }

        modified_ = true;
    }

    /**
     * \param[in] object Pointer to an object.
     *
     * \return Range occupied by the object.
     * If no range is found, invalid range is returned.
     */
    TextRange getRange(T *object) const {
        return find(object2range_, object);
    }

    /**
     * \param position Position.
     *
     * \return A pointer to the object whose range covers given position.
     * If there are several such objects, the one with the shortest range is returned.
     */
    T *getObject(int position) const {
        T *result = NULL;
        int bestLength = -1;

        TextRange key(position, position + 1);

        foreach (auto &map, range2object_) {
            auto i = map.find(key);
            if (i != map.end()) {
                if (bestLength == -1 || i->first.length() < bestLength) {
                    bestLength = i->first.length();
                    result = i->second;
                }
            }
        }

        return result;
    }

    /**
     * Finds objects covered by given range.
     * Current implementation returns all objects whose centers fall into given range.
     *
     * \param[in] range Range.
     *
     * \return Pointers to objects covered by given range.
     */
    std::vector<T *> getObjects(const TextRange &range) const {
        autoSort();

        auto i = std::lower_bound(center2object_.begin(), center2object_.end(), std::pair<int, T *>(range.start(), NULL));
        auto iend = std::upper_bound(center2object_.begin(), center2object_.end(), std::pair<int, T *>(range.end(), NULL));

        std::vector<T *> result;
        result.reserve(iend - i);

        while (i != iend) {
            result.push_back(i++->second);
        }

        if (result.empty()) {
            if (T *object = getObject(range.center())) {
                result.push_back(object);
            }
        }

        return result;
    }

    private:

    /**
     * Does some precomputations if necessary.
     */
    void autoSort() const {
        if (modified_) {
            std::sort(center2object_.begin(), center2object_.end());
            modified_ = false;
        }
    }
};

} // namespace gui
} // namespace nc

/* vim:set et sts=4 sw=4: */
