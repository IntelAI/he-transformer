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

#include <memory>

#include "gtest/gtest.h"
#include "he_tensor.hpp"
#include "he_type.hpp"
#include "ngraph/ngraph.hpp"
#include "seal/he_seal_backend.hpp"
#include "seal/he_seal_executable.hpp"
#include "test_util.hpp"
#include "util/test_tools.hpp"

namespace ngraph::runtime::he {

TEST(he_tensor, empty_plain_tensor) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  auto t_a = std::static_pointer_cast<HETensor>(he_backend->create_plain_tensor(
      ngraph ::element::f32, Shape{1, 2, 3}, false));

  EXPECT_EQ(t_a->data().size(), 6);
  for (const auto& elem : t_a->data()) {
    EXPECT_TRUE(elem.is_plaintext());
  }
}

TEST(he_tensor, empty_cipher_tensor) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  auto t_a =
      std::static_pointer_cast<HETensor>(he_backend->create_cipher_tensor(
          ngraph ::element::f32, Shape{1, 2, 3}, false));

  EXPECT_EQ(t_a->data().size(), 6);
  for (const auto& elem : t_a->data()) {
    EXPECT_TRUE(elem.is_ciphertext());
  }
}

TEST(he_tensor, pack_non_zero_axis) {
  EXPECT_ANY_THROW(HETensor::pack_shape(Shape{2, 1}, 1));
  EXPECT_ANY_THROW(HETensor::unpack_shape(Shape{1, 1}, 2, 1));
}

TEST(he_tensor, pack) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 2};
  HETensor plain(element::f32, shape, false, false, false, *he_backend);

  std::vector<HEType> elements;
  for (size_t i = 0; i < shape_size(shape); ++i) {
    elements.emplace_back(HEPlaintext({static_cast<double>(i)}), false);
  }
  plain.data() = elements;
  plain.pack();

  EXPECT_TRUE(plain.is_packed());
  EXPECT_EQ(plain.get_packed_shape(), (Shape{1, 2}));
  EXPECT_EQ(plain.get_batch_size(), 2);
  EXPECT_EQ(plain.data().size(), 2);
  for (size_t i = 0; i < 2; ++i) {
    EXPECT_TRUE(plain.data(i).is_plaintext());
    EXPECT_EQ(plain.data(i).get_plaintext().size(), 2);
  }
  EXPECT_EQ(plain.data(0).get_plaintext()[0], 0);
  EXPECT_EQ(plain.data(0).get_plaintext()[1], 2);
  EXPECT_EQ(plain.data(1).get_plaintext()[0], 1);
  EXPECT_EQ(plain.data(1).get_plaintext()[1], 3);
}

TEST(he_tensor, unpack) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 2};
  HETensor plain(element::f32, shape, true, false, false, *he_backend);
  std::vector<HEType> elements;

  elements.emplace_back(HEPlaintext(std::initializer_list<double>{0, 1}),
                        false);
  elements.emplace_back(HEPlaintext(std::initializer_list<double>{2, 3}),
                        false);
  plain.data() = elements;
  plain.unpack();

  EXPECT_FALSE(plain.is_packed());
  EXPECT_EQ(plain.get_packed_shape(), (Shape{2, 2}));
  EXPECT_EQ(plain.data().size(), 4);
  EXPECT_EQ(plain.get_batch_size(), 1);

  for (size_t i = 0; i < 4; ++i) {
    EXPECT_TRUE(plain.data(i).is_plaintext());
    EXPECT_EQ(plain.data(i).get_plaintext().size(), 1);
  }
  EXPECT_EQ(plain.data(0).get_plaintext()[0], 0);
  EXPECT_EQ(plain.data(1).get_plaintext()[0], 2);
  EXPECT_EQ(plain.data(2).get_plaintext()[0], 1);
  EXPECT_EQ(plain.data(3).get_plaintext()[0], 3);
}

TEST(he_tensor, save) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());
  auto parms = HESealEncryptionParameters::default_real_packing_parms();
  he_backend->update_encryption_parameters(parms);

  Shape shape{2};

  auto tensor = he_backend->create_plain_tensor(element::f32, shape);
  std::vector<float> tensor_data({5, 6});

  copy_data(tensor, tensor_data);
  auto he_tensor = std::static_pointer_cast<HETensor>(tensor);

  auto pb_tensors = he_tensor->write_to_pb_tensors();

  // Validate saved pb_tensors
  EXPECT_EQ(pb_tensors.size(), 1);
  const auto& pb_tensor = pb_tensors[0];
  EXPECT_EQ(pb_tensor.name(), he_tensor->get_name());

  std::vector<uint64_t> expected_shape{shape};
  for (size_t shape_idx = 0; shape_idx < expected_shape.size(); ++shape_idx) {
    EXPECT_EQ(pb_tensor.shape(shape_idx), expected_shape[shape_idx]);
  }

  EXPECT_EQ(pb_tensor.offset(), 0);
  EXPECT_EQ(pb_tensor.packed(), he_tensor->is_packed());
  EXPECT_EQ(pb_tensor.data_size(), he_tensor->data().size());
  for (size_t i = 0; i < he_tensor->data().size(); ++i) {
    EXPECT_TRUE(pb_tensor.data(i).is_plaintext());

    std::vector<float> plain = {pb_tensor.data(i).plain().begin(),
                                pb_tensor.data(i).plain().end()};
    EXPECT_EQ(plain.size(), 1);
    EXPECT_FLOAT_EQ(plain[0], tensor_data[i]);
  }
}

TEST(he_tensor, load) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());
  auto parms = HESealEncryptionParameters::default_real_packing_parms();
  he_backend->update_encryption_parameters(parms);

  Shape shape{2};
  auto tensor =
      he_backend->create_plain_tensor(element::f32, shape, "tensor_name");
  std::vector<float> tensor_data({5, 6});

  copy_data(tensor, tensor_data);
  auto saved_he_tensor = std::static_pointer_cast<HETensor>(tensor);

  auto pb_tensors = saved_he_tensor->write_to_pb_tensors();

  auto loaded_he_tensor = HETensor::load_from_pb_tensors(
      pb_tensors, *he_backend->get_ckks_encoder(), he_backend->get_context(),
      *he_backend->get_encryptor(), *he_backend->get_decryptor(),
      he_backend->get_encryption_parameters());

  EXPECT_EQ(loaded_he_tensor->get_name(), saved_he_tensor->get_name());
  EXPECT_EQ(loaded_he_tensor->is_packed(), saved_he_tensor->is_packed());
  EXPECT_EQ(loaded_he_tensor->data().size(), saved_he_tensor->data().size());
  EXPECT_EQ(loaded_he_tensor->get_shape().size(),
            saved_he_tensor->get_shape().size());
  for (size_t shape_idx = 0; shape_idx < saved_he_tensor->get_shape().size();
       ++shape_idx) {
    EXPECT_EQ(loaded_he_tensor->get_shape()[shape_idx],
              saved_he_tensor->get_shape()[shape_idx]);
  }

  NGRAPH_INFO << "saved_he_tensor " << saved_he_tensor->get_element_type();
  NGRAPH_INFO << "loaded_he_tensor " << loaded_he_tensor->get_element_type();

  EXPECT_TRUE(test::all_close(read_vector<float>(loaded_he_tensor),
                              read_vector<float>(saved_he_tensor)));
}

TEST(he_tensor, load_from_context) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());
  auto parms = HESealEncryptionParameters::default_real_packing_parms();
  he_backend->update_encryption_parameters(parms);

  Shape shape{2};
  auto tensor = he_backend->create_plain_tensor(element::f32, shape, false,
                                                "tensor_name");
  std::vector<float> tensor_data({5, 6});

  copy_data(tensor, tensor_data);
  auto saved_he_tensor = std::static_pointer_cast<HETensor>(tensor);

  auto loaded_he_tensor =
      std::static_pointer_cast<HETensor>(he_backend->create_plain_tensor(
          saved_he_tensor->get_element_type(), saved_he_tensor->get_shape(),
          saved_he_tensor->is_packed(), saved_he_tensor->get_name()));

  auto pb_tensors = saved_he_tensor->write_to_pb_tensors();

  HETensor::load_from_pb_tensor(loaded_he_tensor, pb_tensors[0],
                                he_backend->get_context());

  EXPECT_EQ(loaded_he_tensor->get_name(), saved_he_tensor->get_name());
  EXPECT_EQ(loaded_he_tensor->is_packed(), saved_he_tensor->is_packed());
  EXPECT_EQ(loaded_he_tensor->data().size(), saved_he_tensor->data().size());
  EXPECT_EQ(loaded_he_tensor->get_shape().size(),
            saved_he_tensor->get_shape().size());
  for (size_t shape_idx = 0; shape_idx < saved_he_tensor->get_shape().size();
       ++shape_idx) {
    EXPECT_EQ(loaded_he_tensor->get_shape()[shape_idx],
              saved_he_tensor->get_shape()[shape_idx]);
  }

  NGRAPH_INFO << "saved_he_tensor " << saved_he_tensor->get_element_type();
  NGRAPH_INFO << "loaded_he_tensor " << loaded_he_tensor->get_element_type();

  EXPECT_TRUE(test::all_close(read_vector<float>(loaded_he_tensor),
                              read_vector<float>(saved_he_tensor)));
}

TEST(he_tensor, io_bounds) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  auto element_type = element::f32;

  auto t_a = std::static_pointer_cast<HETensor>(
      he_backend->create_plain_tensor(element_type, Shape{1, 2, 3}, false));

  void* dummy = nullptr;
  // Bytes not a factor of element type
  EXPECT_ANY_THROW(t_a->read(dummy, element_type.size() + 1));
  EXPECT_ANY_THROW(t_a->write(dummy, element_type.size() + 1));

  // Too many bytes
  EXPECT_ANY_THROW(t_a->read(dummy, element_type.size() * 100));
  EXPECT_ANY_THROW(t_a->write(dummy, element_type.size() * 100));
}

TEST(he_tensor, zero) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  auto t_zero = std::static_pointer_cast<HETensor>(
      he_backend->create_plain_tensor(element::f32, Shape{0, 4}, true));

  // I/O access out of bounds
  void* dummy = nullptr;
  EXPECT_ANY_THROW(t_zero->write(dummy, 1));
  EXPECT_ANY_THROW(t_zero->read(dummy, 1));

  EXPECT_EQ(t_zero->get_batched_element_count(), 0);
}
}  // namespace ngraph::runtime::he
