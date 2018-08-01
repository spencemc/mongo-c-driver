//
// Created by Spencer McKenney on 7/23/18.
//

#include "mongoc-gridfs-bucket.h"
#include "mongoc-gridfs-bucket-private.h"
#include "mongoc-gridfs-bucket-file-private.h"
#include "mongoc-stream-gridfs-upload-private.h"

mongoc_gridfs_bucket_t *
mongoc_gridfs_bucket_new (mongoc_database_t *db,
                          const bson_t *opts)
{
   // general plan:
   // 1. get the collection for the gridfs and put it into the gridfs_bucket_t
   // 2. use the prefix from opts
   // 3. apply every other option if possible

   mongoc_gridfs_bucket_t *bucket;
   bucket = bson_malloc0 (sizeof (mongoc_gridfs_bucket_t));

   char *prefix = "fs";

   if (bson_has_field (opts, "prefix")) {
      // set prefix to field.
   }

   bucket->chunks =
      mongoc_database_get_collection (db, strcat (prefix, ".files"));

   // check for errors

   bucket->files =
      mongoc_database_get_collection (db, strcat (prefix, ".chunks"));

   // check for errors

   // set other mongoc_gridfs_bucket_t fields

   bucket->chunk_size = 255 * 1024; // Default 255KB

   return bucket;
}


mongoc_stream_t *
mongoc_gridfs_open_upload_stream (mongoc_gridfs_bucket_t *bucket,
                                  char *filename,
                                  bson_t *opts /* upload opts */)
{
   // opens a stream that you can write to.


   mongoc_gridfs_bucket_file_t *bucket_file;


   // set opts!
   // set filename!


   // return a new upload stream
   return _mongoc_upload_stream_gridfs_new (bucket_file);
}


bson_value_t *
mongoc_gridfs_upload_from_stream (mongoc_gridfs_bucket_t *bucket,
                                  char *filename,
                                  mongoc_stream_t *source /* download stream (Maybe replace with generic file stream?) */,
                                  bson_t *opts /* upload opts */) {
   // plan:
   //  1. call mongoc_gridfs_open_upload_stream
   //  2. continuously write to the upload stream from the given "source"
   //  stream.
   //  3. return value of new file.

   // done
}


mongoc_stream_t *
mongoc_gridfs_open_download_stream (mongoc_gridfs_bucket_t *bucket,
                                    bson_value_t *file_id)
{
   // opens a stream that the user can read from
}
