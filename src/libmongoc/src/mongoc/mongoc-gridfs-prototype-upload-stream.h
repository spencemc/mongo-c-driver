//
// Created by Spencer McKenney on 7/23/18.
//

#ifndef MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_STREAM_H
#define MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_STREAM_H


// NOTE: We need to add two streams for the new gridfs.

// Stream 1: Upload Stream
// Stream 2: Download stream.
//
// Lets throw an error if they try to do the wrong operations on a stream.


#include "mongoc-gridfs-prototype.h"


mongoc_stream_t *
mongoc_upload_stream_gridfs_new (mongoc_gridfs_bucket_file_t *file);

#endif //MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_STREAM_H
