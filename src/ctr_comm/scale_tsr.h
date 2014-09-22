/*Copyright (c) 2011, Edgar Solomonik, all rights reserved.*/

#ifndef __SCL_TSR_H__
#define __SCL_TSR_H__

#include "../shared/util.h"

class scl {
  public:
    char * A; 
    char * alpha;
    void * buffer;

    virtual void run() {};
    virtual int64_t mem_fp() { return 0; };
    virtual scl * clone() { return NULL; };
    
    virtual ~scl(){ if (buffer != NULL) CTF_free(buffer); }
    scl(scl * other);
    scl(){ buffer = NULL; }
};

class scl_virt : public scl {
  public: 
    /* Class to be called on sub-blocks */
    scl * rec_scl;

    int num_dim;
    int * virt_dim;
    int order_A;
    int64_t blk_sz_A;
    int const * idx_map_A;
    
    void run();
    int64_t mem_fp();
    scl * clone();
    
    scl_virt(scl * other);
    ~scl_virt();
    scl_virt(){}
};



#endif // __SCL_TSR_H__
