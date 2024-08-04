#ifndef _PARSETCOMMON_H_
#define _PARSETCOMMON_H_

#include "typedefs.h"

typedef struct pic_parameter_set_rbsp_t
{
    Boolean   Valid;                  // indicates the parameter set is valid
    unsigned int pic_parameter_set_id;                             // ue(v)
    unsigned int seq_parameter_set_id;                             // ue(v)
    int       pic_init_qp_minus26;
} pic_parameter_set_rbsp_t;

typedef struct seq_parameter_set_rbsp_t
{

} seq_parameter_set_rbsp_t;
#endif
