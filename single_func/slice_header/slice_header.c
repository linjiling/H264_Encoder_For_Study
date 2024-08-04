#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "vlc_base.h"
#include "types.h"

static const byte assignSE2partition[][SE_MAX_ELEMENTS] =
{
  // 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  // element number (do not uncomment)
  {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },   //!< all elements in one partition no data partitioning
  {  0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 2, 2, 2, 2, 0, 0, 0, 0 }    //!< three partitions per slice
};

int dec_ref_pic_marking(Bitstream *bitstream, DecRefPicMarking_t *p_drpm, int idr_flag, int no_output_of_prior_pics_flag, int long_term_reference_flag )
{
    int len = 0;

    if (idr_flag)
    {
      len += write_u_1("SH: no_output_of_prior_pics_flag", no_output_of_prior_pics_flag, bitstream);
      len += write_u_1("SH: long_term_reference_flag", long_term_reference_flag, bitstream);
    }
    return len;
}

static int get_picture_type(Slice *currSlice)
{
  // set this value to zero for transmission without signaling
  // that the whole picture has the same slice type
  int same_slicetype_for_whole_frame = 5;

  switch (currSlice->slice_type)
  {
  case I_SLICE:
    return 2 + same_slicetype_for_whole_frame;
    break;
  case P_SLICE:
    return 0 + same_slicetype_for_whole_frame;
    break;
  case B_SLICE:
    return 1 + same_slicetype_for_whole_frame;
    break;
  case SP_SLICE:
    return 3 + same_slicetype_for_whole_frame;
    break;
  case SI_SLICE:
    return 4 + same_slicetype_for_whole_frame;
    break;
  default:
    printf("Picture Type not supported!\n");
    break;
  }

  return 0;
}

int SliceHeader(Slice* currSlice)
{
    VideoParameters *p_Vid = currSlice->p_Vid;
    pic_parameter_set_rbsp_t *active_pps = currSlice->active_pps;

    int dP_nr = assignSE2partition[currSlice->partition_mode][SE_HEADER];
    Bitstream *bitstream = currSlice->partArr[dP_nr].bitstream;
    int len = 0;
    unsigned int field_pic_flag = 0;
    byte bottom_field_flag = 0;

    int num_bits_slice_group_change_cycle;
    float numtmp;

    len  = write_ue_v("SH: first_mb_in_slice", p_Vid->current_mb_nr,   bitstream);
    len += write_ue_v("SH: slice_type", get_picture_type(currSlice),   bitstream);
    len += write_ue_v("SH: pic_parameter_set_id" , active_pps->pic_parameter_set_id ,bitstream);
    len += write_u_v (p_Vid->log2_max_frame_num_minus4 + 4,"SH: frame_num", p_Vid->frame_num, bitstream);
    if (currSlice->idr_flag)
    {
      // idr_pic_id
      len += write_ue_v ("SH: idr_pic_id", (p_Vid->number & 0x01), bitstream);
    }

    len += write_se_v("SH: slice_qp_delta", (currSlice->qp - 26 - active_pps->pic_init_qp_minus26), bitstream);

    return len;
}

int main(int argc, char *argv[])
{
    Slice currSlice;
    int i;

    VideoParameters p_Vid;
    pic_parameter_set_rbsp_t active_pps;
    DataPartition partArr[3];
    Bitstream     bitstream;
    byte *pstream = (byte *)malloc(1024);

    currSlice.p_Vid = &p_Vid;
    currSlice.active_pps = &active_pps;

    memset(pstream, 0, 1024);
    memset(&bitstream, 0, sizeof(Bitstream));
    bitstream.streamBuffer = pstream;
    bitstream.bits_to_go = 8;
    partArr[0].bitstream = &bitstream;
    currSlice.partArr = partArr;

    currSlice.partition_mode = 0;
    p_Vid.current_mb_nr = 0;
    currSlice.slice_type = I_SLICE;
    active_pps.pic_parameter_set_id = 0;
    p_Vid.frame_num = 0;
    p_Vid.log2_max_frame_num_minus4 = 4;
    currSlice.idr_flag = 1;
    p_Vid.number = 0;
    currSlice.qp = 28;
    active_pps.pic_init_qp_minus26 = 0;


    SliceHeader(&currSlice);

    for (i = 0; i < bitstream.byte_pos; i++) {
   	printf("0x%x ", pstream[i]);
    }
    printf("\n");
    return 0;
}
