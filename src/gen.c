#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <portaudio.h>

#define COLOR "\x1b[32m"
#define RESET "\x1b[0m"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
#define NUM_SECONDS 5
#define NUM_SAMPLES (NUM_SECONDS * SAMPLE_RATE)
#define PASSWORD_LENGTH 12

typedef struct {
    float buffer[NUM_SAMPLES];
    int index;
} Data;

static int
callback (const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) 
{
  Data *data = (Data*)userData;
  const float *in = (const float*)inputBuffer;
  unsigned long i;

    for (i = 0; i < framesPerBuffer; i++) 
      {
        if (data->index < NUM_SAMPLES)
          {
            data->buffer[data->index++] = *in++;
          }
      }
    return paContinue;
}

void
record (Data *data) 
{
  PaStream *stream;
  PaError err;

  err = Pa_Initialize();
  if (err != paNoError) return;

  err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, FRAMES_PER_BUFFER, callback, data);
  if (err != paNoError) return;

  err = Pa_StartStream(stream);
  if (err != paNoError) return;

  printf(COLOR "\n[+] " RESET "Recording...\n");
  Pa_Sleep(NUM_SECONDS * 1000);

  err = Pa_StopStream(stream);
  if (err != paNoError) return;

  err = Pa_CloseStream(stream);
  if (err != paNoError) return;

  Pa_Terminate();
  printf("-   Recording completed\n");
}

unsigned int
entropy_gen (float *Data, int length)
{
  unsigned int entropy = 0;
  for (int i = 0; i < length; i++) 
    {
      entropy ^= ((unsigned int)(Data[i] * 1000)) << (i % 8);
    }
  return entropy;
}

void
generator (char *password, int length, unsigned int seed)
{
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  srand(seed);
  for (int i = 0; i < length; i++) 
    {
      password[i] = charset[rand() % (sizeof(charset) - 1)];
    }
  password[length] = '\0';
}

int
main (void) 
{
  Data data = { {0}, 0 };
  record(&data);

  unsigned int entropy = entropy_gen(data.buffer, data.index);
  printf("-   Entropy: %u\n", entropy);

  char password[PASSWORD_LENGTH + 1];
  generator(password, PASSWORD_LENGTH, entropy);
  printf(COLOR "\n[+] " RESET "Password: %s\n", password);

  return 0;
}

