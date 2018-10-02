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

#include <fstream>

#include <boost/iostreams/device/mapped_file.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory_view.hpp>

namespace {

using mv::memory_view;
using mv::ptr_memory_source;

class ptr_memory_source_suite : public ::testing::Test {
protected:
    std::array<char, 4> arr = {1, 2, 3, 4};
    memory_view mv = memory_view(ptr_memory_source(arr.data(), 4));
};

TEST_F(ptr_memory_source_suite, size)
{
    ASSERT_EQ(mv.size(), arr.size());
}

TEST_F(ptr_memory_source_suite, inner_view)
{
    memory_view inner = memory_view(ptr_memory_source(&arr[1], 2));
    ASSERT_EQ(inner.size(), 2);
    auto span = inner.as_span<char>();
    ASSERT_EQ(&span[0], &arr[1]);
}

TEST_F(ptr_memory_source_suite, range_slice)
{
    auto slice = mv(1, 3);
    ASSERT_EQ(slice.size(), 2);
    auto span = slice.as_span<char>();
    ASSERT_EQ(&span[0], &arr[1]);
}

TEST_F(ptr_memory_source_suite, from_slice)
{
    auto slice = mv(1, mv::end);
    ASSERT_EQ(slice.size(), 3);
    auto span = slice.as_span<char>();
    ASSERT_EQ(&span[0], &arr[1]);
}

TEST_F(ptr_memory_source_suite, to_slice)
{
    auto slice = mv(mv::begin, 3);
    ASSERT_EQ(slice.size(), 3);
    auto span = slice.as_span<char>();
    ASSERT_EQ(&span[0], &arr[0]);
}

TEST_F(ptr_memory_source_suite, as_int)
{
    ASSERT_EQ(mv.as<int>(), 67305985);
}

TEST_F(ptr_memory_source_suite, as_int_span)
{
    auto span = mv.as_span<int>();
    ASSERT_EQ(span[0], 67305985);
    ASSERT_EQ(span.size(), 1);
}

TEST_F(ptr_memory_source_suite, as_int8_span)
{
    auto span = mv.as_span<std::int8_t>();
    ASSERT_EQ(span[0], 1);
    ASSERT_EQ(span[1], 2);
    ASSERT_EQ(span[2], 3);
    ASSERT_EQ(span[3], 4);
    ASSERT_EQ(span.size(), 4);
}

TEST_F(ptr_memory_source_suite, unpack_int8)
{
    auto [a, b, c, d] =
        mv.unpack<std::int8_t, std::int8_t, std::int8_t, std::int8_t>();
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);
    ASSERT_EQ(c, 3);
    ASSERT_EQ(d, 4);
}

TEST_F(ptr_memory_source_suite, unpack_different)
{
    auto [a, b, c] =
        mv.unpack<std::int8_t, char, std::int16_t>();
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, static_cast<char>(2));
    ASSERT_EQ(c, 1027);
}

TEST_F(ptr_memory_source_suite, unpack_array)
{
    auto [n, arr] = mv.unpack<std::int8_t, std::array<char, 3>>();
    ASSERT_EQ(n, 1);
    std::vector<char> expected = {2, 3, 4};
    ASSERT_THAT(arr, ::testing::ElementsAreArray(expected));
}

TEST_F(ptr_memory_source_suite, unpack_head)
{
    auto [n, tail] = mv.unpack_head<std::int8_t>();
    ASSERT_EQ(n, 1);
    std::vector<char> expected = {2, 3, 4};
    ASSERT_THAT(tail.as_span<char>(), ::testing::ElementsAreArray(expected));
}

TEST(memory_view, create_from_char_vector)
{
    std::vector<char> vec = {0, 1, 2, 3};
    mv::memory_view mv = mv::make_memory_view(vec);
    ASSERT_THAT(mv.as_span<char>(), ::testing::ElementsAreArray(vec));
}

TEST(memory_view, create_from_int_vector)
{
    std::vector<int> vec = {0, 1, 2, 3};
    mv::memory_view mv = mv::make_memory_view(vec);
    auto span = mv.as_span<char>();
    ASSERT_THAT(mv.as_span<int>(), ::testing::ElementsAreArray(vec));
}

TEST(memory_view, create_from_mapped_file)
{
    std::vector<int> vec = {0, 1, 2, 3};
    {
        std::ofstream out("tmpfile");
        out.write(reinterpret_cast<char*>(&vec[0]), vec.size() * sizeof(int));
    }
    boost::iostreams::mapped_file_source src("tmpfile");
    mv::memory_view mv = mv::make_memory_view<const char>(src);
    ASSERT_THAT(mv.as_span<int>(), ::testing::ElementsAreArray(vec));
}

};  // namespace

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
