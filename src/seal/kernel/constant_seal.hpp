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

#pragma once

#include <vector>

#include "he_type.hpp"
#include "ngraph/type/element_type.hpp"
#include "seal/he_seal_backend.hpp"

namespace ngraph::runtime::he {
void constant_seal(std::vector<HEType>& out, const element::Type& element_type,
                   const void* data_ptr, const HESealBackend& he_seal_backend,
                   size_t count);

}  // namespace ngraph::runtime::he
