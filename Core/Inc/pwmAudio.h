/*
 * pwmAudio.h
 *
 *  Created on: Mar 21, 2021
 *      Author: David
 */

#ifndef INC_PWMAUDIO_H_
#define INC_PWMAUDIO_H_
#include "main.h"

typedef struct{
	int8_t	 	driveStatus; 		// 0 = no drive, 1=mounted, 2=scanned
	uint8_t		fileStatus; 		// 0 = No file, 1 = File opened, 2 = File end reached
	uint16_t	remainingSamples;	// Remaining samples in the PCM buffer
	uint8_t		audioBits;			// 8Bits, 16Bits
	uint8_t		audioChannels;		// Mono, Stereo
	uint8_t		audioRate;			// 16KHz, 32KHz, 48KHz
	uint8_t		audioStatus;		// Idle, Playing, Stopped(after playing)
}system_t;


void handleAudio(void);
void InitAudio(void);
void AudioStop(void);
void AudioStart(void);
void setBuffer(uint16_t* dest, uint16_t data, uint16_t count);
uint16_t fillBuffer(uint16_t* dest, uint16_t sampleCount);


#endif /* INC_PWMAUDIO_H_ */
