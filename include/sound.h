#ifndef _SOUND_H_
#define _SOUND_H_

#include <alsa/asoundlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    int snd_pcm_init(void);
    void snd_pcm_set_format(snd_pcm_format_t format);
    void snd_pcm_set_rate(unsigned int rate);
    void snd_pcm_set_channels(unsigned int channels);
    void snd_pcm_set_period_size(unsigned int size);
    unsigned int snd_pcm_get_period_size(void);
    snd_pcm_t *snd_pcm_get(void);
    void snd_pcm_dev_close(void);

    int snd_mixer_init(void);
    void snd_mixer_dev_close(void);
    void snd_get_volume(int *cur, int *max);
    void snd_set_volume(int v);

#ifdef __cplusplus
}
#endif

#endif
