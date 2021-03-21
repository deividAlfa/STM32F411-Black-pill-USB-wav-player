/*
 * files.h
 *
 *  Created on: Dec 26, 2020
 *      Author: David
 */

#ifndef INC_FILES_H_
#define INC_FILES_H_
#include "main.h"

typedef enum{
	drive_nodrive = 0,					// No drive on system
	drive_inserted,						// Drive was inserted
	drive_mounted,						// Drive was successfully mounted
	drive_ready,						// Drive is mounted and has files
	drive_nofiles,						// Drive mounted but no files
	drive_removed,						// Drive removed while mounted
	drive_error,						// Drive error
	drive_unmounted,					// Drive unmounted
}driveStatus;

typedef enum{
	file_none = 0,						// No file opened
	file_opened,						// File opened
	file_end							// File reached end
}fileStatus;

typedef enum{
	audio_idle = 0,						// Audio idle after boot
	audio_play,							// Audio is playing
	audio_stop,							// Audio was stopped after playing

	audio_mono,
	audio_stereo,

	audio_8bit,							// Standard = 8 bit unsigned
	audio_16bit,						// Standard = 16 bit signed

	audio_8KHz,
	audio_16KHz,
	audio_22KHz,
	audio_32KHz,
	audio_44KHz,
	audio_48KHz,
	audio_96KHz

}audioStatus;

typedef struct wav_header {
    // RIFF Header
	uint32_t ChunkID; 				// Contains "RIFF"
    uint32_t ChunkSize; 			// Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    uint32_t Format; 				// Contains "WAVE"

    // Format Header
    uint32_t Subchunk1ID; 			// Contains "fmt " (includes trailing space)
    uint32_t Subchunk1Size; 		// Should be 16 for PCM
    uint16_t AudioFormat; 			// Should be 1 for PCM. 3 for IEEE Float
    uint16_t NumChannels ;
    uint32_t SampleRate;
    uint32_t ByteRate; 				// Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    uint16_t BlockAlign; 			// num_channels * Bytes Per Sample
    uint16_t BitsPerSample; 		// Number of bits per sample

    // Data
    uint32_t Subchunk2ID; 			// Contains "data"
    uint32_t Subchunk2Size ; 		// Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; 			// Remainder of wave file is bytes
} wav_header;

void handleFS(void);
uint8_t checkWav(void);

#endif /* INC_FILES_H_ */
