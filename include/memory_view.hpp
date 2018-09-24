// MIT License
//
// Copyright (c) 2018 Michal Siedlaczek
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/// \file
/// \author Michal Siedlaczek
/// \copyright MIT License

#pragma once

#include <memory>

#include <gsl/span>

namespace mv {

    /// Represents the left end of a range when slicing.
    struct begin_t {} begin;

    /// Represents the right end of a range when slicing.
    struct end_t {} end;

    /// A view to an arbitrary memory buffer, such as array, or memory mapped file.
    ///
    /// In a lazy memory source, any data access will cause to fetch
    /// the underlying data.
    /// No data is accessed when slicing.
    class memory_view {
    public:
        /// Creates a memory view from a memory source.
        ///
        /// \tparam source_type     This type must provide the following methods:
        /// ```
        /// virtual const char* fetch(std::ptrdiff_t begin, std::ptrdiff_t end) const;
        /// virtual std::size_t size() const;
        /// ```
        template<typename source_type>
        memory_view(source_type source)
            : self_(std::make_shared<model<source_type>>(source)),
              begin_(0),
              end_(self_->size())
        {}

        memory_view() = default;
        ~memory_view() = default;
        memory_view(const memory_view&) = default;
        memory_view(memory_view&&) noexcept = default;
        memory_view& operator=(const memory_view&) = default;
        memory_view& operator=(memory_view&&) noexcept = default;

        /// Returns the pointer to to the first byte.
        const char* as_ptr() const
        {
            return self_->fetch(begin_, end_);
        }

        /// Casts data to the given type, starting at the beginning of the range.
        template<typename T>
        T as() const
        {
            return *reinterpret_cast<const T*>(as_ptr());
        }

        /// Returns a `gsl::span` of the given type.
        template<typename T>
        gsl::span<const T> as_span() const
        {
            return gsl::span<const T>(
                reinterpret_cast<const T*>(as_ptr()),
                size() / sizeof(T));
        }

        /// Unpacks the memory into several variables, returned as a tuple.
        ///
        /// Example:
        /// ```
        /// auto [n, ch, arr] = mv.unpack<int, char, std::array<char, 5>>();
        /// ```
        template<typename ...T>
        std::tuple<T...> unpack() const
        {
            std::ptrdiff_t offset = sizeof(std::tuple<T...>);
            const char* ptr = as_ptr();
            return std::make_tuple(*cast_and_move<T>(ptr, offset)...);
        }

        /// Unpacks leading values and returns them along with the remaining slice.
        template<typename ...T>
        std::tuple<T..., memory_view> unpack_head() const
        {
            std::ptrdiff_t offset = sizeof(std::tuple<T...>);
            auto tail = this->operator()(offset, mv::end);
            const char* ptr = as_ptr();
            auto head = std::make_tuple(*cast_and_move<T>(ptr, offset)...);
            return std::tuple_cat(head, std::tuple<memory_view>(tail));
        }

        std::size_t size() const { return end_ - begin_; }
        bool empty() const { return size() == 0; }

        /// Returns a slice `[first, last)`.
        memory_view operator()(std::ptrdiff_t first, std::ptrdiff_t last) const
        {
            memory_view copy = *this;
            copy.begin_ += first;
            copy.end_ = begin_ + last;
            return copy;
        }

        /// Returns a slice from `first` to the end of the view.
        memory_view operator()(std::ptrdiff_t first, end_t) const
        {
            memory_view copy = *this;
            copy.begin_ += first;
            return copy;
        }

        /// Returns a slice from the beginning of the view to last (exclusive).
        memory_view operator()(begin_t, std::ptrdiff_t last) const
        {
            memory_view copy = *this;
            copy.end_ = begin_ + last;
            return copy;
        }

    private:
        struct source_concept {
            source_concept() = default;
            source_concept(const source_concept&) = default;
            source_concept(source_concept&&) = default;
            source_concept& operator=(const source_concept&) = default;
            source_concept& operator=(source_concept&&) = default;
            virtual ~source_concept() = default;
            virtual const char* fetch(
                std::ptrdiff_t begin, std::ptrdiff_t end) const = 0;
            virtual std::size_t size() const = 0;
        };

        template<typename source_type>
        class model : public source_concept {
        public:
            explicit model(source_type source) : source_(std::move(source)) {}

            const char*
            fetch(std::ptrdiff_t begin, std::ptrdiff_t end) const override
            {
                return source_.fetch(begin, end);
            }

            std::size_t size() const override { return source_.size(); }
        private:
            source_type source_;
        };

        template<typename T>
        const T* cast_and_move(const char* ptr, std::ptrdiff_t& offset) const
        {
            offset -= sizeof(T);
            return reinterpret_cast<const T*>(ptr + offset);
        }

        std::shared_ptr<source_concept> self_ = nullptr;
        std::ptrdiff_t begin_ = 0;
        std::ptrdiff_t end_ = 0;
    };

    class ptr_memory_source {
    public:
        ptr_memory_source(const char* ptr, std::size_t size)
            : ptr_(ptr), len_(size) {}

        template<int size>
        ptr_memory_source(const std::array<char, size>& arr)
            : ptr_(&arr[0]), len_(arr.size()) {}

        template<typename ContiguousContainer>
        ptr_memory_source(const ContiguousContainer& container)
            : ptr_(&container[0]), len_(container.size()) {}

        ptr_memory_source() = default;
        ~ptr_memory_source() = default;
        ptr_memory_source(const ptr_memory_source&) = default;
        ptr_memory_source(ptr_memory_source&) = default;
        ptr_memory_source& operator=(const ptr_memory_source&) = default;
        ptr_memory_source& operator=(ptr_memory_source&&) = default;
        const char* fetch(std::ptrdiff_t begin, std::ptrdiff_t end) const
        {
            return std::next(ptr_, begin);
        }
        std::size_t size() const { return len_; }

    private:
        const char* ptr_ = nullptr;
        std::size_t len_ = 0;
    };

    template<typename Container>
    memory_view make_memory_view(const Container& container)
    {
        return memory_view(ptr_memory_source(
            reinterpret_cast<const char*>(container.data()),
            container.size() * sizeof(container[0])));
    }

    template<typename T, typename Container>
    memory_view make_memory_view(const Container& container)
    {
        return memory_view(ptr_memory_source(
            reinterpret_cast<const char*>(container.data()),
            container.size() * sizeof(T)));
    }

}  // memory_view