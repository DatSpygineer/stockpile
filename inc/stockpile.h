#ifndef STOCKPILE_H
#define STOCKPILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <string.h>

#define STP_VERSION_MAJOR 1
#define STP_VERSION_MINOR 0

#if defined(_WIN32) && !defined(SSIZE_DEFINED)
    #define SSIZE_DEFINED
    #if defined(__amd64) || defined(__amd64__) || defined(__x86_64) || defined(__x86_64__)
        typedef int64_t ssize_t;
    #else
        typedef int32_t ssize_t;
    #endif
#endif

typedef struct StpArchive StpArchive;
typedef struct StpEntry StpEntry;
typedef struct StpArchiveBuilder StpArchiveBuilder;

typedef enum StpErrorCode {
    STP_OK,
    STP_ERROR_INVALID_ARGUMENT,
    STP_ERROR_OUT_OF_MEMORY,
    STP_ERROR_OUT_OF_RANGE,
    STP_ERROR_FILE_NOT_FOUND,
    STP_ERROR_FILE_OPEN,
    STP_ERROR_FILE_READ,
    STP_ERROR_FILE_WRITE,
    STP_ERROR_IDENTIFIER_MISMATCH,
    STP_ERROR_CRC,
    STP_ERROR_ENTRY_REDEFINITION,
    STP_ERROR_ENTRY_NOT_FOUND,
} StpErrorCode;

typedef void (*StpErrorCallback)(StpErrorCode code, const char* message);

void StpSetErrorCallback(StpErrorCallback callback);
const char* StpGetError(StpErrorCode* code);

uint32_t StpCrc32(const void* data, size_t length);

StpArchive* StpOpenArchive(const char* filename);
StpArchive* StpOpenArchiveFromFile(FILE* file);
void StpCloseArchive(StpArchive** archive);
bool StpArchiveWriteToFile(const StpArchive* archive, const char* filename, bool compressed, bool enable_crc);
bool StpArchiveWriteToFileF(const StpArchive* archive, FILE* file, bool compressed, bool enable_crc);

StpEntry* StpArchiveOpenEntry(const StpArchive* archive, const char* name);
bool StpArchiveHasEntry(const StpArchive* archive, const char* name);
ssize_t StpReadEntry(const StpEntry* entry, void* buffer_out, size_t buffer_max);
bool StpExtractEntryToFile(const StpEntry* entry, const char* out_filename);
bool StpExtractEntryToFileF(const StpEntry* entry, FILE* out_file);
void StpFreeEntry(StpEntry** entry);

StpArchiveBuilder* StpCreateArchiveBuilder(size_t capacity, bool generate_crc);
bool StpBuilderAppendFile(StpArchiveBuilder* builder, const char* filename, bool binary);
bool StpBuilderAppendFileNameOverride(StpArchiveBuilder* builder, const char* filename, bool binary, const char* entry_name);
bool StpBuilderAppendFileF(StpArchiveBuilder* builder, FILE* file, const char* entry_name);
bool StpBuilderAppendString(StpArchiveBuilder* builder, const char* entry_name, const char* value);
bool StpBuilderAppendBinary(StpArchiveBuilder* builder, const char* entry_name, const void* buffer, size_t buffer_size);
bool StpBuilderAppendArchive(StpArchiveBuilder* builder, const StpArchive* archive);
bool StpBuilderReserve(StpArchiveBuilder* builder, size_t capacity);
void StpBuilderClear(StpArchiveBuilder* builder);
StpArchive* StpBuilderFinalize(StpArchiveBuilder* builder);
bool StpBuilderFinalizeToFile(StpArchiveBuilder* builder, const char* filename, bool compressed);
bool StpBuilderFinalizeToFileF(StpArchiveBuilder* builder, FILE* file, bool compressed);
void StpFreeBuilder(StpArchiveBuilder** builder);

#ifdef __cplusplus
}
#endif

#endif
