#include "mp3filo.h"
#include <vector>
#include <string.h>

using namespace std;

static vector<mp3_header_pos_t> *mp3_headpos = nullptr;

static void _mp3filo_alloc(void);
static void _mp3filo_push(mp3_header_pos_t *hpos);
static void _mp3filo_read(int index, mp3_header_pos_t *hpos);
static void _mp3filo_free(void);

void mp3filo_alloc(void)
{
    _mp3filo_alloc();
}

void mp3filo_push(mp3_header_pos_t *hpos)
{
    _mp3filo_push(hpos);
}

void mp3filo_read(int index, mp3_header_pos_t *hpos)
{
    _mp3filo_read(index, hpos);
}

void mp3filo_free(void)
{
    _mp3filo_free();
}

static void _mp3filo_alloc(void)
{
    mp3_headpos = new vector<mp3_header_pos_t>;
}

static void _mp3filo_push(mp3_header_pos_t *hpos)
{
    if (mp3_headpos != nullptr && hpos != NULL)
        mp3_headpos->push_back(*hpos);
}

static void _mp3filo_read(int index, mp3_header_pos_t *hpos)
{
    if (mp3_headpos != nullptr && hpos != NULL)
    {
        if (mp3_headpos->size() > index)
            memcpy(hpos, &mp3_headpos[index], sizeof(mp3_header_pos_t));
    }
}

static void _mp3filo_free(void)
{
    delete mp3_headpos;
}
