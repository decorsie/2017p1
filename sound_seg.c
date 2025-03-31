#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


struct sound_seg {
    int16_t* data;    // Pointer to the audio samples
    size_t length;    // Number of samples in the track
};

// Load a WAV file into buffer
void wav_load(const char* fname, int16_t* dest) {
    // Load file to read and calculate size to determine number of bytes to load
    FILE* file = fopen(fname, "rb");
    fseek(file, 0, SEEK_END);
    size_t bytes_to_read = ftell(file) - 44;
    
    // Set file pointer back to just past header, then read the raw audio data
    fseek(file, 44, SEEK_SET);
    fread(dest, 1, bytes_to_read, file);
    fclose(file);   
}

// Create/write a WAV file from buffer
void wav_save(const char* fname, const int16_t* src, size_t len) {
    // Load file to write binary, then calculate data/file size
    // File size ignores first 8 bytes of header per RIFF specification
    FILE* file = fopen(fname, "wb");
    uint32_t data_size = len * sizeof(int16_t);
    uint32_t file_size = 36 + data_size;
    
    // Creating WAV file header
    uint8_t header[44] = {
        'R', 'I', 'F', 'F',        // Indicating start of the master RIFF chunk
        (uint8_t)(file_size & 0xff),
        (uint8_t)((file_size >> 8) & 0xff),
        (uint8_t)((file_size >> 16) & 0xff),
        (uint8_t)((file_size >> 24) & 0xff),
        'W', 'A', 'V', 'E',

        'f', 'm', 't', ' ',         // Indicating start of the format chunk
        16, 0, 0, 0,                // Chunk size: 16 for PCM
        1, 0,                       // Audio format: 1 for PCM
        1, 0,                       // Number of channels: 1 for mono
        0x40, 0x1F, 0, 0,           // Samples per second: 8000 (hexadecimal) as specified
        0x80, 0x3E, 0, 0,           // Byte rate: 16000 (hexadecimal)
        2, 0,                       // Data block size: 2
        16, 0,                      // Bits per sample: 16 as specified

        'd', 'a', 't', 'a',         // Indicating start of the sampled data chunk
        (uint8_t)(data_size & 0xff),
        (uint8_t)((data_size >> 8) & 0xff),
        (uint8_t)((data_size >> 16) & 0xff),
        (uint8_t)((data_size >> 24) & 0xff)
    };
    
    // Writing header and audio data to given file
    fwrite(header, 1, sizeof(header), file);
    fwrite(src, sizeof(int16_t), len, file);
    fclose(file);
}

// Initialize a new sound_seg object
struct sound_seg* tr_init() {
    // Allocate memory for the sound_seg structure, then check success
    struct sound_seg* track = (struct sound_seg*)malloc(sizeof(struct sound_seg));
    if (track == NULL) {
        return NULL;
    }
    
    // Initialize the track with default values
    track->data = NULL;    // No initial data
    track->length = 0;     // Empty track, length of zero
    return track;
}

// Destroy a sound_seg object and free all allocated memory
void tr_destroy(struct sound_seg* track) {
    // Check if track is NULL
    if (track == NULL) {
        return;
    }
    
    // Free the data array if it exists
    if (track->data != NULL) {
        free(track->data);
        track->data = NULL;
    }
    
    // Free the track structure itself
    free(track);
}

// Return the length of the segment
size_t tr_length(struct sound_seg* seg) {
    // Check if track is NULL
    if (seg == NULL) {
        return 0; // Return 0 for a NULL track
    }
    
    // Return the number of samples in the track
    return seg->length;
}

// Read len elements from position pos into dest
void tr_read(struct sound_seg* track, int16_t* dest, size_t pos, size_t len) {
    return;
}

// Write len elements from src into position pos
void tr_write(struct sound_seg* track, int16_t* src, size_t pos, size_t len) {
    return;
}

// Delete a range of elements from the track
bool tr_delete_range(struct sound_seg* track, size_t pos, size_t len) {
    return true;
}

// Returns a string containing <start>,<end> ad pairs in target
char* tr_identify(struct sound_seg* target, struct sound_seg* ad){
    return NULL;
}

//test

// Insert a portion of src_track into dest_track at position destpos
void tr_insert(struct sound_seg* src_track,
            struct sound_seg* dest_track,
            size_t destpos, size_t srcpos, size_t len) {
    return;
}

int main(){
    //hello 
    // yay
    //now testing for commit 3, this time for sure right
}