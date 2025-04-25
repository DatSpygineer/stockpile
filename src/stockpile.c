/**
 * @file stockpile.h
 * @author Marcell Rozs
 * @brief Stockpile is a simple file collection format for game engines.
 * @version 1.0
 * @date 2025-04-25
 * 
*/

////////////////////////////////////////////////////////////
/// Includes
////////////////////////////////////////////////////////////
#include "stockpile.h"

#include <sys/stat.h>
#include <errno.h>
#include <zlib.h>

////////////////////////////////////////////////////////////
/// Local macros
////////////////////////////////////////////////////////////

#define STP_FAILURE(__code, __message, ...) StpSetError(__code, __message, ##__VA_ARGS__); goto STP_ERROR;
#define STP_ON_FAILURE STP_ERROR:
#define STP_FREE_SAFE(x) if ((x) != NULL) free(x);

////////////////////////////////////////////////////////////
/// Local type definitions
////////////////////////////////////////////////////////////

typedef struct StpArchiveBuilderEntry {
    char name[255];
    void* data;
    size_t size;
    uint8_t name_len;
} StpArchiveBuilderEntry;

struct StpArchive {
    void* data;
    size_t size;
    StpEntry* entries;
    size_t entry_count;
};

struct StpEntry {
    void* origin;
    char entry_name[256];
    size_t size;
};

struct StpArchiveBuilder {
    StpArchiveBuilderEntry* entries;
    uint32_t* crc_data;
    size_t count, capacity;
    bool enable_crc;
};

typedef struct StpArchiveHeader {
    char magic[4];
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t reserved_0;
    struct {
        uint8_t compressed : 1;
        uint8_t crc_enabled : 1;
        uint8_t reserved_1 : 6;
    };
    uint32_t raw_size;
    uint32_t uncompressed_size;
    uint32_t data_origin;
    uint32_t crc_origin;
    uint32_t entry_count;
} StpArchiveHeader;

typedef struct StpEntryRaw {
    char entry_name[255];
    uint32_t origin, size;
    uint8_t name_len;
} StpEntryRaw;

////////////////////////////////////////////////////////////
/// Local variables
////////////////////////////////////////////////////////////

static char s_stp_error_message[1024] = { 0 };
static StpErrorCode s_stp_error_code = STP_OK;
static StpErrorCallback s_stp_error_callback = NULL;

////////////////////////////////////////////////////////////
/// Local functions
////////////////////////////////////////////////////////////

static void StpSetError(StpErrorCode code, const char* description, ...) {
    s_stp_error_code = code;

    va_list args;
    va_start(args, description);
    vsnprintf(s_stp_error_message, 1024, description, args);
    va_end(args);

    if (s_stp_error_callback != NULL) {
        s_stp_error_callback(s_stp_error_code, s_stp_error_message);
    }
}
static void StpResetError() {
    s_stp_error_code = STP_OK;
    memset(s_stp_error_message, 0, 1024);
}

static bool StpArchiveWriteToFileF_CRC(const StpArchive* archive, FILE* file, bool compressed, const uint32_t* crc_data) {
    if (archive == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to write archive to file: argument \"archive\" is NULL!");
        return NULL;
    }
    if (file == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to write archive to file: argument \"file\" is NULL!");
        return NULL;
    }

    StpArchiveHeader header = {
        .magic = { 'S', 'T', 'P', 'A' },
        .version_major = STP_VERSION_MAJOR,
        .version_minor = STP_VERSION_MINOR,
        .compressed = compressed,
        .crc_enabled = crc_data != NULL,
        .raw_size = archive->size,
        .entry_count = archive->entry_count,
        .data_origin = sizeof(StpArchiveHeader),
        .crc_origin = 0
    };

    for (size_t i = 0; i < archive->entry_count; i++) {
        header.data_origin += strlen(archive->entries[i].entry_name) + 1;
        header.data_origin += sizeof(uint32_t) * 2;
    }

    size_t data_size = archive->size;
    void* data = malloc(data_size);

    if (compressed) {
        size_t compressed_size = 0;
        compress(data, &compressed_size, archive->data, archive->size);
        data_size = compressed_size;
    } else {
        memcpy(data, archive->data, archive->size);
    }

    header.crc_origin = header.data_origin + data_size;

    fwrite(&header, sizeof(header), 1, file);

    for (size_t i = 0; i < archive->entry_count; i++) {
        const uint8_t name_len = strlen(archive->entries[i].entry_name);
        const uint32_t origin = (uint32_t)(archive->entries[i].origin - archive->data);
        const uint32_t size = (uint32_t)archive->entries[i].size;

        fwrite(&name_len, 1, 1, file);
        fwrite(archive->entries[i].entry_name, name_len, 1, file);
        fwrite(&origin, sizeof(origin), 1, file);
        fwrite(&size, sizeof(size), 1, file);
    }

    fwrite(data, 1, data_size, file);
    free(data);

    if (crc_data) {
        fwrite(crc_data, sizeof(uint32_t), archive->entry_count, file);
    }

    return true;
}

////////////////////////////////////////////////////////////
/// Global functions
////////////////////////////////////////////////////////////

void StpSetErrorCallback(StpErrorCallback callback) {
    s_stp_error_callback = callback;
}
const char* StpGetError(StpErrorCode* code) {
    if (code != NULL) {
        *code = s_stp_error_code;
    }

    return s_stp_error_message;
}

uint32_t StpCrc32(const void* data, size_t length) {
    uint32_t crc = crc32(0, NULL, 0);
    for (size_t i = 0; i < length; i++) {
        crc = crc32(crc, ((const uint8_t*)data) + i, 1);
    }
    return crc;
}

StpArchive* StpOpenArchive(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        StpSetError(STP_ERROR_FILE_NOT_FOUND, "Failed to open file \"%s\": File doesn't exists!", filename);
        return NULL;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        StpSetError(STP_ERROR_FILE_OPEN, "Failed to open file \"%s\": %s", filename, strerror(errno));
        return NULL;
    }
    StpArchive* archive = StpOpenArchiveFromFile(file);
    fclose(file);
    return archive;
}
StpArchive* StpOpenArchiveFromFile(FILE* file) {
    void* raw_data = NULL;
    uint32_t* crc_data = NULL;
    StpEntryRaw* entries = NULL;
    StpArchive* result = NULL;

    if (!file) {
        STP_FAILURE(STP_ERROR_INVALID_ARGUMENT, "Argument \"file\" must not be null!");
    }

    StpArchiveHeader header;
    if (fread(&header, sizeof(header), 1, file) < sizeof(header)) {
        STP_FAILURE(STP_ERROR_FILE_READ, "Failed to read archive header!");
    }

    if (memcmp(header.magic, "STPA", 4) != 0 || memcmp(header.magic, "APTS", 4) != 0) {
        STP_FAILURE(STP_ERROR_IDENTIFIER_MISMATCH, "Identifier doesn't match!");
    }

    // TODO: Version handling

    entries = (StpEntryRaw*)calloc(header.entry_count, sizeof(StpEntryRaw));
    if (!entries) {
        STP_FAILURE(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate memory for raw entries!");
    }

    for (size_t i = 0; i < header.entry_count; i++) {
        if (fread(&entries[i].name_len, sizeof(entries[i].name_len), 1, file) < sizeof(entries[i].name_len)) {
            STP_FAILURE(STP_ERROR_FILE_READ, "Failed to read entry name length!");
        }

        if (entries[i].name_len == 0) {
            STP_FAILURE(STP_ERROR_OUT_OF_RANGE, "Entry name size must be more then zero!");
        }

        if (fread(entries[i].entry_name, sizeof(char), entries[i].name_len, file) < entries[i].name_len) {
            STP_FAILURE(STP_ERROR_FILE_READ, "Failed to read entry name!");
        }
        if (fread(&entries[i].origin, sizeof(entries[i].origin), 1, file) < sizeof(entries[i].origin)) {
            STP_FAILURE(STP_ERROR_FILE_READ, "Failed to read entry origin address!");
        }
        if (fread(&entries[i].size, sizeof(entries[i].size), 1, file) < sizeof(entries[i].size)) {
            STP_FAILURE(STP_ERROR_FILE_READ, "Failed to read entry size!");
        }
    }

    raw_data = malloc(header.raw_size);
    if (!raw_data) {
        STP_FAILURE(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate memory for raw data!");
    }

    fseek(file, header.data_origin, SEEK_SET);
    if (fread(raw_data, 1, header.raw_size, file) < header.raw_size) {
            STP_FAILURE(STP_ERROR_FILE_READ, "Failed to read raw data!");
    }

    result = (StpArchive*)malloc(sizeof(StpArchive));
    if (!result) {
        STP_FAILURE(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate memory for archive!");
    }

    if (header.compressed) {
        result->data = malloc(header.uncompressed_size);
        uncompress(result->data, &result->size, raw_data, header.raw_size);

        free(raw_data);
    } else {
        result->data = raw_data;
        result->size = header.raw_size;
    }

    result->entries = (StpEntry*)calloc(header.entry_count, sizeof(StpEntry));
    if (!result->entries) {
        STP_FAILURE(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate memory for archive entries!");
    }

    for (size_t i = 0; i < header.entry_count; i++) {
        memset(&result->entries[i], 0, sizeof(StpEntry));
        result->entries[i] = (StpEntry) {
            .origin = result->data + entries[i].origin,
            .size = entries[i].size
        };
        memcpy(result->entries[i].entry_name, entries[i].entry_name, entries[i].name_len * sizeof(char));
    }
    free(entries);

    if (header.crc_enabled) {
        crc_data = (uint32_t*)calloc(header.entry_count, sizeof(uint32_t));
        if (!crc_data) {
            STP_FAILURE(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate memory for crc data!");
        }

        fseek(file, header.crc_origin, SEEK_SET);
        if (fread(crc_data, sizeof(uint32_t), header.entry_count, file) < header.entry_count * sizeof(uint32_t)) {
            STP_FAILURE(STP_ERROR_FILE_READ, "Failed to read crc data!");
        }

        for (size_t i = 0; i < header.entry_count; i++) {
            if (StpCrc32(result->entries[i].origin, result->entries[i].size) != crc_data[i]) {
                STP_FAILURE(STP_ERROR_CRC, "CRC of entry \"%s\" doesn't match!", result->entries[i].entry_name);
            }
        }

        free(crc_data);
        crc_data = NULL;
    }

    return result;

STP_ON_FAILURE
    STP_FREE_SAFE(entries);
    STP_FREE_SAFE(raw_data);
    STP_FREE_SAFE(crc_data);
    STP_FREE_SAFE(result);

    return NULL;
}
void StpCloseArchive(StpArchive** archive) {
    if (archive != NULL && *archive != NULL) {
        free((*archive)->data);
        free((*archive)->entries);
        free(*archive);
        *archive = NULL;
    }
}
bool StpArchiveWriteToFile(const StpArchive* archive, const char* filename, bool compressed, bool enable_crc) {
    if (archive == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to write archive to file \"%s\": argument \"archive\" is NULL!", filename);
        return NULL;
    }

    FILE* f = fopen(filename, "wb");
    if (f == NULL) {
        StpSetError(STP_ERROR_FILE_OPEN, "Failed to open file \"%s\" for writing!", filename);
        return false;
    }

    const bool result = StpArchiveWriteToFileF(archive, f, compressed, enable_crc);
    fclose(f);
    return result;
}
bool StpArchiveWriteToFileF(const StpArchive* archive, FILE* file, bool compressed, bool enable_crc) {
    uint32_t* crc_data = NULL;
    if (enable_crc) {
        crc_data = (uint32_t*)calloc(sizeof(uint32_t), archive->entry_count);
        for (size_t i = 0; i < archive->entry_count; i++) {
            crc_data[i] = StpCrc32(archive->entries[i].origin, archive->entries[i].size);
        }
    }
    return StpArchiveWriteToFileF_CRC(archive, file, compressed, crc_data);
}

StpEntry* StpArchiveOpenEntry(const StpArchive* archive, const char* name) {
    if (name == NULL || strlen(name) == 0) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to get entry: argument \"name\" must not be NULL or empty!");
        return NULL;
    }

    if (archive == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to get entry \"%s\": argument \"archive\" is NULL!", name);
        return NULL;
    }

    for (size_t i = 0; i < archive->entry_count; i++) {
        if (strcmp(name, archive->entries[i].entry_name) == 0) {
            return archive->entries + i;
        }
    }

    StpSetError(STP_ERROR_ENTRY_NOT_FOUND, "Entry \"%s\" not found!", name);
    return NULL;
}
bool StpArchiveHasEntry(const StpArchive* archive, const char* name) {
    if (name != NULL && strlen(name) > 0 && archive != NULL) {
        for (size_t i = 0; i < archive->entry_count; i++) {
            if (strcmp(name, archive->entries[i].entry_name) == 0) {
                return true;
            }
        }
    }
    return false;
}
ssize_t StpReadEntry(const StpEntry* entry, void* buffer_out, const size_t buffer_max) {
    if (entry == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to read entry: argument \"entry\" must not be NULL!");
        return -1;
    }
    if (buffer_out == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to read entry \"%s\": argument \"buffer_out\" must not be NULL!", entry->entry_name);
        return -1;
    }
    if (buffer_max == 0) {
        StpSetError(STP_ERROR_OUT_OF_RANGE, "Failed to read entry \"%s\": argument \"buffer_max\" cannot be zero!", entry->entry_name);
        return -1;
    }

    const size_t size = entry->size > buffer_max ? buffer_max : entry->size;
    memcpy(buffer_out, entry->entry_name, size);
    return (ssize_t)size;
}
bool StpExtractEntryToFile(const StpEntry* entry, const char* out_filename) {
    if (entry == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to read entry: argument \"entry\" must not be NULL!");
        return false;
    }

    struct stat st;
    if (stat(out_filename, &st) == 0) {
        StpSetError(STP_ERROR_FILE_NOT_FOUND, "Failed to open file \"%s\": File not found!", out_filename);
        return false;
    }

    FILE* file = fopen(out_filename, "wb");
    if (file == NULL) {
        StpSetError(STP_ERROR_FILE_OPEN, "Failed to open file \"%s\": %s", out_filename, strerror(errno));
        return false;
    }

    const bool result = StpExtractEntryToFileF(entry, file);

    fclose(file);
    return result;
}
bool StpExtractEntryToFileF(const StpEntry* entry, FILE* out_file) {
    if (entry == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to write entry: argument \"entry\" must not be NULL!");
        return false;
    }
    if (out_file == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to write entry \"%s\": argument \"out_file\" must not be NULL!", entry->entry_name);
        return false;
    }

    errno = 0;
    if (fwrite(entry->origin, entry->size, 1, out_file) == entry->size) {
        StpSetError(STP_ERROR_FILE_WRITE, "Failed to write data of entry \"%s\": %s", entry->entry_name, strerror(errno));
        return false;
    }
    return true;
}
void StpCloseEntry(StpEntry** entry) {
    if (entry != NULL && *entry != NULL) {
        free(*entry);
        *entry = NULL;
    }
}

StpArchiveBuilder* StpCreateArchiveBuilder(size_t capacity, bool generate_crc) {
    StpArchiveBuilder* builder = (StpArchiveBuilder*)malloc(sizeof(StpArchiveBuilder));
    if (builder == NULL) {
        StpSetError(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate archive builder!");
        return NULL;
    }
    builder->entries = capacity > 0 ? (StpArchiveBuilderEntry*)malloc(sizeof(StpArchiveBuilderEntry) * capacity) : NULL;
    if (builder->entries == NULL && capacity > 0) {
        StpSetError(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate archive builder entries!");
        free(builder);
        return NULL;
    }

    builder->count = 0;
    builder->capacity = capacity;
    builder->enable_crc = generate_crc;
    builder->crc_data = builder->enable_crc && capacity > 0 ? (uint32_t*)calloc(sizeof(uint32_t), builder->capacity) : NULL;
    if ((builder->enable_crc && capacity > 0) && builder->crc_data == NULL) {
        StpSetError(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate crc data!");
        free(builder->entries);
        free(builder);
        return NULL;
    }

    return builder;
}
bool StpBuilderAppendFile(StpArchiveBuilder* builder, const char* filename, bool binary) {
    if (builder == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file \"%s\": argument \"builder\" must not be NULL!", filename);
        return false;
    }
    if (filename == NULL || strlen(filename) == 0) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"filename\" must not be NULL or empty!");
        return false;
    }

    const char* name = strrchr(filename,
#ifdef _WIN32
        '\\'
#else
        '/'
#endif
    );

    return StpBuilderAppendFileNameOverride(builder, filename, binary, name == NULL ? filename : name);
}
bool StpBuilderAppendFileNameOverride(StpArchiveBuilder* builder, const char* filename, bool binary, const char* entry_name) {
    if (builder == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file \"%s\": argument \"builder\" must not be NULL!", filename);
        return false;
    }
    if (filename == NULL || strlen(filename) == 0) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"filename\" must not be NULL or empty!");
        return false;
    }
    if (entry_name == NULL || strlen(entry_name) == 0) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"entry_name\" must not be NULL or empty!");
        return false;
    }

    struct stat st;
    if (stat(filename, &st) == 0) {
        StpSetError(STP_ERROR_FILE_NOT_FOUND, "Failed to open file \"%s\": %s", filename, strerror(errno));
        return false;
    }

    FILE* f = fopen(filename, binary ? "rb" : "r");
    if (f == NULL) {
        StpSetError(STP_ERROR_FILE_OPEN, "Failed to open file \"%s\": %s", filename, strerror(errno));
    }

    const bool result = StpBuilderAppendFileF(builder, f, entry_name);
    fclose(f);
    return result;
}
bool StpBuilderAppendFileF(StpArchiveBuilder* builder, FILE* file, const char* entry_name) {
    if (builder == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"builder\" must not be NULL!");
        return false;
    }
    if (file == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"file\" must not be NULL!");
        return false;
    }
    if (entry_name == NULL || strlen(entry_name) == 0) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"entry_name\" must not be NULL or empty!");
        return false;
    }

    fseek(file, 0, SEEK_END);
    const size_t len = ftell(file);
    fseek(file, 0, SEEK_SET);

    void* data = malloc(len);
    if (data == NULL) {
        StpSetError(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate file data");
        return false;
    }

    fread(data, 1, len, file);
    StpArchiveBuilderEntry entry = {
        .data = data,
        .size = len,
    };
    memset(entry.name, 0, 255);
    entry.name_len = (uint8_t)(strlen(entry_name) > 255u ? 255u : strlen(entry_name));
    memcpy(entry.name, entry_name, entry.name_len);

    builder->entries[builder->count++] = entry;
    return true;
}
bool StpBuilderAppendString(StpArchiveBuilder* builder, const char* entry_name, const char* value) {
    return StpBuilderAppendBinary(builder, entry_name, (const void*)value, strlen(value));
}
bool StpBuilderAppendBinary(StpArchiveBuilder* builder, const char* entry_name, const void* buffer, size_t buffer_size) {
    if (builder == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"builder\" must not be NULL!");
        return false;
    }
    if (entry_name == NULL || strlen(entry_name) == 0) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"entry_name\" must not be NULL or empty!");
        return false;
    }
    if (buffer == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append file: argument \"buffer\" must not be NULL!");
        return false;
    }
    if (buffer_size == 0) {
        StpSetError(STP_ERROR_OUT_OF_RANGE, "Failed to append file: Buffer size must be non-zero!");
    }

    void* data = malloc(buffer_size);
    if (data == NULL) {
        StpSetError(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate file data");
        return false;
    }
    memcpy(data, buffer, buffer_size);

    StpArchiveBuilderEntry entry = {
        .data = data,
        .size = buffer_size,
    };
    memset(entry.name, 0, 255);
    entry.name_len = (uint8_t)(strlen(entry_name) > 255 ? 255 : strlen(entry_name));
    memcpy(entry.name, entry_name, entry.name_len);

    if (builder->count >= builder->capacity) {
        if (!StpBuilderReserve(builder, builder->count + 5)) {
            free(data);
            return false;
        }
    }

    builder->entries[builder->count++] = entry;
    return true;
}
bool StpBuilderAppendArchive(StpArchiveBuilder* builder, const StpArchive* archive) {
    if (builder == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append archive: argument \"builder\" must not be NULL!");
        return false;
    }
    if (archive == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to append archive: argument \"archive\" must not be NULL!");
        return false;
    }

    for (size_t i = 0; i < archive->entry_count; i++) {
        const StpEntry entry = archive->entries[i];
        if (!StpBuilderAppendBinary(builder, entry.entry_name, entry.origin, entry.size)) {
            return false;
        }
    }

    return true;
}
bool StpBuilderReserve(StpArchiveBuilder* builder, size_t capacity) {
    if (builder == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to reserve entries: argument \"builder\" must not be NULL!");
        return false;
    }

    if (builder->entries == NULL) {
        builder->entries = malloc(capacity * sizeof(StpEntry));
        builder->capacity = capacity;
        builder->count = 0;
        return true;
    }

    if (capacity <= builder->capacity) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to reserve entries: Capacity must be more then the current!");
        return false;
    }
    StpArchiveBuilderEntry* entries = (StpArchiveBuilderEntry*)calloc(capacity, sizeof(StpArchiveBuilderEntry));
    if (entries == NULL) {
        StpSetError(STP_ERROR_OUT_OF_MEMORY, "Failed to reserve entries");
        return false;
    }
    memset(entries, 0, capacity * sizeof(StpArchiveBuilderEntry));
    memcpy(entries, builder->entries, builder->count * sizeof(StpArchiveBuilderEntry));

    builder->capacity = capacity;
    free(builder->entries);
    builder->entries = entries;
    return true;
}
void StpBuilderClear(StpArchiveBuilder* builder) {
    if (builder && builder->entries != NULL) {
        for (size_t i = 0; i < builder->count; i++) {
            free(builder->entries->data);
        }
        free(builder->entries);
        memset(builder, 0, sizeof(StpArchiveBuilder));
    }
}
StpArchive* StpBuilderFinalize(StpArchiveBuilder* builder) {
    if (builder == NULL) {
        StpSetError(STP_ERROR_INVALID_ARGUMENT, "Failed to create archive: argument \"builder\" must not be NULL!");
        return false;
    }

    StpArchive* result = (StpArchive*)malloc(sizeof(StpArchive));
    if (result == NULL) {
        STP_FAILURE(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate archive");
    }

    size_t size = 0;
    for (size_t i = 0; i < builder->count; i++) {
        size += builder->entries[i].size;
    }
    result->data = malloc(size);

    if (result->data == NULL) {
        STP_FAILURE(STP_ERROR_OUT_OF_MEMORY, "Failed to allocate archive data");
    }

    result->entries = (StpEntry*)calloc(sizeof(StpEntry), builder->count);

    size_t offset = 0;
    for (size_t i = 0; i < builder->count; i++) {
        const size_t entry_size = builder->entries[i].size;
        void* origin = result->data + offset;
        memcpy(origin, builder->entries[i].data, entry_size);
        offset += entry_size;

        result->entries[i] = (StpEntry){
            .origin = origin,
            .size = entry_size
        };
        memset(result->entries[i].entry_name, 0, 256);
        memcpy(result->entries[i].entry_name, builder->entries[i].name, builder->entries[i].name_len);
    }

    StpBuilderClear(builder);
    return result;
STP_ON_FAILURE
    if (result != NULL) {
        StpCloseArchive(&result);
        if (result != NULL) {
            free(result);
        }
    }
    return NULL;
}
bool StpBuilderFinalizeToFile(StpArchiveBuilder* builder, const char* filename, const bool compressed) {
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        StpSetError(STP_ERROR_FILE_OPEN, "Failed to open file for writing \"%s\": %s", filename, strerror(errno));
        return false;
    }

    const bool result = StpBuilderFinalizeToFileF(builder, file, compressed);
    fclose(file);

    return result;
}
bool StpBuilderFinalizeToFileF(StpArchiveBuilder* builder, FILE* file, const bool compressed) {
    const bool enable_crc = builder->enable_crc;
    StpArchive* archive = StpBuilderFinalize(builder);
    if (archive == NULL) {
        return false;
    }

    StpArchiveWriteToFileF(archive, file, compressed, enable_crc);

    StpCloseArchive(&archive);
    return true;
}
void StpFreeBuilder(StpArchiveBuilder** builder) {
    if (builder != NULL && *builder != NULL) {
        StpBuilderClear(*builder);
        free(*builder);
        *builder = NULL;
    }
}
