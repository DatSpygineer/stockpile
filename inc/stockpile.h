/**
 * @file stockpile.h
 * @author Marcell Rozs
 * @brief Stockpile is a simple file collection format for game engines.
 * @version 1.0
 * @date 2025-04-25
 * 
*/

#ifndef STOCKPILE_H
#define STOCKPILE_H

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////
/// Includes
////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <string.h>

////////////////////////////////////////////////////////////
/// Global macros
////////////////////////////////////////////////////////////

#define STP_VERSION_MAJOR 1
#define STP_VERSION_MINOR 0

////////////////////////////////////////////////////////////
/// Global type definitions
////////////////////////////////////////////////////////////

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

/// @brief List of possible error codes.
typedef enum StpErrorCode {
    /// @brief No error
    STP_OK,
    /// @brief Argument is invalid, has unexpected value or NULL
    STP_ERROR_INVALID_ARGUMENT,
    /// @brief Failed to allocate memory.
    STP_ERROR_OUT_OF_MEMORY,
    /// @brief Value is out of range or has an unexpected value.
    STP_ERROR_OUT_OF_RANGE,
    /// @brief File path specified does not point to an existing file.
    STP_ERROR_FILE_NOT_FOUND,
    /// @brief Error while opening a file. Can be caused by permission issues or corrupt files.
    STP_ERROR_FILE_OPEN,
    /// @brief Error while reading file. Can be caused by reaching end of file unexpectedly.
    STP_ERROR_FILE_READ,
    /// @brief Error while writing file.
    STP_ERROR_FILE_WRITE,
    /// @brief Identifier in the header "magic number" doesn't match the expected value.
    STP_ERROR_IDENTIFIER_MISMATCH,
    /// @brief CRC mismatch, calculated CRC doesn't match the stored CRC.
    STP_ERROR_CRC,
    /// @brief An entry with the specified name already exists.
    STP_ERROR_ENTRY_REDEFINITION,
    /// @brief Entry with the specified name does not exists.
    STP_ERROR_ENTRY_NOT_FOUND,
} StpErrorCode;

/**
 * @brief Callback function for error handling.
 * @param code Error code @see StpErrorCode
 * @param 
 */
typedef void (*StpErrorCallback)(StpErrorCode code, const char* message);

////////////////////////////////////////////////////////////
/// Global function prototypes
////////////////////////////////////////////////////////////

/**
 * @brief Set callback function to call when an error is set.
 * 
 * @param callback Function pointer to use.
 */
void StpSetErrorCallback(StpErrorCallback callback);
/**
 * @brief Get the latest error code and desciption.
 * 
 * @param code Output pointer to get the error code. Can be NULL.
 * @return const char* Returns the error description.
 */
const char* StpGetError(StpErrorCode* code);

/**
 * @brief Calculate CRC32 checksum using zlib's crc32 algorithm.
 * 
 * @param data Binary data to hash.
 * @param length Size of the binary data in bytes.
 * @return uint32_t Returns the calculated checksum.
 */
uint32_t StpCrc32(const void* data, size_t length);

/**
 * @brief Open a stockpile at a specific path. This function allocates memory, use @c StpCloseArchive to free allocated memory. @see StpCloseArchive
 * 
 * @param filename Name or path of the stockpile file.
 * @return StpArchive* Returns an allocated pointer, that contains the stockpile data.
 * @return NULL Returns "NULL" on failure.
 */
StpArchive* StpOpenArchive(const char* filename);
/**
 * @brief Load stockpile from file stream. This function allocates memory, use @c StpCloseArchive to free allocated memory. @see StpCloseArchive
 * 
 * @param file File stream to load data from.
 * @return StpArchive* Returns an allocated pointer, that contains the stockpile data.
 * @return NULL Returns "NULL" on failure.
 */
StpArchive* StpOpenArchiveFromFile(FILE* file);
/**
 * @brief Free stockpile data.
 * 
 * @param archive Pointer to an archive pointer. This sets the pointer to "NULL".
 */
void StpCloseArchive(StpArchive** archive);
/**
 * @brief Write archive data to a file at the specified path.
 * 
 * @param archive Archive data to write.
 * @param filename Name or path to the output file.
 * @param compressed If set to "true", archive data will be compressed. This does not compress header, entries and CRC.
 * @param enable_crc If set to "true", calculate CRC32 checksums for entry data and enable CRC.
 * @return true Returns "true" if the stockpile file have been written successfully.
 * @return false Returns "false" on fault.
 */
bool StpArchiveWriteToFile(const StpArchive* archive, const char* filename, bool compressed, bool enable_crc);
/**
 * @brief Write archive data to a file at the specified path.
 * 
 * @param archive Archive data to write.
 * @param file File stream to write into.
 * @param compressed If set to "true", archive data will be compressed. This does not compress header, entries and CRC.
 * @param enable_crc If set to "true", calculate CRC32 checksums for entry data and enable CRC.
 * @return true Returns "true" if the stockpile file have been written successfully.
 * @return false Returns "false" on fault.
 */
bool StpArchiveWriteToFileF(const StpArchive* archive, FILE* file, bool compressed, bool enable_crc);

/**
 * @brief Get entry from the stockpile. This allocates memory, use @c StpCloseEntry to free allocated memory! @see StpCloseEntry
 * 
 * @param archive Source archive, must not be NULL!
 * @param name Name of the entry, must not be NULL or empty!
 * @return StpEntry* Returns a pointer to the archive entry.
 */
StpEntry* StpArchiveOpenEntry(const StpArchive* archive, const char* name);
/**
 * @brief Check if the stockpile contains a specific 
 * 
 * @param archive Source archive, must not be NULL!
 * @param name Name of the entry, must not be NULL or empty!
 * @return bool Returns wheather the entry exists in the stockpile or not.
 */
bool StpArchiveHasEntry(const StpArchive* archive, const char* name);
/**
 * @brief Read entry data out.
 * 
 * @param entry Entry to read, must not be NULL!
 * @param buffer_out Target buffer, must not be NULL!
 * @param buffer_max Maximum size of the target buffer in bytes. Must not be zero!
 * @return ssize_t Returns the amount of read bytes on success, returns -1 on failure.
 */
ssize_t StpReadEntry(const StpEntry* entry, void* buffer_out, size_t buffer_max);
/**
 * @brief Read entry data and write it into a file at the specified path.
 * 
 * @param entry Entry to read, must not be NULL!
 * @param out_filename Target file name/path.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpExtractEntryToFile(const StpEntry* entry, const char* out_filename);
/**
 * @brief Read entry data and write it into a file stream.
 * 
 * @param entry Entry to read, must not be NULL!
 * @param out_file Target file stream.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpExtractEntryToFileF(const StpEntry* entry, FILE* out_file);
/**
 * @brief Free entry data.
 * 
 * @param entry Entry pointer to free.
 */
void StpCloseEntry(StpEntry** entry);

/**
 * @brief Create a new instance of an archive builder.
 * 
 * @param capacity Initial capacity of the entry list.
 * @param generate_crc If set to "true", generates CRC checksum for the stockpile archive.
 * @return StpArchiveBuilder* Returns the allocated builder on success, returns NULL on failure.
 */
StpArchiveBuilder* StpCreateArchiveBuilder(size_t capacity, bool generate_crc);
/**
 * @brief Read file and store its data as archive entry.
 * 
 * @param builder Archive builder pointer.
 * @param filename File path/name to read out.
 * @param binary If set to "true", the file is treated as a binary file, otherwise it's treated as a text file.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderAppendFile(StpArchiveBuilder* builder, const char* filename, bool binary);
/**
 * @brief Read file and store its data as archive entry with custom entry name.
 * 
 * @param builder Archive builder pointer.
 * @param filename File path/name to read out.
 * @param binary If set to "true", the file is treated as a binary file, otherwise it's treated as a text file.
 * @param entry_name Name of the archive entry.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderAppendFileNameOverride(StpArchiveBuilder* builder, const char* filename, bool binary, const char* entry_name);
/**
 * @brief Read file and store its data as archive entry.
 * 
 * @param builder Archive builder pointer.
 * @param file File stream to read out.
 * @param binary If set to "true", the file is treated as a binary file, otherwise it's treated as a text file.
 * @param entry_name Name of the archive entry.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderAppendFileF(StpArchiveBuilder* builder, FILE* file, const char* entry_name);
/**
 * @brief Append string as archive entry.
 * 
 * @param builder Archive builder pointer.
 * @param entry_name Name of the archive entry.
 * @param value String data to use.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderAppendString(StpArchiveBuilder* builder, const char* entry_name, const char* value);
/**
 * @brief Append binary data as archive entry.
 * 
 * @param builder Archive builder pointer.
 * @param entry_name Name of the archive entry.
 * @param buffer Binary data to append.
 * @param buffer_size Size of the binary data in bytes.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderAppendBinary(StpArchiveBuilder* builder, const char* entry_name, const void* buffer, size_t buffer_size);
/**
 * @brief Merge stockpile archive entries into the builder.
 * 
 * @param builder Archive builder pointer.
 * @param archive Archive to merge into the builder.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderAppendArchive(StpArchiveBuilder* builder, const StpArchive* archive);
/**
 * @brief Reserve entries in archive builder.
 * 
 * @param builder Archive builder pointer.
 * @param capacity Target entry capacity.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderReserve(StpArchiveBuilder* builder, size_t capacity);
/**
 * @brief Clear builder entries and crc.
 * 
 * @param builder Archive builder pointer.
 */
void StpBuilderClear(StpArchiveBuilder* builder);
/**
 * @brief Create archive and clear builder.
 * 
 * @param builder Archive builder pointer.
 * @return StpArchive* Return the archive created from the builder on success, NULL on failure.
 */
StpArchive* StpBuilderFinalize(StpArchiveBuilder* builder);
/**
 * @brief Create archive and clear builder, then write the archive into a file.
 * 
 * @param builder Archive builder pointer.
 * @param filename Target file path/name.
 * @param compressed If set to "true", archive data will be compressed.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderFinalizeToFile(StpArchiveBuilder* builder, const char* filename, bool compressed);
/**
 * @brief Create archive and clear builder, then write the archive into a file.
 * 
 * @param builder Archive builder pointer.
 * @param file Target file stream.
 * @param compressed If set to "true", archive data will be compressed.
 * @return bool Returns "true" on success, "false" on fault.
 */
bool StpBuilderFinalizeToFileF(StpArchiveBuilder* builder, FILE* file, bool compressed);
/**
 * @brief Free allocated memory in archive builder. Sets the builder pointer to NULL.
 * 
 * @param builder Pointer to archive builder pointer.
 */
void StpFreeBuilder(StpArchiveBuilder** builder);

#ifdef __cplusplus
}
#endif

#endif
