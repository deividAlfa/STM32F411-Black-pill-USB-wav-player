/*
 * pwmAudio.c
 *
 *  Created on: Mar 21, 2021
 *      Author: David
 */
#include "pwmAudio.h"
#include "fatfs.h"
#include "files.h"

#define PCMsize	512
uint16_t PCMbuffer[PCMsize];
system_t systemStatus;
extern FIL USBHFile;       /* File object for USBH */
extern TIM_HandleTypeDef htim1;




void handleAudio(void){
	if(systemStatus.audioStatus != audio_play && systemStatus.driveStatus == drive_ready && systemStatus.fileStatus == file_opened){
		AudioStart();
	}
}


void AudioStart(void){
	systemStatus.remainingSamples = fillBuffer(&PCMbuffer[0], PCMsize);	// Fill the entire buffer with data

	if(systemStatus.remainingSamples == 0 ){							// If zero bytes transferred
		systemStatus.audioStatus = audio_stop;							// Play finished
	}
	else{
		if(systemStatus.remainingSamples < PCMsize){					// If less than 2048 bytes transferred
			systemStatus.fileStatus = file_end;							// Reached the end
			setBuffer(&PCMbuffer[systemStatus.remainingSamples],\
					PCMsize-systemStatus.remainingSamples,0x80);		// Fill the remaining data with silence
		}

		__HAL_DBGMCU_FREEZE_TIM1();										// Enable timer freeze when halting the core when debugging

		if(systemStatus.audioRate == audio_8KHz){						// 8KHz sample rate
			TIM1->RCR=47;												// Adjust repetition counter
		}
		else if(systemStatus.audioRate == audio_16KHz){					// 16KHz sample rate
			TIM1->RCR=23;												// Adjust repetition counter
		}
		else if(systemStatus.audioRate == audio_22KHz){					// 22KHz sample rate
			TIM1->RCR=15;
		}
		else if(systemStatus.audioRate == audio_32KHz){					// 32KHz sample rate
			TIM1->RCR=11;
		}
		else if(systemStatus.audioRate == audio_96KHz){					// 96KHz sample rate
			TIM1->RCR=3;
		}
		else{															// 48KHz / 44.1KHz sample rate
			TIM1->RCR=7;
		}
																		// 22.05 / 44.1KHz sample rate
		if(systemStatus.audioRate == audio_44KHz || systemStatus.audioRate == audio_22KHz){
			TIM1->ARR=271;												// Adjust period counter (22059Hz, 44117Hz, 0.04% error)
		}
		else{															// 16/32/48KHz sample rate
			TIM1->ARR=249;												// Adjust period counter
		}
																		// Start DMA
		HAL_TIM_DMABurst_MultiWriteStart(&htim1, TIM_DMABASE_CCR1, TIM_DMA_UPDATE, (uint32_t*)&PCMbuffer[0], TIM_DMABURSTLENGTH_2TRANSFERS, PCMsize);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);						// Start PWM
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		systemStatus.audioStatus=audio_play;							// Playing
	}
	printf("Playback started\n");
}


void AudioStop(void){

	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);							// Stop PWM
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	HAL_TIM_DMABurst_WriteStop(&htim1, TIM_DMA_UPDATE);					// Stop DMA

	systemStatus.audioStatus = audio_stop;								// Play finished
	printf("Playback stopped\n\n");
}


void setBuffer(uint16_t* dest, uint16_t data, uint16_t count){
	for(uint16_t x=0; x<count; x++){
		*dest++ = data;
	}
}


uint16_t fillBuffer(uint16_t* dest, uint16_t sampleCount){
	uint16_t byteCount;
	UINT doneCount;
	union{																		// Temporal buffer
		int16_t data16[PCMsize];												// For storing 16 bit samples
		uint8_t data8[PCMsize];													// For storing 8 bit samples
	}data;

	if(systemStatus.audioChannels == audio_mono && systemStatus.audioBits == audio_8bit){
		byteCount = sampleCount /2;														// In mono 8 bit, read half bytes than samples
	}
	else if(systemStatus.audioChannels == audio_stereo && systemStatus.audioBits == audio_16bit){
		byteCount = sampleCount *2;														// In stereo 16bit, read double bytes than samples
	}
	else{
		byteCount = sampleCount;														// Mono 16 bit / stereo 8 bit, read same bytes as samples
	}

	if( f_read(&USBHFile, &data.data8[0], byteCount, &doneCount) != FR_OK){				// Fill temp buffer
		printf("Error reading file\n");
		return 0;
	}
	if(doneCount == 0){ return 0; }

	if(systemStatus.audioChannels == audio_stereo){						// Stereo mode
		if(systemStatus.audioBits == audio_8bit){						// 8 bit wav
			for(uint16_t x=0; x<doneCount; x++){							// Transfer to dest, converting 8 to 16bit
				*dest++ = data.data8[x];								// Copy data to channel
			}
		}
		else{															// 16 bit wav
			doneCount /= 2;												// We read 2 bytes/sample, we now use 16 bit pointer, so divide  by 2
			for(uint16_t x=0; x<doneCount; x++){						// Transfer to dest, converting signed 16bit to unsigned 8bit
				*dest++ = (uint16_t)((int32_t)data.data16[x]+32768)>>8; // Copy data to channel
			}
		}
	}

	else{																// Mono mode
		if(systemStatus.audioBits == audio_8bit){						// 8 bit wav
			for(uint16_t x=0; x<doneCount; x++){						// Transfer to dest, converting 8 to 16bit
				*dest++ = data.data8[x];								// Copy data to left channel
				*dest++ = data.data8[x];								// Duplicate for right channel
			}
			doneCount *= 2;												// Double effective read samples (mono->stereo)
		}
		else{															// 16 bit wav
			for(uint16_t x=0; x<doneCount/2; x++){						// Transfer to dest, converting signed 16bit to unsigned 8bit
				*dest++ = (uint16_t)((int32_t)data.data16[x]+32768)>>8;	// Copy data to left channel
				*dest++ = (uint16_t)((int32_t)data.data16[x]+32768)>>8;	// Duplicate for right channel
			}
		}
	}

	return doneCount;													// Return read samples (don't confuse with read bytes)
}


void handleBuffer(uint16_t offset){
	if(systemStatus.fileStatus==file_opened){							// File ok?
		uint16_t count = fillBuffer(&PCMbuffer[offset], PCMsize/2);		// Transfer 1/2 buffer data
		if(count<PCMsize/2){											// If less data than expected
			systemStatus.fileStatus=file_end;							// File end reached
			f_close(&USBHFile);											// Close file
			systemStatus.remainingSamples -= PCMsize/2;					// Subtract 1/2 buffer count
			systemStatus.remainingSamples += count;						// Add the remaining bytes
			setBuffer(&PCMbuffer[offset+count], 0x80, (PCMsize/2)-count);	// Fill the remaining data with silence
		}
	}
	else{																// File already reached end, so no more data to transfer
		if(systemStatus.remainingSamples < PCMsize/2){					// Remaining bytes less than 1/2 buffer?
			AudioStop();												// Done, stop audio
		}
		else{
			systemStatus.remainingSamples -= PCMsize/2;					// Buffer not done yet
		}
	}
}


void HAL_TIM_PeriodElapsedHalfCpltCallback(TIM_HandleTypeDef *htim){
	if(htim==&htim1){
		handleBuffer(0);			// Refill first half
	}
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim==&htim1){
		handleBuffer(PCMsize/2);	// Refill last half
	}
}
