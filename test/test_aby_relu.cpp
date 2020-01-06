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

#include <random>

#include "ENCRYPTO_utils/crypto/crypto.h"
#include "ENCRYPTO_utils/parse_options.h"
#include "aby/aby_util.hpp"
#include "aby/kernel/relu_aby.hpp"
#include "abycore/aby/abyparty.h"
#include "abycore/circuit/booleancircuits.h"
#include "abycore/circuit/share.h"
#include "abycore/sharing/sharing.h"
#include "gtest/gtest.h"

namespace ngraph::runtime::aby {

auto test_relu_circuit = [](size_t num_vals, size_t coeff_modulus) {
  e_sharing sharing = S_BOOL;
  uint32_t bitlen = 64;

  std::vector<uint64_t> zeros(num_vals, 0);

  std::vector<uint64_t> x(num_vals);
  std::vector<uint64_t> xs(num_vals);
  std::vector<uint64_t> xc(num_vals);
  std::vector<uint64_t> r(num_vals);
  std::vector<bool> bigger_than_zero(num_vals);
  std::vector<uint64_t> exp_output(num_vals);

  std::random_device rd;
  std::mt19937 gen(0);  // rd());
  std::uniform_int_distribution<uint64_t> dis(0, coeff_modulus - 1);
  for (int i = 0; i < static_cast<int>(num_vals); ++i) {
    x[i] = i;
    r[i] = dis(gen);
    xc[i] = dis(gen);
    xs[i] = (x[i] % coeff_modulus + coeff_modulus) - xc[i];
    xs[i] = xs[i] % coeff_modulus;
    // Relu circuit expects transformation (-q/2, q/2) => (0,q) by adding q to
    // values < 0
    bigger_than_zero[i] = (x[i] % coeff_modulus) <= (coeff_modulus / 2);
    exp_output[i] = bigger_than_zero[i] ? (x[i] + r[i]) % coeff_modulus : r[i];

    EXPECT_EQ((xs[i] + xc[i]) % coeff_modulus, x[i] % coeff_modulus);
  }

  // Server function
  auto server_fun = [&]() {
    auto server = std::make_unique<ABYParty>(
        SERVER, "0.0.0.0", 30001, get_sec_lvl(128), 64, 1, MT_OT, 100000);

    std::vector<Sharing*>& sharings = server->GetSharings();
    BooleanCircuit& circ = dynamic_cast<BooleanCircuit&>(
        *sharings[sharing]->GetCircuitBuildRoutine());

    std::this_thread::sleep_for(std::chrono::seconds(1));

    relu_aby(circ, num_vals, xs, zeros, r, bitlen, coeff_modulus);
    server->ExecCircuit();
    server->Reset();
  };

  // Client function
  auto client_fun = [&]() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto client = std::make_unique<ABYParty>(
        CLIENT, "localhost", 30001, get_sec_lvl(128), 64, 1, MT_OT, 100000);

    std::vector<Sharing*>& sharings = client->GetSharings();
    BooleanCircuit& circ = dynamic_cast<BooleanCircuit&>(
        *sharings[sharing]->GetCircuitBuildRoutine());

    share* relu_out =
        relu_aby(circ, num_vals, zeros, xc, zeros, bitlen, coeff_modulus);

    client->ExecCircuit();

    uint32_t out_bitlen_relu, out_num_aby_vals;
    uint64_t* out_vals_relu;

    relu_out->get_clear_value_vec(&out_vals_relu, &out_bitlen_relu,
                                  &out_num_aby_vals);

    for (size_t i = 0; i < out_num_aby_vals; ++i) {
      if (out_vals_relu[i] != exp_output[i]) {
        NGRAPH_INFO << "Not same at index " << i;
        NGRAPH_INFO << "\tx[i] " << x[i];
        NGRAPH_INFO << "\txs[i] " << xs[i];
        NGRAPH_INFO << "\txc[i] " << xc[i];
        NGRAPH_INFO << "\tr[i] " << r[i];
        NGRAPH_INFO << "\tbigger_than_zero[i] " << bigger_than_zero[i];
        NGRAPH_INFO << "\texp_output[i] " << exp_output[i];
        NGRAPH_INFO << "\toutput " << out_vals_relu[i];
      }
      EXPECT_EQ(out_vals_relu[i], exp_output[i]);
    }

    client->Reset();
  };
  std::thread server_thread(server_fun);
  client_fun();
  server_thread.join();
};

TEST(aby, relu_circuit_10_q8) { test_relu_circuit(10, 8); }

TEST(aby, relu_circuit_100_q8) { test_relu_circuit(100, 8); }

TEST(aby, relu_circuit_10_q9) { test_relu_circuit(10, 9); }

TEST(aby, relu_circuit_100_q9) { test_relu_circuit(100, 9); }

TEST(aby, relu_circuit_100_q_large) {
  test_relu_circuit(100, 18014398509404161);
}

}  // namespace ngraph::runtime::aby
