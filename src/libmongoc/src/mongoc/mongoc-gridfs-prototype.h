//
// Created by Spencer McKenney on 7/23/18.
//

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
 *        disableMD5: Boolean
 *
 */
mongoc_gridfs_bucket_t*
mongoc_gridfs_bucket_new (mongoc_database_t *db /* IN */,
                          bson_t* opts /* IN */);

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
mongoc_gridfs_open_upload_stream(mongoc_gridfs_bucket_t* bucket /* IN */,
                                 char* filename /* IN */,
                                 bson_t* opts /* IN */);

/*
 * Same as above but user specifies their own file id
 *
 *
 * Upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */
mongoc_stream_t*
mongoc_gridfs_open_upload_stream_with_id(mongoc_gridfs_bucket_t* bucket /* IN */,
                                         bson_value_t* file_id /* IN */,
                                         char* filename /* IN */,
                                         bson_t* opts /* IN */);


/*
 * Uploads a file into gridFS by reading from the given mongoc_stream_t
 * A file id is automatically generated.
 *
 * Returns true on success. On false, error will be set.
 *
 * Upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */
bool
mongoc_gridfs_upload_from_stream(mongoc_gridfs_bucket_t* bucket /* IN */,
                                 char* filename /* IN */,
                                 mongoc_stream_t* source /* IN */,
                                 bson_t* opts /* IN */,
                                 bson_value_t* file_id /* OUT */,
                                 bson_error_t *error /* OUT */);

/*
 * Same as above but user specifies their own file id
 *
 * Upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */
bool
mongoc_gridfs_upload_from_stream_with_id(mongoc_gridfs_bucket_t* bucket /* IN */,
                                         char* filename /* IN */,
                                         mongoc_stream_t* source /* IN */,
                                         bson_t* opts /* IN */,
                                         bson_value_t* file_id /* IN */,
                                         bson_error_t *error /* OUT */);
/*
 * Opens a download stream for the specified file id.
 *
 * Returns true on success. On false, error will be set.
 */
bool
mongoc_gridfs_open_download_stream(mongoc_gridfs_bucket_t* bucket /* IN */,
                                   bson_value_t* file_id /* IN */,
                                   mongoc_stream_t* download_stream /* OUT */,
                                   bson_error_t *error /* OUT */);

/*
 * Writes the specified file to the given destination stream.
 *
 * Returns true on success. On false, error will be set.
 */
bool
mongoc_gridfs_download_to_stream(mongoc_gridfs_bucket_t* bucket /* IN */,
                                 bson_value_t* file_id /* IN */,
                                 mongoc_stream_t* destination /* IN */,
                                 bson_error_t *error /* OUT */);

/*
 * Deletes the specified file from gridFS.
 *
 */
void
mongoc_gridfs_delete(mongoc_gridfs_bucket_t* bucket,
                     bson_value_t* file_id);


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
mongoc_gridfs_find_v2(mongoc_gridfs_bucket_t* bucket, /* IN */
                      bson_t* filter, /* IN */
                      bson_t* opts /* IN */);


/*
 * Destroys and frees the gridfs bucket
 */
void
mongoc_gridfs_bucket_destroy(mongoc_gridfs_bucket_t* bucket);


BSON_END_DECLS

#endif //MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_H
