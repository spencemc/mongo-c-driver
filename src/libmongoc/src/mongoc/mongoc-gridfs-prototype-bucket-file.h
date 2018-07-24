//
// Created by Spencer McKenney on 7/23/18.
//

#ifndef MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_BUCKET_FILE_H
#define MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_BUCKET_FILE_H

#include <limits.h>
#include <time.h>
#include <errno.h>

#include "mongoc-cursor.h"
#include "mongoc-cursor-private.h"
#include "mongoc-collection.h"
#include "mongoc-gridfs.h"
#include "mongoc-gridfs-private.h"
#include "mongoc-gridfs-file.h"
#include "mongoc-gridfs-file-private.h"
#include "mongoc-gridfs-file-page.h"
#include "mongoc-gridfs-file-page-private.h"
#include "mongoc-iovec.h"
#include "mongoc-trace-private.h"
#include "mongoc-error.h"
#include "mongoc-gridfs-prototype.h"

ssize_t
mongoc_gridfs_bucket_file_writev (mongoc_gridfs_bucket_file_t *file,
                                  const mongoc_iovec_t *iov,
                                  size_t iovcnt,
                                  uint32_t timeout_msec);


#endif //MONGO_C_DRIVER_MONGOC_GRIDFS_PROTOTYPE_BUCKET_FILE_H
