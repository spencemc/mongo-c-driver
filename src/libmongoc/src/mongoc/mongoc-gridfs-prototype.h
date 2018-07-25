/*
 * Copyright 2018-present MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_H
#define MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_H

#if !defined(MONGOC_INSIDE) && !defined(MONGOC_COMPILATION)
#error "Only <mongoc.h> can be included directly."
#endif

#include <bson.h>
#include "mongoc.h"

BSON_BEGIN_DECLS

/* Start of new GridFS API */

/*
 * In this file are all the methods that the user will use to interact with GridFS.
 * This implementation should follow the spec defined here:
 *
 * https://github.com/mongodb/specifications/blob/master/source/gridfs/gridfs-spec.rst
 *
 * In addition to these methods, a large part of the implementation work will be implementing
 * two new stream types:
 *
 *      1. GridFS Upload Stream.
 *          This is a stream that the user can only write to. It will carry all the needed metadata
 *          to write the given data to the chunks collection as well as updating the files collection.
 *
 *      2. GridFS Download Stream.
 *          This is a stream that only allows reads. It will carry all the metadata about the given file
 *          and will know how to read from the chunks successfully.
 *
 *
 * Not included yet (but will be in final implementation):
 *      The getter methods for mongoc_gridfs_bucket_t
 *      Potentially a few setter methods.
 *
 * Future work:
 *      https://github.com/mongodb/specifications/blob/master/source/gridfs/gridfs-spec.rst#advanced-api
 *      Add seeking to the download cursor.
 *
 */


typedef struct _mongoc_gridfs_bucket_t  mongoc_gridfs_bucket_t ;

struct _mongoc_gridfs_bucket_t {
  mongoc_collection_t* chunks;
  mongoc_collection_t* files;
  int chunk_size;
  char* bucket_name;
  /* Potentially more fields here if we need more data for implementation */
};

/*
 * Creates a new gridFS bucket in the given database.
 *
 * Returns a new gridFS bucket.
 *
 * Create opts:
 *        bucketName : String
 *        chunkSizeBytes : Int32
 *        writeConcern : WriteConcern
 *        readConcern : ReadConcern
 *        readPreference : ReadPreference
 *
 */
mongoc_gridfs_bucket_t*
mongoc_gridfs_bucket_new (mongoc_database_t *db,
                          const bson_t* opts);

/*
 * Opens an upload stream for the user to write their file's data into.
 * A file id is automatically generated.
 *
 * Returns the newly opened stream.
 *
 * (Note, there will be a method on the stream to get the file id it corresponds to)
 *
 * Upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */
mongoc_stream_t*
mongoc_gridfs_bucket_open_upload_stream(mongoc_gridfs_bucket_t* bucket,
                                        const char* filename,
                                        const bson_t* opts);

/*
 * Same as above but user specifies their own file id
 *
 *
 * Upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */
mongoc_stream_t*
mongoc_gridfs_bucket_open_upload_stream_with_id(mongoc_gridfs_bucket_t* bucket,
                                                const bson_value_t* file_id,
                                                const char* filename,
                                                const bson_t* opts);


/*
 * Uploads a file into gridFS by reading from the given mongoc_stream_t
 * A file id is automatically generated.
 *
 * Returns a bson_value_t containing the file id on success. On failure, it returns NULL.
 *
 * Upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */
bson_value_t*
mongoc_gridfs_bucket_upload_from_stream(mongoc_gridfs_bucket_t* bucket,
                                        const char* filename,
                                        mongoc_stream_t* source,
                                        const bson_t* opts);

/*
 * Same as above but user specifies their own file id
 *
 * Returns true on success. Otherwise, false.
 *
 * Upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */
bool
mongoc_gridfs_bucket_upload_from_stream_with_id(mongoc_gridfs_bucket_t* bucket,
                                                const bson_value_t* file_id,
                                                const char* filename,
                                                mongoc_stream_t* source,
                                                const bson_t* opts);
/*
 * Opens a download stream for the specified file id.
 *
 * Returns a download stream on success. On failure, it returns NULL.
 */
mongoc_stream_t*
mongoc_gridfs_bucket_open_download_stream(mongoc_gridfs_bucket_t* bucket,
                                          const bson_value_t* file_id);

/*
 * Writes the specified file to the given destination stream.
 *
 * Returns true on success. Otherwise, false.
 */
bool
mongoc_gridfs_bucket_download_to_stream(mongoc_gridfs_bucket_t* bucket,
                                        const bson_value_t* file_id,
                                        mongoc_stream_t* destination);

/*
 * Deletes the specified file from gridFS.
 *
 */
void
mongoc_gridfs_bucket_delete_by_id(mongoc_gridfs_bucket_t* bucket,
                                  const bson_value_t* file_id);


/*
 * Finds all files that match the given filter in GridFS
 *
 * Returns a cursor
 *
 * Find opts
 *      batchSize : Int32 optional;
 *      limit : Int32 optional;
 *      maxTimeMS: Int64 optional;
 *      noCursorTimeout : Boolean optional;
 *      skip : Int32;
 *      sort : Document optional;
 */

mongoc_cursor_t*
mongoc_gridfs_bucket_find(mongoc_gridfs_bucket_t* bucket,
                      const bson_t* filter,
                      const bson_t* opts);


/*
 * Get the error that happened while performing bucket operations
 *
 * If an error has happened, this returns true and sets the bson_error_t. Otherwise, this returns false.
 *
 */
bool
mongoc_gridfs_bucket_error(mongoc_gridfs_bucket_t* bucket, bson_error_t* error);

/*
 * Destroys and frees the gridfs bucket
 */
void
mongoc_gridfs_bucket_destroy(mongoc_gridfs_bucket_t* bucket);


BSON_END_DECLS

#endif //MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_H
