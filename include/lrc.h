#ifndef _LRC_H_
#define _LRC_H_

#define __INIT_START \
    do               \
    {
#define __INIT_END \
    }              \
    while (0)      \
        ;
#define __INIT_ABORT break;
#define __INIT_FAILED return NULL;
#define __INIT_ERROR_HANDLE

typedef unsigned int uint32_t;

// 单句歌词节点
typedef struct _lyric_node
{
    uint32_t time;
    const char *lyricString;  // 歌词字符串
    int id;                   // 歌词id
    struct _lyric_node *next; // 下一句歌词
} lyric_node;

typedef struct _lyric
{
    const char *title;  // 歌曲名称
    const char *artist; // 艺人名
    lyric_node *head;   // 歌词表头
} lyric_t;

#ifdef __cplusplus
extern "C"
{
#endif

    // 从lrc文件加载歌词
    lyric_t *lyric_load_from_file(const char *lrc_path);

    // 获取全部歌词,使用sep分隔每句歌词
    const char *lyric_get_all(lyric_t *lyric, char sep);

    const char *lyric_getn(lyric_t *lyric, char sep, int start_id, int n);

    // 释放歌词
    void lyric_free(lyric_t *lyric);

    // 获取某个时间点的歌词，单位ms
    const char *lyric_get_by_time(lyric_t *lyric, uint32_t time);

    // 获取某个时间点的歌词id
    int lyric_getid_by_time(lyric_t *lyric, uint32_t time);

    int lyric_get_sentence_cnt(lyric_t *lyric);

#ifdef __cplusplus
}
#endif

#endif
