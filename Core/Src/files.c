/*
 * files.c
 *
 *  Created on: Dec 26, 2020
 *      Author: David
 */


#include "files.h"
#include "fatfs.h"
#include "pwmAudio.h"
extern FATFS USBHFatFS;    /* File system object for USBH logical drive */
extern FIL USBHFile;       /* File object for USBH */
DIR dir;         /* Directory object */
FILINFO fil;    /* File information */
wav_header wavHeader;
extern system_t systemStatus;


uint8_t checkWav(void){
	uint8_t fail = 0;
	char WavInfo[36] = "WAV Info: ";

	if(wavHeader.ChunkID != 0x46464952 ){		// "RIFF"
		fail=1;
	}
	if(wavHeader.Format != 0x45564157 ){		// "WAVE"
		fail=1;
	}
	if(wavHeader.Subchunk1ID != 0x20746d66 ){	// "fmt "
		fail=1;
	}
	if(wavHeader.Subchunk1Size != 16 ){			// Should be 16
		fail=1;
	}
	if(wavHeader.AudioFormat != 1 ){			// Should be 1
		fail=1;
	}

	if(wavHeader.NumChannels == 1){
		systemStatus.audioChannels=audio_mono;
		strcat(WavInfo, "Mono, ");
	}
	else if(wavHeader.NumChannels == 2 ){
		systemStatus.audioChannels=audio_stereo;
		strcat(WavInfo, "Stereo, ");
	}
	else{ fail=1; }

	if(wavHeader.SampleRate == 8000){
		systemStatus.audioRate = audio_8KHz;
		strcat(WavInfo, "8KHz, ");

	}
	if(wavHeader.SampleRate == 16000){
		systemStatus.audioRate = audio_16KHz;
		strcat(WavInfo, "16KHz, ");
	}
	else if(wavHeader.SampleRate == 32000){
		systemStatus.audioRate = audio_32KHz;
		strcat(WavInfo, "32KHz, ");
	}
	else if(wavHeader.SampleRate == 22050){
		systemStatus.audioRate = audio_22KHz;
		strcat(WavInfo, "22.05KHz, ");
	}
	else if(wavHeader.SampleRate == 44100){
		systemStatus.audioRate = audio_44KHz;
		strcat(WavInfo, "44.1KHz, ");
	}
	else if(wavHeader.SampleRate == 48000){
		systemStatus.audioRate = audio_48KHz;
		strcat(WavInfo, "48KHz, ");
	}
	else if(wavHeader.SampleRate == 96000){
		systemStatus.audioRate = audio_96KHz;
		strcat(WavInfo, "96KHz, ");
	}
	else{ fail=1; }

	if(wavHeader.BitsPerSample == 16 ){
		systemStatus.audioBits = audio_16bit;
		strcat(WavInfo, "16bit\n");
	}
	else if(wavHeader.BitsPerSample == 8 ){
		systemStatus.audioBits = audio_8bit;
		strcat(WavInfo, "8bit\n");
	}
	else{ fail=1; }

	if(wavHeader.Subchunk2ID != 0x61746164 ){	// "data"
		fail=1;
	}
	if(!fail){ printf(WavInfo); }

	return fail;
}

void handleFS(void){
	FRESULT res = FR_OK;
	if(systemStatus.driveStatus==drive_inserted){						// Drive present
		if( f_mount(&USBHFatFS, "", 1) != FR_OK ){
			  printf("Failed to mount volume\n");
			  systemStatus.driveStatus=drive_error;						//Failure on mount
		}
		else{
			if( f_opendir(&dir, "/") != FR_OK ){
				 printf("Failed to open root dir\n");
				 systemStatus.driveStatus=drive_error;					//Failure opening "/"
			}
			else{
				printf("Drive mounted\n");
				systemStatus.driveStatus=drive_ready;
			}
		}
	}
	else if(systemStatus.driveStatus==drive_ready && systemStatus.fileStatus != file_opened	&& systemStatus.audioStatus != audio_play){
		if(systemStatus.fileStatus == file_none){
			res = f_findfirst(&dir, &fil, "/", "*.wav");				// Find first file
			systemStatus.fileStatus = file_end;
		}
		else if(systemStatus.fileStatus == file_end){
			f_close(&USBHFile);
			res = f_findnext(&dir, &fil);								// Find next file
		}
		if(res == FR_OK){												// If OK
			UINT count;
			if(fil.fname[0] == 0){										// No file found or last file
				systemStatus.fileStatus = file_none;					// Start again
				printf("No more files, resetting loop\n");
				return;
			}
			printf("Opening file: %s\n",fil.fname);
			if( f_open(&USBHFile, fil.fname, FA_READ) != FR_OK){				// Open file
				printf("Error opening file\n");
				return;
			}
			if( f_read(&USBHFile, &wavHeader, 44, &count) != FR_OK){			// Read wav header
				printf("Error reading file\n");
				return;
			}
			if(count<44 || checkWav()){									// Check for valid WAV file
				printf("Invalid WAV file\n");
				return;
			}
			systemStatus.fileStatus = file_opened;						// Valid file
		}
	}

	if((systemStatus.driveStatus==drive_error) || (systemStatus.driveStatus==drive_removed)){	//if drive removed or error
		f_mount(0, "", 1);															// remove mount point
		if(systemStatus.driveStatus==drive_removed){
			systemStatus.driveStatus=drive_nodrive;
		}
		else{
			systemStatus.driveStatus=drive_unmounted;
		}
	}
}


