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

#include <ntsid.h>

#include "mongoc-collection.h"
#include "mongoc-gridfs-bucket-file-private.h"
#include "mongoc-gridfs-bucket-private.h"
#include "mongoc-iovec.h"
#include "mongoc-trace-private.h"
#include "mongoc-stream-gridfs-download-private.h"
#include "mongoc-stream-gridfs-upload-private.h"

size_t
min (size_t a, size_t b)
{
   return a < b ? a : b;
}

// This function writes a chunk to the collection with the given bytes and
// number
void
mongoc_gridfs_bucket_write_chunk (mongoc_gridfs_bucket_file_t *file)
{
   // Now, our buffer is full, write a chunk at curr_chunk

   // files_id: id
   // data: binary data
   // n : chunk #

   bool r;

   bson_t *new_doc = bson_new ();

   BSON_APPEND_INT32 (new_doc, "n", file->curr_chunk);
   BSON_APPEND_VALUE (new_doc, "files_id", file->file_id);
   BSON_APPEND_BINARY (new_doc,
                       "data",
                       BSON_SUBTYPE_BINARY,
                       file->buffer,
                       (uint32_t) file->in_buffer);


   r = mongoc_collection_insert_one (file->chunks,
                                     new_doc,
                                     NULL /* opts */,
                                     NULL /* reply */,
                                     NULL /* error */);
   if (!r) {
      // TODO think about errors;
   }

   file->curr_chunk++;

   // reset buffer
   memset (file->buffer, file->chunk_size, 0);
   file->in_buffer = 0;
}

// This function writes a chunk to the collection with the given bytes and
// number
void
mongoc_gridfs_bucket_read_chunk (mongoc_gridfs_bucket_file_t *file)
{
   // Now, our buffer has been completely read.
   // Read another chunk at curr_chunk

   // files_id: id
   // data: binary data
   // n : chunk #

   bool r;


}

/** writev against a gridfs file
 *  timeout_msec is unused */
ssize_t
mongoc_gridfs_bucket_file_writev (mongoc_gridfs_bucket_file_t *file,
                                  const mongoc_iovec_t *iov,
                                  size_t iovcnt,
                                  uint32_t timeout_msec)
{
   uint32_t bytes_written;
   int32_t r;
   size_t i;
   uint32_t iov_pos;

   bytes_written = 0;

   // TODO: make sure that we continue writing where we left off.


   // Cases to think about
   // 1. They write one byte at a time
   // 2. Writing 1 and a half chunks of data


   ENTRY;

   BSON_ASSERT (file);
   BSON_ASSERT (iov);
   BSON_ASSERT (iovcnt);

   if (file->saved) {
      // ERROR! Can't write to this file anymore because it has already been
      // saved. (Writing to a closed stream essentially)
   }

   for (i = 0; i < iovcnt; i++) {
      // plan:
      // start writing bytes at the beginning of iov and continue to append
      // until we run out.
      // flush a write when necessary
      int written_this_iov = 0;

      while (written_this_iov < iov[i].iov_len) {
         size_t to_copy = min (iov[i].iov_len - written_this_iov,
                               file->chunk_size - file->in_buffer);
         memcpy (file->buffer + file->in_buffer,
                 iov[i].iov_base + written_this_iov,
                 to_copy);
         file->in_buffer += to_copy;
         written_this_iov += to_copy;
         bytes_written += to_copy;
         if (file->in_buffer == file->chunk_size) {
            // write a chunk from buffer
            mongoc_gridfs_bucket_write_chunk (file);
         }
      }
   }

   if (file->in_buffer != 0) {
      // write last chunk here from buffer here! OR save in the file? And finish
      // it off when we save to files collection
   }

   RETURN (bytes_written);
}


/** readv against a gridfs file
 *  timeout_msec is unused */
ssize_t
mongoc_gridfs_bucket_file_readv (mongoc_gridfs_bucket_file_t *file,
                                 mongoc_iovec_t *iov,
                                 size_t iovcnt,
                                 size_t min_bytes,
                                 uint32_t timeout_msec)
{
   uint32_t bytes_read = 0;
   int32_t r;
   size_t i;
   uint32_t iov_pos;

   ENTRY;

   BSON_ASSERT (file);
   BSON_ASSERT (iov);
   BSON_ASSERT (iovcnt);




   RETURN (bytes_read);
}


// Only call this function when all data for the file has been written into the
// stream! This will perform the remaining
// steps to solidify the file into gridFS
int
mongoc_gridfs_bucket_file_save (mongoc_gridfs_bucket_file_t *file)
{
   // 1. make sure no errors have occurred.

   // 2. check if we need to write a chunk, write it.

   // 3. write the files data to the "files" collection
   bool r;
   bson_t *new_doc;

   if (file->saved) {
      // SUCCESS! It already happened
      return -1;
   }


   // TODO check for errors

   if (file->in_buffer != 0) {
      mongoc_gridfs_bucket_write_chunk (file);
   }

   new_doc = bson_new ();

   BSON_APPEND_VALUE (new_doc, "_id", file->file_id);
   BSON_APPEND_INT64 (new_doc, "length", file->length);
   BSON_APPEND_INT32 (new_doc, "chunkSize", file->chunk_size);
   BSON_APPEND_DATE_TIME (new_doc, "uploadDate", bson_get_monotonic_time());
   BSON_APPEND_UTF8 (new_doc, "filename", file->filename);
   BSON_APPEND_DOCUMENT (new_doc, "metadata", file->metadata);

   r = mongoc_collection_insert_one (file->files, new_doc, NULL, NULL, NULL);
   if (!r) {
      // TODO figure out errors
   }

   file->saved = true;

   return -1;
}

void
mongoc_gridfs_bucket_file_destroy (mongoc_gridfs_bucket_file_t *file)
{
   /* Do cleanup! */
   mongoc_collection_destroy (file->files);
   mongoc_collection_destroy (file->chunks);
   bson_free (file->file_id);
   bson_destroy (file->metadata);
   bson_free (file->buffer);
   bson_free (file);
}
