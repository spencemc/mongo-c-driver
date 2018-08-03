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

#include <bson.h>
#include <bson-types.h>
#include "mongoc-collection.h"
#include "mongoc-stream.h"
#include "mongoc-gridfs-bucket.h"
#include "mongoc-gridfs-bucket-private.h"
#include "mongoc-gridfs-bucket-file-private.h"
#include "mongoc-stream-gridfs-upload-private.h"
#include "mongoc-write-concern-private.h"
#include "mongoc-stream-gridfs-download-private.h"
#include "mongoc-stream-private.h"

mongoc_gridfs_bucket_t *
mongoc_gridfs_bucket_new (mongoc_database_t *db, const bson_t *opts)
{
   bson_iter_t iter;
   bool r;
   const char *key;
   const bson_value_t *value;
   char *bucket_name;
   mongoc_gridfs_bucket_t *bucket;
   mongoc_read_prefs_t *read_prefs;
   mongoc_write_concern_t *write_concern;
   mongoc_read_concern_t *read_concern;
   int chunk_size;

   bucket = (mongoc_gridfs_bucket_t *) bson_malloc0 (sizeof *bucket);
   chunk_size = 255 * 1024; // Default 255KB
   bucket_name = "fs";

   r = bson_iter_init (&iter, opts);
   if (!r) {
      // TODO: How to set error? Should I return a mongoc_gridfs_bucket with
      // just the error set?
      return NULL;
   }

   while (bson_iter_next (&iter)) {
      key = bson_iter_key (&iter);
      value = bson_iter_value (&iter);
      if (strcmp(key, "bucketName") == 0) {
         bucket_name = value->value.v_utf8.str;
      } else if (strcmp(key, "chunkSizeBytes") == 0) {
         chunk_size = value->value.v_int32;
      } else if (strcmp(key, "writeConcern") == 0) {
         // TODO support the error message
         write_concern =
            _mongoc_write_concern_new_from_iter (&iter, NULL /* error */);

      } else if (strcmp(key, "readConcern") == 0) {
         // TODO

      } else if (strcmp(key, "readPreference") == 0) {
         // TODO
      }
   }

   bucket->chunks =
      mongoc_database_get_collection (db, strcat (bucket_name, ".files"));
   bucket->files =
      mongoc_database_get_collection (db, strcat (bucket_name, ".chunks"));

   if (write_concern) {
      mongoc_collection_set_write_concern (bucket->chunks, write_concern);
      mongoc_collection_set_write_concern (bucket->files, write_concern);
   }

   /* set read concern and read prefs here */

   bucket->chunk_size = chunk_size;
   bucket->bucket_name = bucket_name;

   /*
    THE FOLLOWING INDEXES MUST EXIST:
         an index on { filename : 1, uploadDate : 1 } on the files collection
         a unique index on { files_id : 1, n : 1 } on the chunks collection

    we NEED to do this before returning bucket.

    determine if the files collection is empty using the primary read preference
    mode.
    and if so, create the indexes described above if they do not already exist
    */

   return bucket;
}


mongoc_stream_t *
mongoc_gridfs_bucket_open_upload_stream_with_id (mongoc_gridfs_bucket_t *bucket,
                                                 const bson_value_t *file_id,
                                                 const char *filename,
                                                 const bson_t *opts)
{
   mongoc_gridfs_bucket_file_t *bucket_file;
   bson_iter_t iter;
   const char *key;
   const bson_value_t *value;
   bool r;
   int chunk_size;


   r = bson_iter_init (&iter, opts);
   if (!r) {
      // TODO: Set the error on the bucket.
      return NULL;
   }

   bucket_file =
      (mongoc_gridfs_bucket_file_t *) bson_malloc0 (sizeof *bucket_file);
   chunk_size = bucket->chunk_size;

   while (bson_iter_next (&iter)) {
      key = bson_iter_key (&iter);
      value = bson_iter_value (&iter);
      if (strcmp(key, "chunkSizeBytes") == 0) {
         chunk_size = value->value.v_int32;
      } else if (strcmp(key, "metadata") == 0) {
         bucket_file->metadata = bson_new_from_data (
            value->value.v_doc.data, value->value.v_doc.data_len);
      }
   }

   // QUESTION: Should we manage the memory of file_id for them? Or should we
   // copy the file_id
   bucket_file->file_id = file_id;

   // TODO: How do I copy these? Or should I just keep a reference to the
   // bucket? Hmmm...
   bucket_file->files = bucket->files;
   bucket_file->chunks = bucket->chunks;

   bucket_file->chunk_size = chunk_size;
   bucket_file->filename = filename;

   bucket_file->buffer = bson_malloc (chunk_size);
   bucket_file->in_buffer = 0;

   // return a new upload stream
   return _mongoc_upload_stream_gridfs_new (bucket_file);
}

mongoc_stream_t *
mongoc_gridfs_bucket_open_upload_stream (mongoc_gridfs_bucket_t *bucket,
                                         const char *filename,
                                         const bson_t *opts)
{
   bson_oid_t object_id;
   bson_value_t *file_id;

   bson_oid_init (&object_id, bson_context_get_default ());
   file_id = bson_malloc0 (sizeof (*file_id));
   file_id->value_type = BSON_TYPE_OID;
   file_id->value.v_oid = object_id;

   return mongoc_gridfs_bucket_open_upload_stream_with_id (
      bucket, file_id, filename, opts);
}

bool
mongoc_gridfs_bucket_upload_from_stream_with_id (mongoc_gridfs_bucket_t *bucket,
                                                 const bson_value_t *file_id,
                                                 const char *filename,
                                                 mongoc_stream_t *source,
                                                 const bson_t *opts)
{
   mongoc_stream_t *upload_stream;
   size_t bytes_read;
   char buf[512];

   upload_stream = mongoc_gridfs_bucket_open_upload_stream_with_id (
      bucket, file_id, filename, opts);


   /* while loop that reads from source and writes to upload stream */

   // QUESTION: What's the best way to buffer this?


   bytes_read = 0;

   while ((bytes_read = mongoc_stream_read (source, buf, 256, 1, 0))) {
      mongoc_stream_write (upload_stream, buf, bytes_read, 0);
   }

   // DONE! close the stream

   mongoc_stream_close (upload_stream);
   mongoc_stream_destroy (upload_stream);

   // QUESTION: Should we destroy their stream?

   return true;
}

bson_value_t *
mongoc_gridfs_bucket_upload_from_stream (mongoc_gridfs_bucket_t *bucket,
                                         const char *filename,
                                         mongoc_stream_t *source,
                                         const bson_t *opts)
{
   bool r;
   bson_oid_t object_id;
   bson_value_t *file_id;

   bson_oid_init (&object_id, bson_context_get_default ());
   file_id = bson_malloc0 (sizeof (*file_id));
   file_id->value_type = BSON_TYPE_OID;
   file_id->value.v_oid = object_id;

   r = mongoc_gridfs_bucket_upload_from_stream_with_id (
      bucket, file_id, filename, source, opts);
   if (!r) {
      bson_free (file_id);
      return NULL;
   }

   return file_id;
}


mongoc_stream_t *
mongoc_gridfs_bucket_open_download_stream (mongoc_gridfs_bucket_t *bucket,
                                           const bson_value_t *file_id)
{
   // 1. do a find in "files" collection for the given file id
   // 2. use the data in the "files" collection to construct a
   // gridfs-bucket-file
   // 3. create the stream and return it.
   mongoc_gridfs_bucket_file_t *bucket_file;
   bson_t *query;
   const bson_t *file;
   const char *key;
   const bson_value_t *value;
   bson_iter_t iter;
   mongoc_cursor_t *cursor;

   query = bson_new ();
   BSON_APPEND_VALUE (query, "_id", file_id);
   cursor = mongoc_collection_find_with_opts (bucket->files, query, NULL, NULL);
   mongoc_cursor_next (cursor, &file);

   // TODO: handle case where file doesn't exist

   bucket_file =
      (mongoc_gridfs_bucket_file_t *) bson_malloc0 (sizeof *bucket_file);

   bucket_file->file_id = file_id;

   bson_iter_init (&iter, file);

   while (bson_iter_next (&iter)) {
      key = bson_iter_key (&iter);
      value = bson_iter_value (&iter);
      // TODO: Do we need to validate the value type? So the user doesn't get
      // weird errors?
      if (strcmp(key, "length") == 0) {
         bucket_file->length = value->value.v_int64;
      } else if (strcmp(key, "chunkSize") == 0) {
         bucket_file->chunk_size = value->value.v_int32;
      } else if (strcmp(key, "filename") == 0) {
         bucket_file->filename = value->value.v_utf8.str;
      } else if (strcmp(key, "metadata") == 0) {
         bucket_file->metadata = bson_new_from_data (
            value->value.v_doc.data, value->value.v_doc.data_len);
      }
   }

   /*
   BSON_APPEND_VALUE (new_doc, "_id", file->file_id);
   BSON_APPEND_INT64 (new_doc, "length", file->length);
   BSON_APPEND_INT32 (new_doc, "chunkSize", file->chunk_size);
   BSON_APPEND_DATE_TIME (new_doc, "uploadDate",);
   BSON_APPEND_UTF8 (new_doc, "filename", file->filename);
   BSON_APPEND_DOCUMENT (new_doc, "metadata", file->metadata);
   */


   BSON_ASSERT (bucket_file->metadata);
   BSON_ASSERT (bucket_file->length);
   BSON_ASSERT (bucket_file->chunk_size);
   BSON_ASSERT (bucket_file->filename);
   BSON_ASSERT (bucket_file->file_id);

   return _mongoc_download_stream_gridfs_new (bucket_file);
}

bool
mongoc_gridfs_bucket_download_to_stream (mongoc_gridfs_bucket_t *bucket,
                                         const bson_value_t *file_id,
                                         mongoc_stream_t *destination)
{
   mongoc_stream_t *download_stream;
   size_t bytes_read;
   char buf[512];

   download_stream =
      mongoc_gridfs_bucket_open_download_stream (bucket, file_id);
   bytes_read = 0;

   // QUESTION: What's the best way to do this?

   while (bytes_read = mongoc_stream_read (download_stream, buf, 256, 1, 0)) {
      mongoc_stream_write (destination, buf, bytes_read, 0);
   }

   mongoc_stream_close (download_stream);
   mongoc_stream_destroy (download_stream);

   return true;
}

bool
mongoc_gridfs_bucket_delete_by_id (mongoc_gridfs_bucket_t *bucket,
                                   const bson_value_t *file_id)
{
   // 1. delete from files collection
   // 2. clean up chunks collection

   return false;
}

mongoc_cursor_t *
mongoc_gridfs_bucket_find (mongoc_gridfs_bucket_t *bucket,
                           const bson_t *filter,
                           const bson_t *opts)
{

   return NULL;
}

bool
mongoc_gridfs_bucket_error (mongoc_gridfs_bucket_t *bucket, bson_error_t *error)
{
   if (bucket->error) {
      error = bucket->error;
      return true;
   }

   return false;
}

void
mongoc_gridfs_bucket_destroy (mongoc_gridfs_bucket_t *bucket)
{

}

bson_value_t *
mongoc_gridfs_bucket_get_file_id_from_stream (mongoc_stream_t *stream)
{

   if(stream->type == MONGOC_STREAM_GRIDFS_UPLOAD){
      // TODO
   } else if (stream->type == MONGOC_STREAM_GRIDFS_DOWNLOAD){
      // TODO
   } else {
      // TODO error here
      return NULL;
   }
}