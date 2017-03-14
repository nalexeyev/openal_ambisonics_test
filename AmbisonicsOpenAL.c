#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "alhelpers.h"
//#include "openal-info.h"

#define PRINT_WHEADER printf("(1-4): %s \n", header.riff); \
	printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]); \
	printf("(5-8) Overall size: bytes:%u, Kb:%u \n", header.overall_size, header.overall_size / 1024); \
	printf("(9-12) Wave marker: %s\n", header.wave); \
	printf("(13-16) Fmt marker: %s\n", header.fmt_chunk_marker); \
	printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]); \
	printf("(17-20) Length of Fmt header: %u \n", header.length_of_fmt); \
	printf("%u %u \n", buffer2[0], buffer2[1]); \
	printf("(21-22) Format type: %u %s \n", header.format_type, format_name); \
	printf("%u %u \n", buffer2[0], buffer2[1]); \
	printf("(23-24) Channels: %u \n", header.channels); \
	printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]); \
	printf("(25-28) Sample rate: %u\n", header.sample_rate); \
	printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]); \
	printf("(29-32) Byte Rate: %u , Bit Rate:%u\n", header.byterate, header.byterate * 8); \
	printf("%u %u \n", buffer2[0], buffer2[1]); \
	printf("(33-34) Block Alignment: %u \n", header.block_align); \
	printf("%u %u \n", buffer2[0], buffer2[1]); \
	printf("(35-36) Bits per sample: %u \n", header.bits_per_sample); \
	printf("(37-40) Data Marker: %s \n", header.data_chunk_header); \
	printf("%u %u %u %u\n", buffer4[0], buffer4[1], buffer4[2], buffer4[3]); \
	printf("(41-44) Size of data chunk: %u \n", header.data_size); \
	printf("Number of samples:%lu \n", num_samples); \
	printf("Size of each sample:%ld bytes\n", size_of_each_sample); \
	printf("Approx.Duration in seconds=%f\n", duration_in_seconds);

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#define PAUSE_NSEC 10000000

unsigned char* raw_data = NULL;
size_t size;

unsigned long ticks_max = 0;

struct WHEADER {
	unsigned char riff[4];				// RIFF string
	unsigned int overall_size;			// overall size of file in bytes
	unsigned char wave[4];				// WAVE string
	unsigned char fmt_chunk_marker[4];	// fmt string with trailing null char
	unsigned int length_of_fmt;			// length of the format data
	unsigned int format_type;			// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	unsigned int channels;				// no.of channels
	unsigned int sample_rate;			// sampling rate (blocks per second)
	unsigned int byterate;				// SampleRate * NumChannels * BitsPerSample/8
	unsigned int block_align;			// NumChannels * BitsPerSample/8
	unsigned int bits_per_sample;		// bits per sample, 8- 8bits, 16- 16 bits etc
	unsigned char data_chunk_header[4];	// DATA string or FLLR string
	unsigned int data_size; 			// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
};

ALuint LoadSound(const char *filename) {
	ALuint al_buffer = 0;
	int read;
	char format_name[8] = { 0 };
	unsigned char buffer4[4];
	unsigned char buffer2[2];
	struct WHEADER header;
	long num_samples;
	long size_of_each_sample;
	float duration_in_seconds;
	ALenum format;
	FILE* ptr;

	ptr = fopen(filename, "rb");
	if (ptr == NULL) {
		printf("Error opening file\n");
		return al_buffer;
	}

	// read header parts
	read = fread(header.riff, sizeof(header.riff), 1, ptr);

	read = fread(buffer4, sizeof(buffer4), 1, ptr);

	// convert little endian to big endian 4 byte int
	header.overall_size = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	read = fread(header.wave, sizeof(header.wave), 1, ptr);

	read = fread(header.fmt_chunk_marker, sizeof(header.fmt_chunk_marker), 1, ptr);

	read = fread(buffer4, sizeof(buffer4), 1, ptr);

	// convert little endian to big endian 4 byte integer
	header.length_of_fmt = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	read = fread(buffer2, sizeof(buffer2), 1, ptr);

	header.format_type = buffer2[0] | (buffer2[1] << 8);
	if (header.format_type == 1)
		strcpy(format_name, "PCM");
	else if (header.format_type == 6)
		strcpy(format_name, "A-law");
	else if (header.format_type == 7)
		strcpy(format_name, "Mu-law");
	else if (header.format_type == 0xFFFE)
		strcpy(format_name, "WAVE-EX");

	read = fread(buffer2, sizeof(buffer2), 1, ptr);
	header.channels = buffer2[0] | (buffer2[1] << 8);

	read = fread(buffer4, sizeof(buffer4), 1, ptr);
	header.sample_rate = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	read = fread(buffer4, sizeof(buffer4), 1, ptr);
	header.byterate = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	read = fread(buffer2, sizeof(buffer2), 1, ptr);
	header.block_align = buffer2[0] | (buffer2[1] << 8);

	read = fread(buffer2, sizeof(buffer2), 1, ptr);
	header.bits_per_sample = buffer2[0] | (buffer2[1] << 8);

	// limitation to 4 channel 16bit PCM
	if (header.format_type != 1 || header.channels != 4 || header.bits_per_sample != 16) {
		printf("Unsupported format");
		return EXIT_FAILURE;
	}

	read = fread(header.data_chunk_header, sizeof(header.data_chunk_header), 1, ptr);

	read = fread(buffer4, sizeof(buffer4), 1, ptr);
	header.data_size = buffer4[0] | (buffer4[1] << 8) | (buffer4[2] << 16) | (buffer4[3] << 24);

	// calculate no.of samples
	num_samples = (8 * header.data_size) / (header.channels * header.bits_per_sample);

	size_of_each_sample = (header.channels * header.bits_per_sample) / 8;

	// calculate duration of file
	duration_in_seconds = (float) header.overall_size / header.byterate;
	ticks_max = duration_in_seconds * 100;

	// read samples from data chunk
	size = size_of_each_sample * num_samples;
	raw_data = malloc(size);
	read = fread(raw_data, size, 1, ptr);

	format = AL_FORMAT_BFORMAT3D_16;

	alGenBuffers(1, &al_buffer);
	alBufferData(al_buffer, format, raw_data, size, header.sample_rate);

	free(raw_data);

	assert(alGetError()==AL_NO_ERROR && "Failed to load buffer");

	printf("Closing file..\n");
	fclose(ptr);
	//PRINT_WHEADER
	return al_buffer;
}

int main() {
	ALCdevice *device;
	ALuint source = 0, buffer;
	ALenum state;

	//const char *soundname = "resources/guitar-[48000_4ch_16bitPCM_FuMa_FuMa(WXYZ)].wav";
	//const char *soundname = "resources/r2d2-[48000_4ch_16bitPCM_SN3D_ACN(WX_of_WYZX)].wav";
	const char *soundname = "resources/r2d2-[48000_4ch_16bitPCM_FuMa_FuMa(WX_of_WXYZ)].wav";

	//showInfo(0, NULL);

	if(InitAL(NULL, NULL) != 0)
		return EXIT_FAILURE;

	device = alcGetContextsDevice(alcGetCurrentContext());

	/* Check B-FORMAT support in OpenAL. */
	if (!alIsExtensionPresent("AL_EXT_BFORMAT")) {
		fprintf(stderr, "Error: AL_EXT_BFORMAT not supported\n");
		CloseAL();
		return EXIT_FAILURE;
	}

	/* Load the sound into a buffer. */
	buffer = LoadSound(soundname);
	if (!buffer) {
		CloseAL();
		return EXIT_FAILURE;
	}

	/* Create the source to play the sound with. */
	alGenSources(1, &source);
	alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
	alSourcei(source, AL_BUFFER, buffer);
	assert(alGetError()==AL_NO_ERROR && "Failed to setup sound source");

	/* Setup initial listener's orientation */
	ALfloat listenerOri[6];
	/* "AT" vector */
	listenerOri[0] = (ALfloat)1.0; // Default x = 0.0
	listenerOri[1] = (ALfloat)0.0; // Default y = 0.0
	listenerOri[2] = (ALfloat)0.0; // Default z = -1.0
	/* "UP" vector */
	listenerOri[3] = (ALfloat)0.0; // Default x = 0.0
	listenerOri[4] = (ALfloat)1.0; // Default y = 1.0
	listenerOri[5] = (ALfloat)0.0; // Default z = 0.0

	alSourcefv(source, AL_ORIENTATION, listenerOri);

	/* Play the sound until it finishes. */

	int ticks = 0;

	alSourcePlay(source);
	do {
		al_nssleep(PAUSE_NSEC);
		//listenerOri[0] = (ALfloat)-1.0 + 2.0/ticks_max*ticks;
		//listenerOri[0] = 1.0;
		alSourcefv(source, AL_ORIENTATION, listenerOri);
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		//ticks++;
	} while(state == AL_PLAYING);

	/* All done. Delete resources, and close OpenAL. */
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);

	CloseAL();

	return EXIT_SUCCESS;
}
