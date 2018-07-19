
/*
 * NOTE: This file is for api review only.
 *
 * In this file are my thoughts on how to implement gridfs in a way that is spec compliant.
 * A lot of this stuff comes directly from the spec.
 *
 *
 * Main differences:
 *
 * Switch from a grid_fs object to a grid_fs bucket object
 * Switch from grid_fs_file_t to grid_fs_file_id_t
 *
 */

#ifndef MONGOC_GRIDFS_NEW_PRIVATE_H
#define MONGOC_GRIDFS_NEW_PRIVATE_H

#if !defined(MONGOC_INSIDE) && !defined(MONGOC_COMPILATION)
#error "Only <mongoc.h> can be included directly."
#endif

#include <bson.h>
#include "mongoc.h"

BSON_BEGIN_DECLS


/* PUBLIC STUFF */

typedef struct _mongoc_gridfs_bucket_t  mongoc_gridfs_bucket_t ;
typedef struct _mongoc_gridfs_file_id_t mongoc_gridfs_bucket_file_t;



/* PRIVATE STUFF */


struct _mongoc_gridfs_bucket_t {
  // TODO
};

struct _mongoc_gridfs_file_id_t {

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
mongoc_gridfs_bucket_t *
mongoc_gridfs_bucket_new (mongoc_database_t *db,
                          bson_t* opts /* create opts */,
                          bson_error_t *error);




/*
 * Valid upload opts:
 *      chunkSizeBytes : Int32
 *      metadata : Document
 */

mongoc_stream_t *
    mongoc_gridfs_open_upload_stream(mongoc_gridfs_bucket_t* bucket,
                                     char* filename,
                                     bson_t* opts /* upload opts */);

mongoc_stream_t *
    mongoc_gridfs_open_upload_stream_with_id(mongoc_gridfs_bucket_t* bucket,
                                             mongoc_gridfs_file_id_t* id,
                                             char* filename,
                                             bson_t* opts /* upload opts */);

mongoc_gridfs_file_id_t *
    mongoc_gridfs_upload_from_stream(mongoc_gridfs_bucket_t* bucket,
                                     char* filename,
                                     mongoc_stream_t* source,
                                     bson_t* opts /* upload opts */);

void
    mongoc_gridfs_upload_from_stream_with_id(mongoc_gridfs_bucket_t* bucket,
                                             mongoc_gridfs_file_id_t* file_id,
                                             char* filename,
                                             mongoc_stream_t* source,
                                             bson_t* opts /* upload opts */);


// Download methods:

mongoc_stream_t *
    mongoc_gridfs_open_download_stream(mongoc_gridfs_bucket_t* bucket,
                                       mongoc_gridfs_file_id_t* file_id);

void
    mongoc_gridfs_download_to_stream(mongoc_gridfs_bucket_t* bucket,
                                     mongoc_gridfs_file_id_t* file_id,
                                     mongoc_stream_t* stream);


// Delete

void
    mongoc_gridfs_delete(mongoc_gridfs_bucket_t* bucket,
                         mongoc_gridfs_file_id_t* file_id);


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
    mongoc_gridfs_new_find(mongoc_gridfs_bucket_t* bucket, bson_t* filter, bson_t* opts);


BSON_END_DECLS;


#endif /* MONGOC_NEW_GRIDFS_H */

