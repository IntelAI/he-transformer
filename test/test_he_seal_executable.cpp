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

#include <sstream>
#include <unordered_set>

#include "gtest/gtest.h"
#include "ngraph/ngraph.hpp"
#include "seal/he_seal_executable.hpp"
#include "seal/seal.h"
#include "test_util.hpp"
#include "util/test_tools.hpp"

namespace ngraph::runtime::he {

class TestHESealExecutable {
 public:
  std::shared_ptr<HESealExecutable> he_seal_executable;

  void generate_calls(const element::Type& type,
                      const NodeWrapper& node_wrapper,
                      const std::vector<std::shared_ptr<HETensor>>& out,
                      const std::vector<std::shared_ptr<HETensor>>& args) {
    he_seal_executable->generate_calls(type, node_wrapper, out, args);
  }
};

TEST(he_seal_executable, generate_calls) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 2};

  auto a = std::make_shared<op::Parameter>(element::f32, shape);
  auto f = std::make_shared<Function>(a, ParameterVector{a});

  auto t_a = test::tensor_from_flags(*he_backend, shape, false, false);

  auto he_handle =
      std::static_pointer_cast<HESealExecutable>(he_backend->compile(f));

  std::vector<std::shared_ptr<HETensor>> args{
      std::static_pointer_cast<HETensor>(t_a)};
  std::vector<std::shared_ptr<HETensor>> out{
      std::static_pointer_cast<HETensor>(t_a)};

  // Unsupported op
  {
    NodeWrapper node_wrapper(std::make_shared<op::Abs>(a));
    auto test_he_seal_executable = TestHESealExecutable{he_handle};
    EXPECT_ANY_THROW(test_he_seal_executable.generate_calls(
        element::f32, node_wrapper, out, args));
  }
  // Skipped op -- parameter
  {
    NodeWrapper node_wrapper(a);
    auto test_he_seal_executable = TestHESealExecutable{he_handle};
    EXPECT_NO_THROW(test_he_seal_executable.generate_calls(
        element::f32, node_wrapper, out, args));
  }
}

TEST(he_seal_executable, plaintext_with_encrypted_annotation) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 2};
  bool packed = true;
  bool arg1_encrypted = true;
  bool arg2_encrypted = false;

  auto a = std::make_shared<op::Parameter>(element::f32, shape);
  auto b = std::make_shared<op::Parameter>(element::f32, shape);
  auto t = std::make_shared<op::Add>(a, b);
  auto f = std::make_shared<Function>(t, ParameterVector{a, b});

  const auto& arg1_config =
      test::config_from_flags(false, arg1_encrypted, packed);
  const auto& arg2_config =
      test::config_from_flags(false, arg2_encrypted, packed);

  std::string error_str;
  he_backend->set_config({{"enable_client", "false"},
                          {a->get_name(), arg1_config},
                          {b->get_name(), arg2_config}},
                         error_str);

  // Create plaintext tensor for ciphertext argument
  // This behavior occurs when using ngraph-bridge
  auto t_a = test::tensor_from_flags(*he_backend, shape, false, packed);
  auto t_b =
      test::tensor_from_flags(*he_backend, shape, arg2_encrypted, packed);
  auto t_result = test::tensor_from_flags(
      *he_backend, shape, arg1_encrypted || arg2_encrypted, packed);

  std::vector<float> input_a{1, 2, 3, 4};
  std::vector<float> input_b{0, -1, 2, -3};
  std::vector<float> exp_result{1, 1, 5, 1};

  copy_data(t_a, input_a);
  copy_data(t_b, input_b);

  auto he_handle =
      std::static_pointer_cast<HESealExecutable>(he_backend->compile(f));

  he_handle->call_with_validate({t_result}, {t_a, t_b});
  EXPECT_TRUE(test::all_close(read_vector<float>(t_result), exp_result, 1e-3f));
}

TEST(he_seal_executable, performance_data) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 2};

  bool packed = true;
  bool arg1_encrypted = false;
  bool arg2_encrypted = false;

  auto a = std::make_shared<op::Parameter>(element::f32, shape);
  auto b = std::make_shared<op::Parameter>(element::f32, shape);
  auto t = std::make_shared<op::Add>(a, b);
  auto f = std::make_shared<Function>(t, ParameterVector{a, b});

  const auto& arg1_config =
      test::config_from_flags(false, arg1_encrypted, packed);
  const auto& arg2_config =
      test::config_from_flags(false, arg2_encrypted, packed);

  std::string error_str;
  he_backend->set_config({{"enable_client", "false"},
                          {a->get_name(), arg1_config},
                          {b->get_name(), arg2_config}},
                         error_str);

  auto t_a =
      test::tensor_from_flags(*he_backend, shape, arg1_encrypted, packed);
  auto t_b =
      test::tensor_from_flags(*he_backend, shape, arg2_encrypted, packed);
  auto t_result = test::tensor_from_flags(
      *he_backend, shape, arg1_encrypted || arg2_encrypted, packed);

  std::vector<float> input_a{1, 2, 3, 4};
  std::vector<float> input_b{0, -1, 2, -3};
  std::vector<float> exp_result{1, 1, 5, 1};
  copy_data(t_a, input_a);
  copy_data(t_b, input_b);

  auto he_handle =
      std::static_pointer_cast<HESealExecutable>(he_backend->compile(f));

  he_handle->call_with_validate({t_result}, {t_a, t_b});
  EXPECT_TRUE(test::all_close(read_vector<float>(t_result), exp_result, 1e-3f));

  std::unordered_set<std::string> node_names;
  for (const auto& node : f->get_ops()) {
    node_names.insert(node->get_name());
  }

  const auto& perf_data = he_handle->get_performance_data();
  for (const auto& perf_counter : perf_data) {
    EXPECT_EQ(perf_counter.call_count(), 1);
    const std::string& node_name = perf_counter.get_node()->get_name();
    ASSERT_TRUE(node_names.find(node_name) != node_names.end());
    node_names.erase(node_name);
    NGRAPH_INFO << perf_counter.get_node()->get_name() << ": call count "
                << perf_counter.call_count() << ", microseconds "
                << perf_counter.microseconds();
  }
}

TEST(he_seal_executable, verbose_op) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 2};

  bool packed = true;
  bool arg1_encrypted = false;
  bool arg2_encrypted = false;

  auto a = std::make_shared<op::Parameter>(element::f32, shape);
  auto b = std::make_shared<op::Parameter>(element::f32, shape);
  auto t = std::make_shared<op::Add>(a, b);
  auto f = std::make_shared<Function>(t, ParameterVector{a, b});

  const auto& arg1_config =
      test::config_from_flags(false, arg1_encrypted, packed);
  const auto& arg2_config =
      test::config_from_flags(false, arg2_encrypted, packed);

  std::string error_str;
  he_backend->set_config({{"enable_client", "false"},
                          {a->get_name(), arg1_config},
                          {b->get_name(), arg2_config}},
                         error_str);

  auto t_a =
      test::tensor_from_flags(*he_backend, shape, arg1_encrypted, packed);
  auto t_b =
      test::tensor_from_flags(*he_backend, shape, arg2_encrypted, packed);
  auto t_result = test::tensor_from_flags(
      *he_backend, shape, arg1_encrypted || arg2_encrypted, packed);

  std::vector<float> input_a{1, 2, 3, 4};
  std::vector<float> input_b{0, -1, 2, -3};
  std::vector<float> exp_result{1, 1, 5, 1};
  copy_data(t_a, input_a);
  copy_data(t_b, input_b);

  auto he_handle =
      std::static_pointer_cast<HESealExecutable>(he_backend->compile(f));

  he_handle->set_verbose_all_ops(false);

  he_handle->call_with_validate({t_result}, {t_a, t_b});
  EXPECT_TRUE(test::all_close(read_vector<float>(t_result), exp_result, 1e-3f));
}

TEST(he_seal_executable, provenance_tag) {
  auto backend = runtime::Backend::create("HE_SEAL");
  auto he_backend = static_cast<HESealBackend*>(backend.get());

  Shape shape{2, 2};

  bool packed = true;
  bool arg1_encrypted = false;
  bool arg2_encrypted = false;

  auto a = std::make_shared<op::Parameter>(element::f32, shape);
  auto b = std::make_shared<op::Parameter>(element::f32, shape);
  auto t = std::make_shared<op::Add>(a, b);
  auto f = std::make_shared<Function>(t, ParameterVector{a, b});

  std::string b_provenance_tag{"b_provenance_tag"};
  b->add_provenance_tag(b_provenance_tag);

  // Create plaintext tensor for ciphertext argument
  // This behavior occurs when using ngraph-bridge
  auto t_a = test::tensor_from_flags(*he_backend, shape, false, packed);
  auto t_b =
      test::tensor_from_flags(*he_backend, shape, arg2_encrypted, packed);
  auto t_result = test::tensor_from_flags(
      *he_backend, shape, arg1_encrypted || arg2_encrypted, packed);

  std::vector<float> input_a{1, 2, 3, 4};
  std::vector<float> input_b{0, -1, 2, -3};
  std::vector<float> exp_result{1, 1, 5, 1};

  copy_data(t_a, input_a);
  copy_data(t_b, input_b);

  auto he_handle =
      std::static_pointer_cast<HESealExecutable>(he_backend->compile(f));

  he_handle->call_with_validate({t_result}, {t_a, t_b});
  EXPECT_TRUE(test::all_close(read_vector<float>(t_result), exp_result, 1e-3f));
}

}  // namespace ngraph::runtime::he
