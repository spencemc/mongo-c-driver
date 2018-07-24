//
// Created by Spencer McKenney on 7/23/18.
//

#include "mongoc-gridfs-prototype-bucket-file.h"
#include "mongoc-gridfs-prototype.h"

/** writev against a gridfs file
 *  timeout_msec is unused */
ssize_t
mongoc_gridfs_bucket_file_writev (mongoc_gridfs_bucket_file_t *file,
                           const mongoc_iovec_t *iov,
                           size_t iovcnt,
                           uint32_t timeout_msec)
{
    // Essentially, bucket_file_t is just for storing gridfs metadata
    uint32_t bytes_written = 0;
    int32_t r;
    size_t i;
    uint32_t iov_pos;
    int chunk_size = 255 * 1024;

    char buffer[chunk_size];
    int buffer_left = chunk_size;
    int offset = 0;

    // TODO: make sure that we continue writing where we left off. Store data in bucket_file_t

    if(file->offset > 0){
        // get last chunk, and put in buffer to be rewritten
    }

    ENTRY;

    BSON_ASSERT (file);
    BSON_ASSERT (iov);
    BSON_ASSERT (iovcnt);

    for(i = 0; i < iovcnt; i++){
        // plan:
        // start writing bytes at the beginning of iov and continue to append until we run out.
        // flush a write when necessary
        int written_this_iov = 0;

        while(written_this_iov < iov[i].iov_len){
            int to_copy = max(iov[i].iov_len - written_this_iov, buffer_left);
            memcpy(buffer, iov[i].iov_base, to_copy, offset);
            buffer_left = buffer_left - to_copy;
            written_this_iov += to_copy;
            bytes_written += to_copy;
            if(buffer_left == 0){
                // write a chunk
                mongoc_gridfs_bucket_write_chunk();
                offset = 0;
                buffer_left = chunk_size;
                memset(buffer, chunk_size, 0);
            } else {
                offset += to_copy;
            }
        }

    }

    if(buffer_left != chunk_size){
        // write last chunk here from buffer here! OR save in the file? And finish it off when we save to files collection
    }

    RETURN (bytes_written);
}


// This function writes a chunk to the collection with the given bytes and number
ssize_t
mongoc_gridfs_bucket_write_chunk (char* data, int n) {

}


/** readv against a gridfs file
 *  timeout_msec is unused */
ssize_t
mongoc_gridfs_bucket_file_readv (mongoc_gridfs_file_t *file,
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