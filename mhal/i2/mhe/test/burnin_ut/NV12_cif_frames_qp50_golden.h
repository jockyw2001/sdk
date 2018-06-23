/*
 * NV12_qcif_frames_qp50_golden.h
 *
 *  Created on: Oct 26, 2017
 *      Author: derek.lee
 */

#ifndef DRIVER_MHE_KERNEL_UT_NV12_QCIF_FRAMES_QP50_GOLDEN_H_
#define DRIVER_MHE_KERNEL_UT_NV12_QCIF_FRAMES_QP50_GOLDEN_H_

unsigned int bsp_out_0_Size = 400+81;
unsigned int bsp_out_1_Size = 88+10;
unsigned int bsp_out_2_Size = 121+10;
unsigned int bsp_out_3_Size = 122+10;
unsigned int bsp_out_4_Size = 124+10;
unsigned int bsp_out_5_Size = 119+10;
unsigned int bsp_out_6_Size = 96+10;
unsigned int bsp_out_7_Size = 116+10;
unsigned int bsp_out_8_Size = 82+10;
unsigned int bsp_out_9_Size = 71+10;

static const unsigned char gH265_qp50[] =
{
    0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0x0C, 0x01, 0xFF, 0xFF, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00,
    0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x96, 0xAC, 0x09, 0x00, 0x00, 0x00, 0x01,
    0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x03, 0x00, 0x96, 0xA0, 0x0B, 0x08, 0x04, 0x87, 0xF9, 0x6B, 0xB9, 0x1B, 0x06, 0xBB, 0x04, 0x00,
    0x00, 0x00, 0x01, 0x44, 0x01, 0xC0, 0x71, 0x81, 0x12, 0x00, 0x00, 0x01, 0x26, 0x01, 0xAC, 0x18,
    0x60,
    64,132,4,225,112,25,187,251,106,76,72,216,229,0,228,143,59,107,200,180,63,233,186,197,168,173,130,18,101,147,106,169,42,187,163,79,90,31,26,102,160,221,125,78,78,146,119,131,104,54,94,224,189,201,152,213,
    111,123,28,168,67,23,124,208,36,54,46,20,69,44,40,19,235,236,67,134,107,158,72,77,105,107,107,151,248,119,11,87,107,162,223,92,245,159,173,4,232,251,172,46,17,214,10,114,110,14,27,180,14,249,17,151,62,
    22,187,70,193,156,66,105,67,147,171,90,202,23,166,241,206,73,153,91,204,110,120,198,9,120,194,16,145,15,126,174,115,86,128,153,5,164,172,125,148,32,148,229,218,168,178,155,241,107,85,29,9,33,239,51,10,
    133,191,84,31,206,2,212,71,198,106,71,9,157,52,186,78,91,189,128,83,106,70,65,113,156,205,176,167,139,228,198,202,18,29,6,120,62,189,51,115,105,255,196,197,40,52,248,216,16,62,98,67,125,246,220,216,124,
    51,195,47,54,37,132,250,168,86,16,115,13,164,44,33,209,220,144,59,95,22,252,126,255,201,237,22,4,204,161,35,222,239,2,197,29,115,247,140,23,80,159,231,149,92,49,177,28,53,91,106,6,7,201,160,16,51,61,191,
    52,232,79,226,216,175,191,184,139,48,222,47,12,190,47,129,17,38,10,99,143,18,96,218,190,241,185,43,132,243,196,119,90,56,81,92,114,249,90,197,186,205,219,205,55,168,98,39,204,144,120,244,65,186,23,189,
    42,195,113,196,212,15,27,138,101,166,249,239,130,59,183,104,207,77,191,71,82,9,135,238,215,201,193,72,26,72,127,40,20,170,226,26,238,203,131,107,160,182,1,162,243,230,40,138,142,22,242,109,44,190,96,117,
    8,39,144,
    //frame 1 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x0C, 0x83, 0x0C,
    68,243,139,34,138,61,9,220,96,190,150,73,42,14,208,123,74,115,252,187,121,164,54,242,171,6,55,6,192,134,187,212,78,193,226,91,77,130,199,56,145,35,250,43,237,24,243,123,67,168,136,137,83,190,231,65,12,
    140,200,94,145,51,30,160,200,216,66,189,140,252,30,236,20,3,165,140,7,72,5,242,235,167,254,76,69,6,191,192,
    //frame 2 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x14, 0x83, 0x0C,
    29,31,234,127,132,27,232,78,195,154,206,142,21,253,242,245,47,180,125,143,151,82,187,123,62,142,102,146,18,202,158,46,227,229,71,123,125,209,230,30,24,132,77,151,12,131,138,13,25,157,234,150,149,227,33,
    11,16,101,1,246,236,240,32,167,23,224,55,226,115,23,7,56,14,163,60,221,168,163,167,95,201,63,4,166,132,85,235,217,62,40,107,123,135,46,6,195,135,53,155,94,179,48,160,45,145,19,181,151,135,131,57,208,81,
    153,185,149,102,3,72,168,112,
    //frame 3 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x1C, 0x83, 0x0C,
    234,7,102,236,218,192,253,89,175,33,112,211,33,112,174,210,61,138,224,178,179,35,88,4,46,21,76,136,253,239,182,186,158,27,192,108,237,112,90,65,188,34,163,145,91,130,228,170,46,243,230,171,56,190,125,146,
    181,227,83,161,123,109,236,198,254,64,160,246,70,229,192,117,83,90,101,230,164,22,184,41,85,26,36,152,101,48,109,148,101,15,225,51,39,138,227,81,48,117,10,87,23,13,252,47,170,55,250,117,51,184,255,187,
    161,138,125,86,7,18,235,64,41,164,
    //frame 4 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x24, 0x83, 0x0C,
    62,97,150,216,182,232,92,157,88,227,250,34,48,114,250,31,238,243,49,157,209,139,104,8,103,85,5,201,8,191,115,76,14,198,44,0,179,145,152,135,56,191,158,145,181,84,232,89,219,8,117,36,44,143,227,112,98,219,
    142,47,87,169,34,169,174,77,4,175,57,253,48,221,243,166,187,177,48,131,187,223,20,121,71,128,154,230,125,110,129,183,11,22,130,222,206,179,79,197,12,13,214,70,141,4,197,244,34,56,228,216,216,74,5,42,133,
    192,75,190,20,226,133,91,10,140,
    //frame 5 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x2C, 0x83, 0x0C,
    84,0,148,50,32,137,94,243,164,95,59,44,66,122,44,189,24,245,191,34,181,192,220,96,221,212,235,48,243,25,15,38,220,41,166,179,219,245,236,103,83,143,97,12,208,231,146,24,70,25,185,146,13,64,222,18,62,47,
    110,78,16,221,144,154,44,21,232,19,209,177,88,226,182,118,239,145,235,196,11,177,63,6,240,159,201,161,109,39,185,222,190,148,20,53,133,46,110,119,144,138,209,171,86,208,49,28,52,183,183,45,67,225,9,214,
    26,174,132,241,64,
    //frame 6 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x34, 0x83, 0x0C,
    183,156,255,34,251,172,246,60,66,167,100,135,232,145,100,254,101,85,176,56,41,178,228,234,170,169,163,56,122,136,5,25,173,52,155,53,239,136,53,82,18,24,209,138,99,233,199,25,185,35,97,27,167,33,112,122,
    152,160,60,208,221,154,95,94,9,194,120,255,84,154,230,202,76,190,161,130,239,41,20,39,115,18,218,151,224,87,85,233,36,119,213,232,65,197,118,79,
    //frame 7 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x3C, 0x83, 0x0C,
    69,223,213,201,28,230,130,156,62,132,187,100,118,137,216,189,125,192,81,179,242,132,21,9,16,132,67,146,228,188,136,66,50,79,147,194,83,131,64,100,236,234,219,81,200,197,138,112,3,209,233,212,41,191,216,
    140,137,253,171,214,136,77,233,199,234,218,140,251,255,64,10,7,209,59,39,199,238,149,181,219,3,14,87,229,73,40,58,253,21,153,184,208,197,213,108,118,175,148,224,104,197,25,76,229,70,157,24,72,219,200,211,
    62,108,169,165,32,
    //frame 8 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x44, 0x83, 0x0C,
    243,78,246,200,76,68,155,138,138,114,70,85,121,188,229,219,91,191,142,112,8,46,225,32,33,32,171,183,145,240,197,50,203,166,135,236,145,105,230,49,232,127,180,217,8,48,7,22,59,42,225,252,150,138,82,130,
    86,82,215,93,17,158,208,171,79,71,223,84,240,0,144,45,15,241,120,219,166,134,101,98,223,32,
    //frame 9 golden
    0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0xD0, 0x4C, 0x83, 0x0C,
    28,46,252,66,217,223,222,52,190,217,100,127,62,99,253,209,234,203,216,191,207,222,31,230,161,66,8,231,24,115,212,55,199,4,54,40,215,10,124,53,157,61,218,132,33,166,102,185,31,224,50,243,52,105,106,106,
    33,56,244,9,141,239,110,69,170,163,47,152,75,7,128,0

};

#endif /* DRIVER_MHE_KERNEL_UT_NV12_QCIF_FRAMES_QP50_GOLDEN_H_ */
