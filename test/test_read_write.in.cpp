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

#include <climits>

#include "ngraph/ngraph.hpp"
#include "seal/he_seal_backend.hpp"
#include "test_util.hpp"
#include "util/all_close.hpp"
#include "util/ndarray.hpp"
#include "util/test_control.hpp"
#include "util/test_tools.hpp"

static const char* s_manifest = "${MANIFEST}";

namespace ngraph::runtime::he {

NGRAPH_TEST(${BACKEND_NAME}, plain_tv_write_read_scalar) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{};
  {
    auto t_a = he_backend->create_plain_tensor(element::f32, shape);
    copy_data(t_a, std::vector<float>{5});
    EXPECT_TRUE(
        test::all_close(read_vector<float>(t_a), (std::vector<float>{5})));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::f64, shape);
    copy_data(t_a, std::vector<double>{5});
    EXPECT_TRUE(
        test::all_close(read_vector<double>(t_a), (std::vector<double>{5})));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::i32, shape);
    copy_data(t_a, std::vector<int32_t>{5});
    EXPECT_TRUE(
        test::all_close(read_vector<int32_t>(t_a), (std::vector<int32_t>{5})));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::i64, shape);
    copy_data(t_a, std::vector<int64_t>{5});
    EXPECT_TRUE(
        test::all_close(read_vector<int64_t>(t_a), (std::vector<int64_t>{5})));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, plain_tv_write_read_large_scalar_int64) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{};
  auto t_a = he_backend->create_plain_tensor(element::i64, shape);
  copy_data(t_a, std::vector<int64_t>{LLONG_MAX});
  EXPECT_TRUE(test::all_close(read_vector<int64_t>(t_a),
                              (std::vector<int64_t>{LLONG_MAX})));

  copy_data(t_a, std::vector<int64_t>{LLONG_MIN});
  EXPECT_TRUE(test::all_close(read_vector<int64_t>(t_a),
                              (std::vector<int64_t>{LLONG_MIN})));
}

NGRAPH_TEST(${BACKEND_NAME}, plain_tv_write_read_2) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2};
  {
    auto t_a = he_backend->create_plain_tensor(element::f32, shape);
    copy_data(t_a, std::vector<float>{5, 6});
    EXPECT_TRUE(
        test::all_close(read_vector<float>(t_a), (std::vector<float>{5, 6})));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::f64, shape);
    copy_data(t_a, std::vector<double>{5, 6});
    EXPECT_TRUE(
        test::all_close(read_vector<double>(t_a), (std::vector<double>{5, 6})));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::i32, shape);
    copy_data(t_a, std::vector<int32_t>{5, 6});
    EXPECT_TRUE(test::all_close(read_vector<int32_t>(t_a),
                                (std::vector<int32_t>{5, 6})));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::i64, shape);
    copy_data(t_a, std::vector<int64_t>{5, 6});
    EXPECT_TRUE(test::all_close(read_vector<int64_t>(t_a),
                                (std::vector<int64_t>{5, 6})));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, plain_tv_write_read_2_3) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 3};
  {
    auto t_a = he_backend->create_plain_tensor(element::f32, shape);
    copy_data(
        t_a,
        ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<float>(t_a),
        ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}})
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::f64, shape);
    copy_data(t_a, ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<double>(t_a),
        ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}})
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::i32, shape);
    copy_data(t_a, ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int32_t>(t_a),
        ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}})
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_plain_tensor(element::i64, shape);
    copy_data(t_a, ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int64_t>(t_a),
        ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}})
            .get_vector()));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, plain_tv_batch_write_read_2_3) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 3};
  {
    auto t_a = he_backend->create_packed_plain_tensor(element::f32, shape);
    copy_data(
        t_a,
        ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<float>(t_a),
        (ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_plain_tensor(element::f64, shape);
    copy_data(t_a, ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<double>(t_a),
        (ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_plain_tensor(element::i32, shape);
    copy_data(t_a, ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int32_t>(t_a),
        (ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_plain_tensor(element::i64, shape);
    copy_data(t_a, ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int64_t>(t_a),
        (ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, cipher_tv_write_read_scalar) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{};
  {
    auto t_a = he_backend->create_cipher_tensor(element::f32, shape);
    copy_data(t_a, std::vector<float>{5});
    EXPECT_TRUE(
        test::all_close(read_vector<float>(t_a), (std::vector<float>{5})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::f64, shape);
    copy_data(t_a, std::vector<double>{5});
    EXPECT_TRUE(
        test::all_close(read_vector<double>(t_a), (std::vector<double>{5})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i32, shape);
    copy_data(t_a, std::vector<int32_t>{5});
    EXPECT_TRUE(
        test::all_close(read_vector<int32_t>(t_a), (std::vector<int32_t>{5})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i64, shape);
    copy_data(t_a, std::vector<int64_t>{5});
    EXPECT_TRUE(
        test::all_close(read_vector<int64_t>(t_a), (std::vector<int64_t>{5})));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, cipher_tv_write_read_2) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());
  Shape shape{2};
  {
    auto t_a = he_backend->create_cipher_tensor(element::f32, shape);
    copy_data(t_a, std::vector<float>{5, 6});
    EXPECT_TRUE(
        test::all_close(read_vector<float>(t_a), (std::vector<float>{5, 6})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::f64, shape);
    copy_data(t_a, std::vector<double>{5, 6});
    EXPECT_TRUE(
        test::all_close(read_vector<double>(t_a), (std::vector<double>{5, 6})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i32, shape);
    copy_data(t_a, std::vector<int32_t>{5, 6});
    EXPECT_TRUE(test::all_close(read_vector<int32_t>(t_a),
                                (std::vector<int32_t>{5, 6})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i64, shape);
    copy_data(t_a, std::vector<int64_t>{5, 6});
    EXPECT_TRUE(test::all_close(read_vector<int64_t>(t_a),
                                (std::vector<int64_t>{5, 6})));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, cipher_tv_write_read_2_complex) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());
  he_backend->update_encryption_parameters(
      HESealEncryptionParameters::default_complex_packing_parms());

  Shape shape{2};
  {
    auto t_a = he_backend->create_cipher_tensor(element::f32, shape);
    copy_data(t_a, std::vector<float>{5, 6});
    EXPECT_TRUE(
        test::all_close(read_vector<float>(t_a), (std::vector<float>{5, 6})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::f64, shape);
    copy_data(t_a, std::vector<double>{5, 6});
    EXPECT_TRUE(
        test::all_close(read_vector<double>(t_a), (std::vector<double>{5, 6})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i32, shape);
    copy_data(t_a, std::vector<int32_t>{5, 6});
    EXPECT_TRUE(test::all_close(read_vector<int32_t>(t_a),
                                (std::vector<int32_t>{5, 6})));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i64, shape);
    copy_data(t_a, std::vector<int64_t>{5, 6});
    EXPECT_TRUE(test::all_close(read_vector<int64_t>(t_a),
                                (std::vector<int64_t>{5, 6})));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, cipher_tv_write_read_2_3) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 3};
  {
    auto t_a = he_backend->create_cipher_tensor(element::f32, shape);
    copy_data(
        t_a,
        ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<float>(t_a),
        (ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::f64, shape);
    copy_data(t_a, ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<double>(t_a),
        (ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i32, shape);
    copy_data(t_a, ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int32_t>(t_a),
        (ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i64, shape);
    copy_data(t_a, ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int64_t>(t_a),
        (ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, cipher_tv_write_read_5_5) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{5, 5};
  {
    auto t_a = he_backend->create_cipher_tensor(element::f32, shape);
    copy_data(t_a, ngraph::test::NDArray<float, 2>({{1, 2, 3, 4, 5},
                                                    {6, 7, 8, 9, 10},
                                                    {11, 12, 13, 14, 15},
                                                    {16, 17, 18, 19, 20},
                                                    {21, 22, 23, 24, 25}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<float>(t_a),
        (ngraph::test::NDArray<float, 2>({{1, 2, 3, 4, 5},
                                          {6, 7, 8, 9, 10},
                                          {11, 12, 13, 14, 15},
                                          {16, 17, 18, 19, 20},
                                          {21, 22, 23, 24, 25}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::f64, shape);
    copy_data(t_a, ngraph::test::NDArray<double, 2>({{1, 2, 3, 4, 5},
                                                     {6, 7, 8, 9, 10},
                                                     {11, 12, 13, 14, 15},
                                                     {16, 17, 18, 19, 20},
                                                     {21, 22, 23, 24, 25}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<double>(t_a),
        (ngraph::test::NDArray<double, 2>({{1, 2, 3, 4, 5},
                                           {6, 7, 8, 9, 10},
                                           {11, 12, 13, 14, 15},
                                           {16, 17, 18, 19, 20},
                                           {21, 22, 23, 24, 25}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i32, shape);
    copy_data(t_a, ngraph::test::NDArray<int32_t, 2>({{1, 2, 3, 4, 5},
                                                      {6, 7, 8, 9, 10},
                                                      {11, 12, 13, 14, 15},
                                                      {16, 17, 18, 19, 20},
                                                      {21, 22, 23, 24, 25}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int32_t>(t_a),
        (ngraph::test::NDArray<int32_t, 2>({{1, 2, 3, 4, 5},
                                            {6, 7, 8, 9, 10},
                                            {11, 12, 13, 14, 15},
                                            {16, 17, 18, 19, 20},
                                            {21, 22, 23, 24, 25}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_cipher_tensor(element::i64, shape);
    copy_data(t_a, ngraph::test::NDArray<int64_t, 2>({{1, 2, 3, 4, 5},
                                                      {6, 7, 8, 9, 10},
                                                      {11, 12, 13, 14, 15},
                                                      {16, 17, 18, 19, 20},
                                                      {21, 22, 23, 24, 25}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int64_t>(t_a),
        (ngraph::test::NDArray<int64_t, 2>({{1, 2, 3, 4, 5},
                                            {6, 7, 8, 9, 10},
                                            {11, 12, 13, 14, 15},
                                            {16, 17, 18, 19, 20},
                                            {21, 22, 23, 24, 25}}))
            .get_vector()));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, cipher_tv_batch_write_read_2_3) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 3};
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::f32, shape);
    copy_data(
        t_a,
        ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<float>(t_a),
        (ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::f64, shape);
    copy_data(t_a, ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<double>(t_a),
        (ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::i32, shape);
    copy_data(t_a, ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int32_t>(t_a),
        (ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::i64, shape);
    copy_data(t_a, ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int64_t>(t_a),
        (ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, cipher_tv_batch_write_read_2_3_complex) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());
  he_backend->update_encryption_parameters(
      HESealEncryptionParameters::default_complex_packing_parms());

  Shape shape{2, 3};
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::f32, shape);
    copy_data(
        t_a,
        ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<float>(t_a),
        (ngraph::test::NDArray<float, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::f64, shape);
    copy_data(t_a, ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<double>(t_a),
        (ngraph::test::NDArray<double, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::i32, shape);
    copy_data(t_a, ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int32_t>(t_a),
        (ngraph::test::NDArray<int32_t, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::i64, shape);
    copy_data(t_a, ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}})
                       .get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int64_t>(t_a),
        (ngraph::test::NDArray<int64_t, 2>({{1, 2}, {3, 4}, {5, 6}}))
            .get_vector()));
  }
}

NGRAPH_TEST(${BACKEND_NAME}, cipher_tv_batch_write_read_2_1) {
  auto backend = runtime::Backend::create("${BACKEND_NAME}");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 1};
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::f32, shape);
    copy_data(t_a, ngraph::test::NDArray<float, 2>({{1, 2}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<float>(t_a),
        (ngraph::test::NDArray<float, 2>({{1, 2}})).get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::f64, shape);
    copy_data(t_a, ngraph::test::NDArray<double, 2>({{1, 2}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<double>(t_a),
        (ngraph::test::NDArray<double, 2>({{1, 2}})).get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::i32, shape);
    copy_data(t_a, ngraph::test::NDArray<int32_t, 2>({{1, 2}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int32_t>(t_a),
        (ngraph::test::NDArray<int32_t, 2>({{1, 2}})).get_vector()));
  }
  {
    auto t_a = he_backend->create_packed_cipher_tensor(element::i64, shape);
    copy_data(t_a, ngraph::test::NDArray<int64_t, 2>({{1, 2}}).get_vector());
    EXPECT_TRUE(test::all_close(
        read_vector<int64_t>(t_a),
        (ngraph::test::NDArray<int64_t, 2>({{1, 2}})).get_vector()));
  }
}

}  // namespace ngraph::runtime::he
