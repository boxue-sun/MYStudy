#ifndef __FF_H264_PARAMETERS_H__
#define __FF_H264_PARAMETERS_H__

#include <cstdint>
#include <cmath>
#include <vector>

extern "C" {
    #include <libavcodec/avcodec.h>
}


class ff_h264_parameters
{
public:
    static uint32_t Ue(uint8_t *pBuff, uint8_t nLen, uint8_t &nStartBit);

    static int Se(uint8_t *pBuff, uint8_t nLen, uint8_t &nStartBit);

    static uint32_t u(uint8_t BitCount, uint8_t *buf, uint8_t &nStartBit);

    static bool get_resolution(const AVPacket *packet, int &Width, int& Height);

};

#endif