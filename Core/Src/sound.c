#include "main.h"
#include "sound.h"

#include <stdint.h>
#include <math.h>

/* Configuration */
#define SOUND_BUFFER_SIZE 512
#define SOUND_SAMPLE_COUNT (SOUND_BUFFER_SIZE / 4)

/* I2S DMA buffer: 2 halfwords per audio sample */
static uint16_t soundBuffer[SOUND_BUFFER_SIZE];

/* Audio level */
static volatile uint8_t soundLevel = 0;

/* DMA processing flags */
static volatile uint8_t halfReady = 0;
static volatile uint8_t fullReady = 0;


/* Calculate RMS and convert it to percentage */
static uint8_t Sound_CalculateLevel(uint16_t *buffer)
{
    double sum = 0.0;

    for (uint16_t i = 0; i < SOUND_BUFFER_SIZE / 4; i++)
    {
        uint16_t index = i * 2;

        /* Combine two 16-bit words */
        int32_t sample =
            ((int32_t)buffer[index] << 16) |
            buffer[index + 1];

        /* Extract signed 24-bit audio sample */
        sample >>= 8;

        sum += (double)sample * sample;
    }

    double rms = sqrt(sum / SOUND_SAMPLE_COUNT);

    double level = (rms / 100000 ) * 100.0;
//
//    if (level > 100.0)
//        level = 100.0;

    return (uint8_t)level;
}


/* Start microphone */
void Sound_Init(void)
{
    soundLevel = 0;
    halfReady = 0;
    fullReady = 0;

    HAL_I2S_Receive_DMA(
        &hi2s3,
        soundBuffer,
        256
    );
}


/* DMA half transfer callback */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI3)
    {
        halfReady = 1;
    }
}


/* DMA complete callback */
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s->Instance == SPI3)
    {
        fullReady = 1;
    }
}


/* Process received audio */
void Sound_Process(void)
{
    if (halfReady)
    {
        halfReady = 0;

        soundLevel = Sound_CalculateLevel(soundBuffer);
    }

    if (fullReady)
    {
        fullReady = 0;

        soundLevel = Sound_CalculateLevel(
            &soundBuffer[SOUND_BUFFER_SIZE / 2]
        );
    }
}


/* Return sound level */
uint8_t Sound_GetLevel(void)
{
    return soundLevel;
}


