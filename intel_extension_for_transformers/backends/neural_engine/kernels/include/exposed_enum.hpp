//  Copyright (c) 2022 Intel Corporation
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef ENGINE_SPARSELIB_INCLUDE_EXPOSED_ENUM_HPP_
#define ENGINE_SPARSELIB_INCLUDE_EXPOSED_ENUM_HPP_

namespace jd {
namespace exposed_enum {

namespace groupnorm {
enum io { SRC, DST, GAMMA, BETA, WORKSPACE, SIZE };
}

namespace mha_dense {
enum io {
  SRC_Q,
  SRC_K,
  SRC_V,
  MASK,
  DST,
  WORKSPACE,
  BINARY_ADD,

  ATT_SCALE,  // scale the QxK; typically `1/sqrt(seqlen)`

  Q_SCALE,
  Q_ZP,
  K_SCALE,
  K_ZP,
  V_SCALE,
  V_ZP,
  SRC_DST_SCALE,  // input scale for dst tensor
  SRC_DST_ZP,     // input zp for dst tensor
  DST_SCALE,      // output scale for dst tensor
  DST_ZP,         // output zp for dst tensor

  BATCH_SIZE,
  HEAD_NUM,
  HEAD_SIZE,
  M,  // "seq_len" for Q & DST
  N,  // "seq_len" for K & V

  SIZE,
};
}  // namespace mha_dense

}  // namespace exposed_enum
}  // namespace jd
#endif  // ENGINE_SPARSELIB_INCLUDE_EXPOSED_ENUM_HPP_
