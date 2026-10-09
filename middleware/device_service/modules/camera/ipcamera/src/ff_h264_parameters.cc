#include "ipcamera/include/ff_h264_parameters.h"


uint32_t ff_h264_parameters::Ue(uint8_t *pBuff, uint8_t nLen, uint8_t &nStartBit) {
    //计算0bit的个数
    uint8_t nZeroNum = 0;
    while (nStartBit < nLen * 8)
    {
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        { //&:按位与，%取余
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit++;

    //计算结果
    uint32_t dwRet = 0;
    for (uint8_t i = 0; i < nZeroNum; i++)
    {
        dwRet <<= 1;
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8)))
        {
            dwRet += 1;
        }
        nStartBit++;
    }
    return (1 << nZeroNum) - 1 + dwRet;
}

int ff_h264_parameters::Se(uint8_t *pBuff, uint8_t nLen, uint8_t &nStartBit) {
    uint8_t nZeroNum = 0;
    while(nStartBit < nLen * 8) {
        if(pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) { //&:按位与，%取余
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit ++;

    //计算结果
    uint32_t dwRet = 0;
    for(uint8_t i=0; i<nZeroNum; i++) {
        dwRet <<= 1;
        if(pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
            dwRet += 1;
        }
        nStartBit++;
    }
    
    int UeVal= (1 << nZeroNum) - 1 + dwRet;
    double k=UeVal;
    int nValue=std::ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
    if(UeVal % 2==0)
        nValue=-nValue;
    return nValue;
}

uint32_t ff_h264_parameters::u(uint8_t BitCount, uint8_t *buf, uint8_t &nStartBit) {
    uint32_t dwRet = 0;
    for (uint8_t i = 0; i < BitCount; i++) {
        dwRet <<= 1;
        if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
            dwRet += 1;
        }
        nStartBit++;
    }
    return dwRet;
}

bool ff_h264_parameters::get_resolution(const AVPacket *packet, int &Width, int& Height) {

    uint8_t *buf = new uint8_t[1024];
    int nLen;
    bool bSpsComplete = false;

    if(packet->flags & AV_PKT_FLAG_KEY) {
        uint8_t* p = packet->data;
        uint8_t last_nal_type = 0;
        int last_nal_pos = 0;
        for(int i=0; i<packet->size-5; i++) {
            p = packet->data + i;
            if(p[0]==0x00&&p[1]==0x00&&p[2]==0x00&&p[3]==0x01) {
                if(last_nal_type == 0x67) {
                    nLen = i-last_nal_pos;
                    memcpy(buf, packet->data+last_nal_pos, nLen);
                    bSpsComplete = true;
                }
                last_nal_type = p[4];
                last_nal_pos = i;
                if(bSpsComplete) {
                    break;
                }
            }
        }
        if(last_nal_type == 0x67 && bSpsComplete == false) {
            nLen = packet->size - last_nal_pos;
            memcpy(buf, packet->data+last_nal_pos, nLen);
            bSpsComplete = true;
        }
    }

    if (!bSpsComplete) {
        delete[] buf;
        return false;
    }

    // Analyze SPS to find width and height
    uint8_t StartBit=0;
    uint8_t *blk = buf;
    buf = buf + 4;
    int forbidden_zero_bit=u(1,buf,StartBit);
    int nal_ref_idc=u(2,buf,StartBit);
    int nal_unit_type=u(5,buf,StartBit);
    if(nal_unit_type==7) {
        int profile_idc=u(8,buf,StartBit);
        int constraint_set0_flag=u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
        int constraint_set1_flag=u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
        int constraint_set2_flag=u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
        int constraint_set3_flag=u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
        int reserved_zero_4bits=u(4,buf,StartBit);
        int level_idc=u(8,buf,StartBit);

        int seq_parameter_set_id=Ue(buf,nLen,StartBit);

        if( profile_idc == 100 || profile_idc == 110 ||
            profile_idc == 122 || profile_idc == 144 ) {
            int chroma_format_idc=Ue(buf,nLen,StartBit);
            if( chroma_format_idc == 3 )
                int residual_colour_transform_flag=u(1,buf,StartBit);
            int bit_depth_luma_minus8=Ue(buf,nLen,StartBit);
            int bit_depth_chroma_minus8=Ue(buf,nLen,StartBit);
            int qpprime_y_zero_transform_bypass_flag=u(1,buf,StartBit);
            int seq_scaling_matrix_present_flag=u(1,buf,StartBit);

            int seq_scaling_list_present_flag[8];
            if( seq_scaling_matrix_present_flag ) {
                for(int i = 0; i < 8; i++ ) {
                    seq_scaling_list_present_flag[i]=u(1,buf,StartBit);
                }
            }
        }
        int log2_max_frame_num_minus4=Ue(buf,nLen,StartBit);
        int pic_order_cnt_type=Ue(buf,nLen,StartBit);
        if( pic_order_cnt_type == 0 ) {
            int log2_max_pic_order_cnt_lsb_minus4=Ue(buf,nLen,StartBit);
        } else if( pic_order_cnt_type == 1 ) {
            int delta_pic_order_always_zero_flag=u(1,buf,StartBit);
            int offset_for_non_ref_pic=Se(buf,nLen,StartBit);
            int offset_for_top_to_bottom_field=Se(buf,nLen,StartBit);
            int num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,nLen,StartBit);

            int *offset_for_ref_frame=new int[num_ref_frames_in_pic_order_cnt_cycle];
            for(int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
                offset_for_ref_frame[i]=Se(buf,nLen,StartBit);
            delete[] offset_for_ref_frame;
        }
        
        int num_ref_frames=Ue(buf,nLen,StartBit);
        int gaps_in_frame_num_value_allowed_flag=u(1,buf,StartBit);
        int pic_width_in_mbs_minus1=Ue(buf,nLen,StartBit);
        int pic_height_in_map_units_minus1=Ue(buf,nLen,StartBit);

        Width=(pic_width_in_mbs_minus1+1)*16;
        Height=(pic_height_in_map_units_minus1+1)*16;
        delete[] blk;
        return true;
    } else {
        delete[] blk;
        return false;
    }
}