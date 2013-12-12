/*Copyright (c) 2011, Edgar Solomonik, all rights reserved.*/

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "../../include/ctf.hpp"
#include "../shared/util.h"

template<typename dtype>
tCTF_Tensor<dtype>::tCTF_Tensor(){
  tid = -1;
  ndim = -1;
  sym = NULL;
  len = NULL;
  name = NULL;
  world = NULL;
}

template<typename dtype>
tCTF_Tensor<dtype>::tCTF_Tensor(const tCTF_Tensor<dtype>& A,
                                 bool                copy){
  int ret;
  world = A.world;
  name = A.name;

  ret = world->ctf->info_tensor(A.tid, &ndim, &len, &sym);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);

  ret = world->ctf->define_tensor(ndim, len, sym, &tid, name, name!=NULL);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);

  //printf("Defined tensor %d to be the same as %d, copy=%d\n", tid, A.tid, (int)copy);

  if (copy){
    ret = world->ctf->copy_tensor(A.tid, tid);
    LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  }
}

template<typename dtype>
tCTF_Tensor<dtype>::tCTF_Tensor(int                 ndim_,
                                int const *         len_,
                                int const *         sym_,
                                tCTF_World<dtype> & world_,
                                char const *        name_,
                                int const           profile_){
  int ret;
  world = &world_;
  name = name_;
  ret = world->ctf->define_tensor(ndim_, len_, sym_, &tid, name_, profile_);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  ret = world->ctf->info_tensor(tid, &ndim, &len, &sym);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
tCTF_Tensor<dtype>::~tCTF_Tensor(){
/*  if (sym != NULL)
    CTF_free_cond(sym);
  if (len != NULL)
    CTF_free_cond(len);*/
  if (sym != NULL)
    free(sym);
  if (len != NULL)
    free(len);
  world->ctf->clean_tensor(tid);
}

template<typename dtype>
dtype * tCTF_Tensor<dtype>::get_raw_data(long_int * size) {
  int ret;
  dtype * data;
  ret = world->ctf->get_raw_data(tid, &data, size);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  
  return data;
}

template<typename dtype>
void tCTF_Tensor<dtype>::read_local(long_int *   npair, 
                                    long_int **  global_idx, 
                                    dtype **   data) const {
  tkv_pair< dtype > * pairs;
  int ret, i;
  ret = world->ctf->read_local_tensor(tid, npair, &pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  /* FIXME: careful with CTF_alloc */
  *global_idx = (long_int*)malloc((*npair)*sizeof(long_int));
  *data = (dtype*)malloc((*npair)*sizeof(dtype));
  for (i=0; i<(*npair); i++){
    (*global_idx)[i] = pairs[i].k;
    (*data)[i] = pairs[i].d;
  }
  if (*npair > 0)
    free(pairs);
}

template<typename dtype>
void tCTF_Tensor<dtype>::read_local(long_int *   npair,
                                    tkv_pair<dtype> **   pairs) const {
  int ret = world->ctf->read_local_tensor(tid, npair, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
void tCTF_Tensor<dtype>::read(long_int          npair, 
                              long_int const *  global_idx, 
                              dtype *           data) const {
  int ret, i;
  tkv_pair< dtype > * pairs;
  pairs = (tkv_pair< dtype >*)CTF_alloc(npair*sizeof(tkv_pair< dtype >));
  for (i=0; i<npair; i++){
    pairs[i].k = global_idx[i];
  }
  ret = world->ctf->read_tensor(tid, npair, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  for (i=0; i<npair; i++){
    data[i] = pairs[i].d;
  }
  CTF_free(pairs);
}

template<typename dtype>
void tCTF_Tensor<dtype>::read(long_int          npair,
                              tkv_pair<dtype> * pairs) const {
  int ret = world->ctf->read_tensor(tid, npair, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
void tCTF_Tensor<dtype>::write(long_int          npair, 
                               long_int const *  global_idx, 
                               dtype const *     data) {
  int ret, i;
  tkv_pair< dtype > * pairs;
  pairs = (tkv_pair< dtype >*)CTF_alloc(npair*sizeof(tkv_pair< dtype >));
  for (i=0; i<npair; i++){
    pairs[i].k = global_idx[i];
    pairs[i].d = data[i];
  }
  ret = world->ctf->write_tensor(tid, npair, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  CTF_free(pairs);
}

template<typename dtype>
void tCTF_Tensor<dtype>::write(long_int                npair,
                               tkv_pair<dtype> const * pairs) {
  int ret = world->ctf->write_tensor(tid, npair, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
void tCTF_Tensor<dtype>::write(long_int         npair, 
                               dtype            alpha, 
                               dtype            beta,
                               long_int const * global_idx, 
                               dtype const *    data) {
  int ret, i;
  tkv_pair< dtype > * pairs;
  pairs = (tkv_pair< dtype >*)CTF_alloc(npair*sizeof(tkv_pair< dtype >));
  for (i=0; i<npair; i++){
    pairs[i].k = global_idx[i];
    pairs[i].d = data[i];
  }
  ret = world->ctf->write_tensor(tid, npair, alpha, beta, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  CTF_free(pairs);
}

template<typename dtype>
void tCTF_Tensor<dtype>::write(long_int          npair,
                               dtype             alpha,
                               dtype             beta,
                               tkv_pair<dtype> const * pairs) {
  int ret = world->ctf->write_tensor(tid, npair, alpha, beta, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
void tCTF_Tensor<dtype>::read(long_int         npair, 
                              dtype            alpha, 
                              dtype            beta,
                              long_int const * global_idx, 
                              dtype *          data) const{
  int ret, i;
  tkv_pair< dtype > * pairs;
  pairs = (tkv_pair< dtype >*)CTF_alloc(npair*sizeof(tkv_pair< dtype >));
  for (i=0; i<npair; i++){
    pairs[i].k = global_idx[i];
    pairs[i].d = data[i];
  }
  ret = world->ctf->read_tensor(tid, npair, alpha, beta, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  for (i=0; i<npair; i++){
    data[i] = pairs[i].d;
  }
  CTF_free(pairs);
}

template<typename dtype>
void tCTF_Tensor<dtype>::read(long_int          npair,
                              dtype             alpha,
                              dtype             beta,
                              tkv_pair<dtype> * pairs) const{
  int ret = world->ctf->read_tensor(tid, npair, alpha, beta, pairs);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}


template<typename dtype>
void tCTF_Tensor<dtype>::read_all(long_int * npair, dtype ** vals) const {
  int ret;
  ret = world->ctf->allread_tensor(tid, npair, vals);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
long_int tCTF_Tensor<dtype>::read_all(dtype * vals) const {
  int ret;
  long_int npair;
  ret = world->ctf->allread_tensor(tid, &npair, vals);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  return npair;
}

template<typename dtype>
int64_t tCTF_Tensor<dtype>::estimate_cost(
                                  const tCTF_Tensor<dtype>&     A,
                                  const char *                  idx_A,
                                  const tCTF_Tensor<dtype>&     B,
                                  const char *                  idx_B,
                                  const char *                  idx_C){
  int * idx_map_A, * idx_map_B, * idx_map_C;
  conv_idx(A.ndim, idx_A, &idx_map_A,
           B.ndim, idx_B, &idx_map_B,
           ndim, idx_C, &idx_map_C);
  return world->ctf->estimate_cost(A.tid, idx_map_A, B.tid, idx_map_B, tid, idx_map_C);
}

template<typename dtype>
int64_t tCTF_Tensor<dtype>::estimate_cost(
                                  const tCTF_Tensor<dtype>&     A,
                                  const char *                  idx_A,
                                  const char *                  idx_B){
  int * idx_map_A, * idx_map_B;
  CTF_sum_type_t st;
  conv_idx(A.ndim, idx_A, &idx_map_A,
           ndim, idx_B, &idx_map_B);
  return world->ctf->estimate_cost(A.tid, idx_map_A, tid, idx_map_B);

  
}

template<typename dtype>
void tCTF_Tensor<dtype>::contract(dtype                         alpha,
                                  const tCTF_Tensor<dtype>&     A,
                                  const char *                  idx_A,
                                  const tCTF_Tensor<dtype>&     B,
                                  const char *                  idx_B,
                                  dtype                         beta,
                                  const char *                  idx_C,
                                  tCTF_fctr<dtype>              fseq){
  int ret;
  CTF_ctr_type_t tp;
  tp.tid_A = A.tid;
  tp.tid_B = B.tid;
  tp.tid_C = tid;
  conv_idx(A.ndim, idx_A, &tp.idx_map_A,
           B.ndim, idx_B, &tp.idx_map_B,
           ndim, idx_C, &tp.idx_map_C);
  LIBT_ASSERT(A.world->ctf == world->ctf);
  LIBT_ASSERT(B.world->ctf == world->ctf);
  if (fseq.func_ptr == NULL)
    ret = world->ctf->contract(&tp, alpha, beta);
  else {
    fseq_elm_ctr<dtype> fs;
    fs.func_ptr = fseq.func_ptr;
    ret = world->ctf->contract(&tp, fs, alpha, beta);
  }
  CTF_free(tp.idx_map_A);
  CTF_free(tp.idx_map_B);
  CTF_free(tp.idx_map_C);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
void tCTF_Tensor<dtype>::set_name(char const * name_) {
  name = name_;
  world->ctf->set_name(tid, name_);
}

template<typename dtype>
void tCTF_Tensor<dtype>::profile_on() {
  world->ctf->profile_on(tid);
}

template<typename dtype>
void tCTF_Tensor<dtype>::profile_off() {
  world->ctf->profile_off(tid);
}

template<typename dtype>
void tCTF_Tensor<dtype>::print(FILE* fp, double cutoff) const{
  world->ctf->print_tensor(fp, tid, cutoff);
}

template<typename dtype>
void tCTF_Tensor<dtype>::compare(const tCTF_Tensor<dtype>& A, FILE* fp, double cutoff) const{
  world->ctf->compare_tensor(fp, tid, A.tid, cutoff);
}

template<typename dtype>
void tCTF_Tensor<dtype>::sum(dtype                      alpha,
                             const tCTF_Tensor<dtype>&  A,
                             const char *               idx_A,
                             dtype                      beta,
                             const char *               idx_B,
                             tCTF_fsum<dtype>           fseq){
  int ret;
  int * idx_map_A, * idx_map_B;
  CTF_sum_type_t st;
  conv_idx(A.ndim, idx_A, &idx_map_A,
           ndim, idx_B, &idx_map_B);
  LIBT_ASSERT(A.world->ctf == world->ctf);
    
  st.idx_map_A = idx_map_A;
  st.idx_map_B = idx_map_B;
  st.tid_A = A.tid;
  st.tid_B = tid;
  if (fseq.func_ptr == NULL)
    ret = world->ctf->sum_tensors(&st, alpha, beta);
  else {
    fseq_elm_sum<dtype> fs;
    fs.func_ptr = fseq.func_ptr;
    ret = world->ctf->sum_tensors(alpha, beta, A.tid, tid, idx_map_A, idx_map_B, fs);
  }
  CTF_free(idx_map_A);
  CTF_free(idx_map_B);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
void tCTF_Tensor<dtype>::scale(dtype                  alpha, 
                               const char *           idx_A,
                               tCTF_fscl<dtype>       fseq){
  int ret;
  int * idx_map_A;
  conv_idx(ndim, idx_A, &idx_map_A);
  if (fseq.func_ptr == NULL)
    ret = world->ctf->scale_tensor(alpha, tid, idx_map_A);
  else{
    fseq_elm_scl<dtype> fs;
    fs.func_ptr = fseq.func_ptr;
    ret = world->ctf->scale_tensor(alpha, tid, idx_map_A, fs);
  }
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}
template<typename dtype>


void tCTF_Tensor<dtype>::permute(dtype                  beta,
                                 tCTF_Tensor &          A,
                                 int * const *           perms_A,
                                 dtype                  alpha){
  int ret = world->ctf->permute_tensor(A.tid, perms_A, alpha, A.world->ctf, 
                                       tid, NULL, beta, world->ctf);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
void tCTF_Tensor<dtype>::permute(int * const *           perms_B,
                                 dtype                  beta,
                                 tCTF_Tensor &          A,
                                 dtype                  alpha){
  int ret = world->ctf->permute_tensor(A.tid, NULL, alpha, A.world->ctf, 
                                       tid, perms_B, beta, world->ctf);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}
template<typename dtype>
void tCTF_Tensor<dtype>::add_to_subworld(
                         tCTF_Tensor<dtype> * tsr,
                         double alpha,
                         double beta) const {
  ABORT;
}

template<typename dtype>
void tCTF_Tensor<dtype>::add_from_subworld(
                         tCTF_Tensor<dtype> * tsr,
                         double alpha,
                         double beta) const {
  ABORT;
}


template<typename dtype>
void tCTF_Tensor<dtype>::slice(int const *    offsets,
                               int const *    ends,
                               dtype          beta,
                               tCTF_Tensor const &  A,
                               int const *    offsets_A,
                               int const *    ends_A,
                               dtype          alpha) const {
  int ret, np_A, np_B;
  if (A.world->comm != world->comm){
    MPI_Comm_size(A.world->comm, &np_A);
    MPI_Comm_size(world->comm,   &np_B);
    LIBT_ASSERT(np_A != np_B);
    ret = world->ctf->slice_tensor(
              A.tid, offsets_A, ends_A, alpha, A.world->ctf, 
              tid, offsets, ends, beta);
  } else {
    ret =  world->ctf->slice_tensor(A.tid, offsets_A, ends_A, alpha,
                                        tid, offsets, ends, beta);
  }
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
void tCTF_Tensor<dtype>::slice(long_int       corner_off,
                               long_int       corner_end,
                               dtype          beta,
                               tCTF_Tensor const &  A,
                               long_int       corner_off_A,
                               long_int       corner_end_A,
                               dtype          alpha) const {
  int * offsets, * ends, * offsets_A, * ends_A;
 
  conv_idx(this->ndim, this->len, corner_off, &offsets);
  conv_idx(this->ndim, this->len, corner_end, &ends);
  conv_idx(A.ndim, A.len, corner_off_A, &offsets_A);
  conv_idx(A.ndim, A.len, corner_end_A, &ends_A);
  
  slice(offsets, ends, beta, A, offsets_A, ends_A, alpha);

  CTF_free(offsets);
  CTF_free(ends);
  CTF_free(offsets_A);
  CTF_free(ends_A);
}

template<typename dtype>
tCTF_Tensor<dtype> tCTF_Tensor<dtype>::slice(int const *          offsets,
                                             int const *          ends) const {

  return slice(offsets, ends, world);
}

template<typename dtype>
tCTF_Tensor<dtype> tCTF_Tensor<dtype>::slice(long_int corner_off,
                                             long_int corner_end) const {

  return slice(corner_off, corner_end, world);
}




template<typename dtype>
tCTF_Tensor<dtype> tCTF_Tensor<dtype>::slice(int const *          offsets,
                                             int const *          ends,
                                             tCTF_World<dtype> *  owrld) const {
  int i;
  int * new_lens = (int*)CTF_alloc(sizeof(int)*ndim);
  int * new_sym = (int*)CTF_alloc(sizeof(int)*ndim);
  for (i=0; i<ndim; i++){
    LIBT_ASSERT(ends[i] - offsets[i] > 0 && 
                offsets[i] >= 0 && 
                ends[i] <= len[i]);
    if (sym[i] != NS){
      if (offsets[i] == offsets[i+1] && ends[i] == ends[i+1]){
        new_sym[i] = sym[i];
      } else {
        LIBT_ASSERT(ends[i+1] >= offsets[i]);
        new_sym[i] = NS;
      }
    } else new_sym[i] = NS;
    new_lens[i] = ends[i] - offsets[i];
  }
  tCTF_Tensor<dtype> new_tsr(ndim, new_lens, new_sym, *owrld);
  std::fill(new_sym, new_sym+ndim, 0);
  new_tsr.slice(new_sym, new_lens, 0.0, *this, offsets, ends, 1.0);
  CTF_free(new_lens);
  CTF_free(new_sym);
  return new_tsr;
}

template<typename dtype>
tCTF_Tensor<dtype> tCTF_Tensor<dtype>::slice(long_int             corner_off,
                                             long_int             corner_end,
                                             tCTF_World<dtype> *  owrld) const {

  int * offsets, * ends;
 
  conv_idx(this->ndim, this->len, corner_off, &offsets);
  conv_idx(this->ndim, this->len, corner_end, &ends);
  
  tCTF_Tensor tsr = slice(offsets, ends, owrld);

  CTF_free(offsets);
  CTF_free(ends);

  return tsr;
}

template<typename dtype>
void tCTF_Tensor<dtype>::align(const tCTF_Tensor& A){
  if (A.world->ctf != world->ctf) {
    printf("ERROR: cannot align tensors on different CTF instances\n");
    ABORT;
  }
  int ret = world->ctf->align(tid, A.tid);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
dtype tCTF_Tensor<dtype>::reduce(CTF_OP op){
  int ret;
  dtype ans;
  ans = 0.0;
  ret = world->ctf->reduce_tensor(tid, op, &ans);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
  return ans;
}
template<typename dtype>
void tCTF_Tensor<dtype>::get_max_abs(int        n,
                                     dtype *    data){
  int ret;
  ret = world->ctf->get_max_abs(tid, n, data);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}

template<typename dtype>
tCTF_Tensor<dtype>& tCTF_Tensor<dtype>::operator=(dtype val){
  long_int size;
  dtype* raw = get_raw_data(&size);
  std::fill(raw, raw+size, val);
  return *this;
}

template<typename dtype>
void tCTF_Tensor<dtype>::operator=(tCTF_Tensor<dtype> A){
  int ret;  

  world = A.world;
  name = A.name;

  if (sym != NULL)
    free(sym);
  if (len != NULL)
    free(len);
    //free(len);
  ret = world->ctf->info_tensor(A.tid, &ndim, &len, &sym);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);

  ret = world->ctf->define_tensor(ndim, len, sym, &tid, name, name != NULL);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);

  //printf("Set tensor %d to be the same as %d\n", tid, A.tid);

  ret = world->ctf->copy_tensor(A.tid, tid);
  LIBT_ASSERT(ret == DIST_TENSOR_SUCCESS);
}
    

template<typename dtype>
tCTF_Idx_Tensor<dtype> tCTF_Tensor<dtype>::operator[](const char * idx_map_){
  tCTF_Idx_Tensor<dtype> itsr(this, idx_map_);
  return itsr;
}


template<typename dtype>
tCTF_Sparse_Tensor<dtype> tCTF_Tensor<dtype>::operator[](std::vector<int64_t> indices){
  tCTF_Sparse_Tensor<dtype> stsr(indices,this);
  return stsr;
}





template class tCTF_Tensor<double>;
#ifdef CTF_COMPLEX
template class tCTF_Tensor< std::complex<double> >;
#endif
