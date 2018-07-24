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


/* PUBLIC STUFF */

typedef struct _mongoc_gridfs_bucket_t  mongoc_gridfs_bucket_t ;
typedef struct _mongoc_gridfs_bucket_file_t mongoc_gridfs_bucket_file_t;



/* PRIVATE STUFF */


struct _mongoc_gridfs_bucket_t {
  mongoc_collection_t* chunks;
  mongoc_collection_t* files;
  int chunk_size;
  char* bucket_name;
  // TODO
};

struct _mongoc_gridfs_bucket_file_t {
  bson_value_t* id;
  char* filename;
  int offset;
  // TODO

};


// QUESTION 1: Should the options be a bson_t* or a struct?
// QUESTION 2: Which methods need bson_error_t? All of them?

/*
 * Valid create opts:
 *        bucketName : String
 *        chunkSizeBytes : Int32
 *        writeConcern : WriteConcern
 *        readConcern : ReadConcern
 *        readPreference : ReadPreference
 *        disableMD5: Boolean
 *
 */
mongoc_gridfs_bucket_t*
mongoc_gridfs_bucket_new (mongoc_database_t *db,
                          bson_t* opts /* create opts */,
                          bson_error_t *error);

/*
 * Valid upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */

mongoc_stream_t*
mongoc_gridfs_open_upload_stream(mongoc_gridfs_bucket_t* bucket,
                                 char* filename,
                                 bson_t* opts /* upload opts */,
                                 bson_error_t *error);

mongoc_stream_t*
mongoc_gridfs_open_upload_stream_with_id(mongoc_gridfs_bucket_t* bucket,
                                         bson_value_t* file_id,
                                         char* filename,
                                         bson_t* opts /* upload opts */,
                                         bson_error_t *error);


bson_value_t*
mongoc_gridfs_upload_from_stream(mongoc_gridfs_bucket_t* bucket,
                                 char* filename,
                                 mongoc_stream_t* source /* download stream (Maybe replace with generic file stream?) */,
                                 bson_t* opts /* upload opts */,
                                 bson_error_t *error);


void
mongoc_gridfs_upload_from_stream_with_id(mongoc_gridfs_bucket_t* bucket,
                                         bson_value_t* file_id,
                                         char* filename,
                                         mongoc_stream_t* source /* download stream (Maybe replace with generic file stream?) */,
                                         bson_t* opts /* upload opts */,
                                         bson_error_t *error);



// Download methods:

mongoc_stream_t *
mongoc_gridfs_open_download_stream(mongoc_gridfs_bucket_t* bucket,
                                   bson_value_t* file_id,
                                   bson_error_t *error);


void
mongoc_gridfs_download_to_stream(mongoc_gridfs_bucket_t* bucket,
                                 bson_value_t* file_id,
                                 mongoc_stream_t* stream,
                                 bson_error_t *error);



// Delete

void
mongoc_gridfs_delete(mongoc_gridfs_bucket_t* bucket,
                     bson_value_t* file_id);


/*
 * Valid find opts
 *      batchSize : Int32 optional;
 *      limit : Int32 optional;
 *      maxTimeMS: Int64 optional;
 *      noCursorTimeout : Boolean optional;
 *      skip : Int32;
 *      sort : Document optional;
 */

mongoc_cursor_t*
mongoc_gridfs_find_v2(mongoc_gridfs_bucket_t* bucket, bson_t* filter, bson_t* opts, bson_error_t *error);

// what's next?

// - Think about how to implement these functions.
//      - Maybe make a rough implementation
// - Go though old API and figure out how to do each of the operations in new API


// TODO: add all getters (and potentially some setters)


BSON_END_DECLS

#endif //MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_H
