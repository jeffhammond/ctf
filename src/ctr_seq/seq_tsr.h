#ifndef __SEQ_TSR_H__
#define __SEQ_TSR_H__

#include "../shared/util.h"
#include "../ctr_comm/ctr_comm.h"
#include "../ctr_comm/sum_tsr.h"
#include "../ctr_comm/scale_tsr.h"
#include "sym_seq_shared.hxx"
#include "sym_seq_sum_inner.hxx"
#include "sym_seq_ctr_inner.hxx"
#include "sym_seq_scl_ref.hxx"
#include "sym_seq_sum_ref.hxx"
#include "sym_seq_ctr_ref.hxx"
#include "sym_seq_scl_cust.hxx"
#include "sym_seq_sum_cust.hxx"
#include "sym_seq_ctr_cust.hxx"

class seq_tsr_ctr : public ctr {
  public:
    char * alpha;
    int order_A;
    int * edge_len_A;
    int const * idx_map_A;
    int * sym_A;
    int order_B;
    int * edge_len_B;
    int const * idx_map_B;
    int * sym_B;
    int order_C;
    int * edge_len_C;
    int const * idx_map_C;
    int * sym_C;
    //fseq_tsr_ctr func_ptr;

    int is_inner;
    iparam inner_params;
    
    int is_custom;
    bivar_function func; // custom_params;

    void run();
    void print();
    int64_t mem_fp();
    double est_time_rec(int nlyr);
    double est_time_fp(int nlyr);
    ctr * clone();

    seq_tsr_ctr(ctr * other);
    ~seq_tsr_ctr(){ CTF_free(edge_len_A), CTF_free(edge_len_B), CTF_free(edge_len_C), 
                    CTF_free(sym_A), CTF_free(sym_B), CTF_free(sym_C); }
    seq_tsr_ctr(){}
};

class seq_tsr_sum : public tsum {
  public:
    int order_A;
    int * edge_len_A;
    int const * idx_map_A;
    int * sym_A;
    int order_B;
    int * edge_len_B;
    int const * idx_map_B;
    int * sym_B;
    //fseq_tsr_sum func_ptr;

    int is_inner;
    int inr_stride;
    
    int is_custom;
    univar_function func; //fseq_elm_sum custom_params;
    

    void run();
    void print();
    int64_t mem_fp();
    tsum * clone();

    seq_tsr_sum(tsum * other);
    ~seq_tsr_sum(){ CTF_free(edge_len_A), CTF_free(edge_len_B), 
                    CTF_free(sym_A), CTF_free(sym_B); };
    seq_tsr_sum(){}
};

class seq_tsr_scl : public scl {
  public:
    int order;
    int * edge_len;
    int const * idx_map;
    int const * sym;
    //fseq_tsr_scl func_ptr;

    int is_custom;
    endomorphism func; //fseq_elm_scl custom_params;

    void run();
    void print();
    int64_t mem_fp();
    scl * clone();

    seq_tsr_scl(scl * other);
    ~seq_tsr_scl(){ CTF_free(edge_len); };
    seq_tsr_scl(){}
};


#endif
