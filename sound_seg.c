#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

struct sound_seg {
    int16_t* buf;              // Buffer containing all underlying data of the track
    size_t buf_capacity;       // Total capacity of the buffer
    struct section* head;      // Points to the head of the track section linked list
};

// 
struct section {
    size_t start;              // Starting index for this section in the track
    size_t length;             // Length of this section
    struct section* next;      // Next section in the linked list
};

// Initialise a new sound_seg object
struct sound_seg* tr_init() {
    struct sound_seg* track = malloc(sizeof(struct sound_seg));
    track->buf = NULL;
    track->buf_capacity = 0;
    track->head = NULL;
    
    return track;
}

// Initialise a new section object
struct section* sc_init(size_t start, size_t length) {
    struct section* s = malloc(sizeof(struct section));
    s->start = start;
    s->length = length;
    s->next = NULL;

    return s;
}

struct wav_header {
    // Start of the master RIFF chunk
    char riff_header[4];       // Contains "RIFF"
    uint32_t wav_size;         // Size of file's wav section
    char wave_header[4];       // Contains "WAVE"
    
    // Start of the format chunk
    char fmt_header[4];        // Contains "fmt "
    uint32_t fmt_chunk_size;   // Chunk size 
    uint16_t audio_format;     // Audio format
    uint16_t num_channels;     // Number of channels
    uint32_t sample_rate;      // Sample rate
    uint32_t byte_rate;        // Byte rate 
    uint16_t sample_alignment; // Sample alignment
    uint16_t bit_depth;        // Bit depth

    // Start of the format chunk
    char data_header[4];       // Contains "data"
    uint32_t data_bytes;       // Number of bytes in data
};

// Load a WAV file into buffer
void wav_load(const char* fname, int16_t* dest) {
    // Open file for reading in binary
    FILE* file = fopen(fname, "rb");

    // Calculate file size to determine amount of bytes past the header
    fseek(file, 0, SEEK_END);
    size_t bytes_to_read = ftell(file) - 44;
    
    // Set pointer to start of audio data, then read all samples to dest
    fseek(file, 44, SEEK_SET);
    fread(dest, sizeof(int16_t), bytes_to_read / sizeof(int16_t), file);
    fclose(file);   
}

// Create/write a WAV file from buffer
void wav_save(const char* fname, const int16_t* src, size_t len) {
    // Open file to write binary, then initialise the WAV header
    FILE* file = fopen(fname, "wb");
    struct wav_header header;
    
    // Assigning RIFF header values
    memcpy(header.riff_header, "RIFF", 4);
    header.wav_size = len * sizeof(int16_t) + sizeof(header) - 8;
    memcpy(header.wave_header, "WAVE", 4);
    
    // Assigning FMT header values per specifications
    memcpy(header.fmt_header, "fmt ", 4);
    header.fmt_chunk_size = 16; // PCM chunk size = 16
    header.audio_format = 1;  // 1 = PCM format
    header.num_channels = 1;  // 1 = Mono
    header.sample_rate = 8000; // 8000Hz
    header.byte_rate = 8000 * sizeof(int16_t) * 1; // sample_rate * bytes_per_sample * channels
    header.sample_alignment = sizeof(int16_t) * 1; // bytes_per_sample * channels
    header.bit_depth = 16;    // 16 bits per sample
    
    // Assigning data header values
    memcpy(header.data_header, "data", 4);
    header.data_bytes = len * sizeof(int16_t);
    
    // Write header and the data provided, then close file
    fwrite(&header, sizeof(header), 1, file);
    fwrite(src, sizeof(int16_t), len, file);
    fclose(file);
}


// Helper function for resizing buffer
bool buf_check(struct sound_seg* track, size_t required_size) {
    if (track->buf_capacity >= required_size) {
        return true; // Already has enough capacity
    }
    
    // Calculate new capacity (double the curr, or the required size)
    size_t new_capacity;
    if (track->buf_capacity == 0) {
        new_capacity = required_size;
    } else {
        new_capacity = track->buf_capacity * 2;
    }

    if (new_capacity < required_size) {
        new_capacity = required_size;
    }
    
    // Reallocate the buffer
    int16_t* new_buf = realloc(track->buf, new_capacity * sizeof(int16_t));
    if (!new_buf) return false;

    track->buf = new_buf;
    track->buf_capacity = new_capacity;
    return true;
}

// Destroy a sound_seg object and free all allocated memory
void tr_destroy(struct sound_seg* track) {
    // Iterate through linked list, free each section
    struct section* curr = track->head;

    while (curr) {
        struct section* next = curr->next;
        free(curr);
        curr = next;
    }
    
    // Free the buffer and finally the track itself
    if (track->buf) {
        free(track->buf);
    }

    free(track);
}

// Return the length of the segment
size_t tr_length(struct sound_seg* track) {
    // If there are no sections, return 0
    if (!track || !track->head) return 0;
    
    // Find the last section
    struct section* curr = track->head;
    while (curr->next) {
        curr = curr->next;
    }
    
    // The length is the start of the last section plus its length
    return curr->start + curr->length;
}

// Read len elements from position pos into dest
void tr_read(struct sound_seg* track, int16_t* dest, size_t pos, size_t len) {
    // Tracking amount read so far, and relative position across sections
    size_t read_amt = 0;        
    size_t relative_pos = 0;
    struct section* curr = track->head;
    
    // Find the first section containing the position
    while (relative_pos + curr->length <= pos) {
        relative_pos += curr->length;
        curr = curr->next;
    }
    
    // Reading data across sections
    while (read_amt < len && curr) {
        // How far we are from the start of the current section
        size_t sc_offset = pos + read_amt - relative_pos;
        
        // How much of the section to read. Cannot exceed distance to section end.
        size_t read_len = curr->length - sc_offset;
        if (read_len > len - read_amt) {
            read_len = len - read_amt;
        }
        
        // Copy data from track buffer to destination
        memcpy(dest + read_amt, 
               track->buf + curr->start + sc_offset, 
               read_len * sizeof(int16_t));
        
        // Update the amount read
        read_amt += read_len;
        
        // Move to the next section
        relative_pos += curr->length;
        curr = curr->next;
    }
}

// Write len elements from src into position pos
void tr_write(struct sound_seg* track, const int16_t* src, size_t pos, size_t len) {
    // Tracking amount written so far, and relative position across sections
    size_t written_amt = 0;
    size_t relative_pos = 0;
    struct section* curr = track->head;
    
    // If head not null: find the first section containing the position
    while (curr && relative_pos + curr->length <= pos) {
        relative_pos += curr->length;
        curr = curr->next;
    }
    
    // Writing data across sections
    while (curr && written_amt < len) {
        // How far we are from the start of the current section
        size_t sc_offset = pos + written_amt - relative_pos;

        // How much we will be writing from section. Cannot exceed distance to section end.
        size_t write_len = len - written_amt;
        if (write_len > curr->length - sc_offset){
            write_len = curr->length - sc_offset;
        }

        // Ensure capacity and write data
        size_t tr_writepos = curr->start + sc_offset;
        if (!buf_check(track, tr_writepos + write_len)) return;
        
        memcpy(track->buf + tr_writepos, src + written_amt, write_len * sizeof(int16_t));        
        written_amt += write_len;
        relative_pos += curr->length;
        curr = curr->next;
    }
    
    // Initalise data, or append any remaining to end.
    if (written_amt < len) {
        // Determine where and how much to write to track
        size_t new_tr_writepos = tr_length(track);
        size_t remaining_len = len - written_amt;

        // Ensure track buf has adequate capacity
        if (!buf_check(track, new_tr_writepos + remaining_len)) return;
        memcpy(track->buf + new_tr_writepos, src + written_amt, remaining_len * sizeof(int16_t));
        
        // Create new section and initialise as head (or append to end)
        struct section* new_section = sc_init(new_tr_writepos, remaining_len);
        if (!new_section) return;
        
        if (!track->head) {
            track->head = new_section;
        } else {
            struct section* last = track->head;
            while (last->next) last = last->next;
            last->next = new_section;
        }
    }
}

/* To be completed:

// Delete a range of elements from the track
bool tr_delete_range(struct sound_seg* track, size_t pos, size_t len) {
    return true;
}

// Returns a string containing <start>,<end> ad pairs in target
char* tr_identify(struct sound_seg* target, struct sound_seg* ad){
    return NULL;
}

// Insert a section of src_track into dest_track at position destpos
void tr_insert(struct sound_seg* src_track,
            struct sound_seg* dest_track,
            size_t destpos, size_t srcpos, size_t len) {
    return;
}*/