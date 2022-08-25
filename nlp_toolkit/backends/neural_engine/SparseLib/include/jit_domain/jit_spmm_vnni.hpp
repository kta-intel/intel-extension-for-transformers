//  Copyright (c) 2021 Intel Corporation
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

#ifndef ENGINE_SPARSELIB_INCLUDE_JIT_DOMAIN_JIT_SPMM_VNNI_HPP_
#define ENGINE_SPARSELIB_INCLUDE_JIT_DOMAIN_JIT_SPMM_VNNI_HPP_

#include <omp.h>
#include <glog/logging.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "jit_generator.hpp"
#include "../kernels/sparse_data.hpp"
#include "../kernels/spmm_types.hpp"
#include "utils.hpp"

namespace jd {
/**
 * @brief jit_spmm_vnni_t calculates this kind matmul: sparse x dense = dst.
 *        weight(M, K) * activation(K, N) + bias(M, 1) = dst(M, N)
 */
class jit_spmm_vnni_t : public jit_generator {
 public:
  explicit jit_spmm_vnni_t(const ssd::vnni_param_t& param) : jit_generator(), param_(param) {
    const dim_t imb_lo = param_.im_start / TH();
    const dim_t imb_hi = (param_.im_start + param.BM) / TH();
    const dim_t indptr_lo = param_.indptr[imb_lo] * spns::ADJ;
    const dim_t indptr_hi = param_.indptr[imb_hi] * spns::ADJ;
    const dim_t blk_size = param_.blocksize[0] * param_.blocksize[1];
    dense_load_offsets.resize((indptr_hi - indptr_lo) * blk_size);

    std::transform(param_.indices.begin() + indptr_lo, param_.indices.begin() + indptr_hi, dense_load_offsets.begin(),
                   [&](decltype(param_.indices)::value_type k) { return k * ld_dst(); });
  }
  virtual ~jit_spmm_vnni_t() {}

 private:
  ssd::vnni_param_t param_;
  std::vector<dim_t> dense_load_offsets;  // param_.indices * ld_dst

 private:
  void generate() override;

 private:
  // internal API of op kernel
  Xbyak::Zmm TH_Vmm(int i = 0);           // Register allocator of load weight. 1D shape=(TH)
  Xbyak::Zmm TW_Vmm(int i = 0);           // Register allocator of load activation. 1D shape=(TW)
  Xbyak::Zmm dst_tile_Vmm(int i, int j);  // Reg alloc of DST tile. 2D shape=(TH,TW), stride=(TW,1)
  void params_alias(const ssd::vnni_param_t& param);
  void read_params();
  void load_bias(dim_t m_start);
  void load_dense(const std::vector<int64_t>& k_indices);
  void load_sparse(const Xbyak::Reg64& reg_addr, uint64_t offset);
  void tile_product(int tile_height, int tile_width);
  void handle_dst_buffer_init(int kb_idx, dim_t m_start);
  void handle_dst_buffer_epilogue(int kb_idx, dim_t m_start);
  void mul_scale(int i);
  void move_out(int i, int j, int row_idx, int bytes = 1);
  void repeat_THx4xTW_matmal(dim_t m_start);
  void clear_dst_tile();
  void load_intermediate_dst(dim_t m_start);
  void store_intermediate_dst(dim_t m_start);
  void gen_subfunc_tile_prod();
  void gen_subfunc_load_and_prod();

  inline int TH() const { return param_.blocksize[0]; }
  inline int TW() const { return param_.tile_w; }
  inline int nt_size() const { return TW() * VEC; }
  inline int mt_size() const { return TH(); }
  inline int n_tiles() const { return param_.BN / nt_size(); }
  inline int m_tiles() const { return param_.BM / mt_size(); }
  inline data_type output_type() const { return param_.output_type; }
  inline int ld_dst() const { return param_.BN; }  // leading dimension of dst matrix

 private:
  const int64_t PADDED_NEG_ONE = -1;
  const int64_t PADDED_ZERO = 0;
  const uint8_t* sfptr_tile_prod_ = nullptr;  // subfunction for tile product
  const uint8_t* sfptr_load_prod_ = nullptr;  // subfunction for dense load & tile product

 private:
  static constexpr int stack_space_needed_ = 200;
  static constexpr int BYTE8 = 8;
  static constexpr int BYTE4 = 4;
  static constexpr int BYTE1 = 1;
  static constexpr int VREG_NUMS = 32;
#ifdef XBYAK64
  static constexpr int PTR_SIZE = 8;
#else
  static constexpr int PTR_SIZE = 4;
#endif
  // Register decomposition
  const Xbyak::Reg64& param1 = rdi;

  const Xbyak::Reg64& reg_wei = rcx;
  const Xbyak::Reg64& reg_dense = rdx;  // the second argument which is input matrix pointer
  const Xbyak::Reg64& reg_bias = rsi;   // the third argument which is bias values pointer
  const Xbyak::Reg64& reg_dst = rax;    // the fourth argument which is output matrix pointer
  const Xbyak::Reg64& reg_scale = rbx;  // the scale
  const Xbyak::Opmask& reg_k1 = k1;

  const Xbyak::Reg64& reg_tmp = r9;
  const Xbyak::Reg64& reg_n_idx = r10;
  const Xbyak::Reg64& reg_seq_indices = r11;
  const Xbyak::Reg64 reg_addr_tmp[4] = {r12, r13, r14, r15};

  const Xbyak::Zmm& vpermt2d_arg_idx = zmm31;
  const Xbyak::Zmm& vpshufb_arg_b = zmm30;
  const Xbyak::Zmm& vreg_temp = zmm29;
  const Xbyak::Zmm& vreg_dst_temp = vreg_temp;
  static constexpr int USED_VREGS = 3;
};
}  // namespace jd
#endif  // ENGINE_SPARSELIB_INCLUDE_JIT_DOMAIN_JIT_SPMM_VNNI_HPP_