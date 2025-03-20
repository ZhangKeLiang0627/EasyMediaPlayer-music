#include "lrc.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static long fgetsize(FILE *stream);
static int str_line_len(const char *str);
static const char *lyric_file_get_single(lyric_node *node, const char *lyric_buf_start);
static const char *lyric_file_get_ar_ti(const char *lyric_buf, const char *substr);

#define lyric_file_get_ar(BUF) lyric_file_get_ar_ti(BUF, "[ar:")
#define lyric_file_get_ti(BUF) lyric_file_get_ar_ti(BUF, "[ti:")

// 从lrc文件加载歌词
lyric_t *lyric_load_from_file(const char *lrc_path)
{
    int state = 0;
    FILE *lrc_file = NULL;
    char *lrc_file_buf = NULL;
    lyric_t *lyric = NULL;

    __INIT_START
    // 打开lrc文件
    lrc_file = fopen(lrc_path, "r");
    if (lrc_file == NULL)
        __INIT_ABORT

    state++;
    // 申请一个lyric类型内存空间
    lyric = (lyric_t *)malloc(sizeof(lyric_t));
    if (lyric == NULL)
        __INIT_ABORT
    memset(lyric, 0, sizeof(lyric_t));

    state++;
    // 将lrc文件加载到内存
    int size = fgetsize(lrc_file);
    lrc_file_buf = (char *)malloc(size);
    if (lrc_file_buf == NULL)
        __INIT_ABORT
    fread(lrc_file_buf, sizeof(char), size, lrc_file);
    fclose(lrc_file);

    state++;
    // 解析lrc文件
    lyric->artist = lyric_file_get_ar(lrc_file_buf); // 获取艺人
    lyric->title = lyric_file_get_ti(lrc_file_buf);  // 获取曲名

    // 创建歌词表
    const char *start = strstr(lrc_file_buf, "[00"); // 获取歌词第一句首地址
    lyric_node *p = NULL;
    int id = 0;

    while (start != NULL)
    {
        lyric_node *node = malloc(sizeof(lyric_node));

        // 空指针检测

        memset(node, 0, sizeof(lyric_node));

        if (lyric->head == NULL)
            lyric->head = node;
        else
            p->next = node;

        p = node;

        start = lyric_file_get_single(node, start);
        node->id = id++;

        start = strchr(start, '[');
    }

    return lyric;
    __INIT_END

    __INIT_ERROR_HANDLE
    switch (state)
    {
    case 3:
        free(lrc_file_buf);
    case 2:
        free(lyric);
    case 1:
        fclose(lrc_file);
    default:;
    }

    __INIT_FAILED
}

// 释放歌词
void lyric_free(lyric_t *lyric)
{
    lyric_node *p = lyric->head;
    while (p != NULL)
    {
        lyric_node *pfree = p;
        p = p->next;
        free((void *)pfree->lyricString);
        free((void *)pfree);
    }

    free((void *)lyric->artist);
    free((void *)lyric->title);
}

// 获取全部歌词,使用sep分隔每句歌词
const char *lyric_get_all(lyric_t *lyric, char sep)
{
    if (lyric == NULL || lyric->head == NULL)
        return NULL;

    lyric_node *p = lyric->head;
    char *str = calloc(1, 1);
    char str_sep[2] = {sep, 0};

    int len = 0;

    while (p != NULL)
    {
        len += strlen(p->lyricString) + 2;
        char *pstr = realloc(str, len);
        if (pstr == NULL)
        {
            free(str);
            return NULL;
        }

        str = pstr;
        strcat(str, p->lyricString);
        strcat(str, str_sep);

        p = p->next;
    }

    return str;
}

// 从某句开始获取n歌词,使用sep分隔每句歌词
const char *lyric_getn(lyric_t *lyric, char sep, int start_id, int n)
{
    if (lyric == NULL || lyric->head == NULL)
        return NULL;

    lyric_node *p = lyric->head;
    // 跳过前面的歌词
    for(int i = 0; i < start_id && p != NULL; i++)
        p = p->next;

    if(p == NULL)
        return NULL;

    char *str = calloc(1, 1);
    char str_sep[2] = {sep, 0};

    int len = 0;
    for (int i = 0; i < n && p != NULL; i++)
    {
        len += strlen(p->lyricString) + 2;
        char *pstr = realloc(str, len);
        if (pstr == NULL)
        {
            free(str);
            return NULL;
        }

        str = pstr;
        strcat(str, p->lyricString);
        strcat(str, str_sep);

        p = p->next;
    }

    return str;
}

// 获取某个时间点的歌词，单位ms
const char *lyric_get_by_time(lyric_t *lyric, uint32_t time)
{
    if (lyric == NULL || lyric->head == NULL)
        return NULL;

    lyric_node *p = lyric->head;
    lyric_node *prev_p = NULL;

    int min = 0;
    int max = 0;

    while (p != NULL)
    {
        max = p->time;

        if (min < time && time < max)
        {
            if (prev_p != NULL)
                return prev_p->lyricString;
            else
                return p->lyricString;
        }

        min = p->time;
        prev_p = p;
        p = p->next;
    }
    if (p == NULL && prev_p != NULL)
        return prev_p->lyricString; // 最后一句歌词

    return NULL;
}

// 获取某个时间点的歌词id
int lyric_getid_by_time(lyric_t *lyric, uint32_t time)
{
    if (lyric == NULL || lyric->head == NULL)
        return -1;

    lyric_node *p = lyric->head;
    lyric_node *prev_p = NULL;

    int min = 0;
    int max = 0;

    while (p != NULL)
    {
        max = p->time;

        if (min < time && time < max)
        {
            if (prev_p != NULL)
                return prev_p->id;
            else
                return p->id;
        }

        min = p->time;
        prev_p = p;
        p = p->next;
    }

    if (p == NULL && prev_p != NULL)
        return prev_p->id; // 最后一句歌词

    return -1;
}

/**
 * @brief 获取歌词句子数
 */
int lyric_get_sentence_cnt(lyric_t *lyric)
{
    if (lyric == NULL || lyric->head == NULL)
        return 0;

    int cnt = 0;

    lyric_node *p = lyric->head;

    while (p != NULL)
    {
        ++cnt;
        p = p->next;
    }

    return cnt;
}

// 解析lrc文件获取单句歌词,并返回下一句歌词的首地址
static const char *lyric_file_get_single(lyric_node *node, const char *lyric_buf_start)
{
    // 获取时间戳ms
    const char *buf = lyric_buf_start + 1; // 跳过'['
    int min = atoi(buf);
    buf += 3;
    int sec = atoi(buf);
    buf += 3;
    node->time = min * 60 * 1000 + sec * 1000 + atoi(buf) * 10;
    buf += 3;

    // 获取一行歌词的长度，申请内存空间
    int lyric_string_len = str_line_len(buf);
    char *lyric_string_buf = (char *)malloc(lyric_string_len + 1);
    if (lyric_string_buf == NULL)
        return NULL;

    memcpy(lyric_string_buf, buf, lyric_string_len);
    lyric_string_buf[lyric_string_len] = '\0';

    node->lyricString = lyric_string_buf;

    return buf + lyric_string_len;
}

// 获取一行字符串长度(遇到\r、\n或\0为止)
static int str_line_len(const char *str)
{
    int len = 0;

    for (; *str != '\r' && *str != '\n' && *str != '\0'; str++)
        len++;

    return len;
}

// 解析lrc文件获取艺人、曲名
static const char *lyric_file_get_ar_ti(const char *lyric_buf, const char *substr)
{
    char *start = strstr(lyric_buf, substr); // strstr待优化-------------------------------------
    if (start != NULL)
    {
        start += 4;
        char *end = strchr(start, ']'); ////strchr待优化-----------------------------------

        if (end != NULL)
        {
            int size = end - start;
            char *tmp = malloc(size + 1);
            memcpy(tmp, start, size);
            tmp[size] = '\0';

            return tmp;
        }
    }

    return NULL;
}

// 获取文件大小
static long fgetsize(FILE *stream)
{
    long file_size = -1;
    long cur_offset = ftell(stream); // 获取当前偏移位置
    if (cur_offset == -1)
        return -1;

    if (fseek(stream, 0, SEEK_END) != 0) // 移动文件指针到文件末尾
        return -1;

    file_size = ftell(stream); // 获取此时偏移值，即文件大小

    if (fseek(stream, cur_offset, SEEK_SET) != 0) // 将文件指针恢复初始位置
        return -1;

    return file_size;
}
