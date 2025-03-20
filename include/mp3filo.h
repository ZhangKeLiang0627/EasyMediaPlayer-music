#ifndef _MP3FILO_H_
#define _MP3FILO_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        unsigned int offset; // 帧头偏移量
        float timelapse;     // 帧时长
    } mp3_header_pos_t;

    void mp3filo_alloc(void);
    void mp3filo_push(mp3_header_pos_t *hpos);
    void mp3filo_read(int index, mp3_header_pos_t *hpos);
    void mp3filo_free(void);

#ifdef __cplusplus
}
#endif

#endif
