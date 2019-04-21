#ifndef CTMF_H
#define CTMF_H


#define CTMF_MAX 255
#define CTMF_BITS 8
#define CTMF_BITS2 4
#define CTMF_BINS 16
#define CTMF_CMASK 0xF0
#define CTMF_FMASK 0x0F

//#define CTMF_MAX 1023
//#define CTMF_BITS 10
//#define CTMF_BITS2 5
//#define CTMF_BINS 32


#ifdef __cplusplus
extern "C" {
#endif

void ctmf(
    const unsigned char* src, float* dst,
    int width, int height,
    int src_step_row, int dst_step_row,
    int r, int channels, const unsigned char threshold,
    unsigned long memsize
);

#ifdef __cplusplus
}
#endif

#endif
