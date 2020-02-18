//*****************************************************************************
// Copyright 2018-2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************

#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "ngraph/type/element_type.hpp"
#include "seal/he_seal_backend.hpp"
#include "test_util.hpp"

namespace ngraph::runtime::he {

TEST(he_plaintext, write) {
  std::vector<double> data{1, 2, 3};
  HEPlaintext plain{1, 2, 3};
  {
    auto type = element::f32;
    auto src = static_cast<char*>(ngraph_malloc(plain.size() * type.size()));
    plain.write(src, type);
    std::vector<float> fl_write(reinterpret_cast<float*>(src),
                                reinterpret_cast<float*>(src) + plain.size());
    EXPECT_TRUE(test::all_close(std::vector<float>{1, 2, 3}, fl_write));
    ngraph_free(src);
  }

  {
    auto type = element::f64;
    auto src = static_cast<char*>(ngraph_malloc(plain.size() * type.size()));
    plain.write(src, type);
    std::vector<double> fl_write(reinterpret_cast<double*>(src),
                                 reinterpret_cast<double*>(src) + plain.size());
    EXPECT_TRUE(test::all_close(std::vector<double>{1, 2, 3}, fl_write));
    ngraph_free(src);
  }

  {
    auto type = element::i32;
    auto src = static_cast<char*>(ngraph_malloc(plain.size() * type.size()));
    plain.write(src, type);
    std::vector<int32_t> fl_write(
        reinterpret_cast<int32_t*>(src),
        reinterpret_cast<int32_t*>(src) + plain.size());
    EXPECT_TRUE(test::all_close(std::vector<int32_t>{1, 2, 3}, fl_write));
    ngraph_free(src);
  }

  {
    auto type = element::i64;
    auto src = static_cast<char*>(ngraph_malloc(plain.size() * type.size()));
    plain.write(src, type);
    std::vector<int64_t> fl_write(
        reinterpret_cast<int64_t*>(src),
        reinterpret_cast<int64_t*>(src) + plain.size());
    EXPECT_TRUE(test::all_close(std::vector<int64_t>{1, 2, 3}, fl_write));
    ngraph_free(src);
  }

  // Unsupported types
  auto src = static_cast<char*>(ngraph_malloc(plain.size()));
  EXPECT_ANY_THROW(plain.write(src, element::bf16));
  EXPECT_ANY_THROW(plain.write(src, element::f16));
  EXPECT_ANY_THROW(plain.write(src, element::i8));
  EXPECT_ANY_THROW(plain.write(src, element::i16));
  EXPECT_ANY_THROW(plain.write(src, element::u8));
  EXPECT_ANY_THROW(plain.write(src, element::u16));
  EXPECT_ANY_THROW(plain.write(src, element::u32));
  EXPECT_ANY_THROW(plain.write(src, element::u64));
  EXPECT_ANY_THROW(plain.write(src, element::dynamic));
  EXPECT_ANY_THROW(plain.write(src, element::boolean));
  ngraph_free(src);
}

TEST(he_plaintext, as_double_vec) {
  std::vector<double> data{1, 2, 3};
  HEPlaintext plain{1, 2, 3};
  EXPECT_TRUE(test::all_close(data, plain.as_double_vec()));
}

TEST(he_plaintext, ostream) {
  std::stringstream ss;
  HEPlaintext plain{1, 2, 3};
  EXPECT_NO_THROW(ss << plain);
}

}  // namespace ngraph::runtime::he
