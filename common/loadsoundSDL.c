#include "loadsoundSDL.h"

static ALuint LoadSoundSDL(const char *filename)
{
    Sound_Sample *sample;
    ALenum err, format;
    ALuint buffer;
    Uint32 slen;

    /* Open the audio file */
    sample = Sound_NewSampleFromFile(filename, NULL, 65536);
    if(!sample)
    {
        fprintf(stderr, "Could not open audio in %s\n", filename);
        return 0;
    }

    /* Get the sound format, and figure out the OpenAL format */
    if(sample->actual.channels == 1)
    {
        if(sample->actual.format == AUDIO_U8)
            format = AL_FORMAT_MONO8;
        else if(sample->actual.format == AUDIO_S16SYS)
            format = AL_FORMAT_MONO16;
        else
        {
            fprintf(stderr, "Unsupported sample format: 0x%04x\n", sample->actual.format);
            Sound_FreeSample(sample);
            return 0;
        }
    }
    else if(sample->actual.channels == 2)
    {
        if(sample->actual.format == AUDIO_U8)
            format = AL_FORMAT_STEREO8;
        else if(sample->actual.format == AUDIO_S16SYS)
            format = AL_FORMAT_STEREO16;
        else
        {
            fprintf(stderr, "Unsupported sample format: 0x%04x\n", sample->actual.format);
            Sound_FreeSample(sample);
            return 0;
        }
    }
    else
    {
        fprintf(stderr, "Unsupported channel count: %d\n", sample->actual.channels);
        Sound_FreeSample(sample);
        return 0;
    }

    /* Decode the whole audio stream to a buffer. */
    slen = Sound_DecodeAll(sample);
    if(!sample->buffer || slen == 0)
    {
        fprintf(stderr, "Failed to read audio from %s\n", filename);
        Sound_FreeSample(sample);
        return 0;
    }

    /* Buffer the audio data into a new buffer object, then free the data and
     * close the file. */
    buffer = 0;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, sample->buffer, slen, sample->actual.rate);
    Sound_FreeSample(sample);

    /* Check if an error occured, and clean up if so. */
    err = alGetError();
    if(err != AL_NO_ERROR)
    {
        fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
        if(buffer && alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);
        return 0;
    }

    return buffer;
}
