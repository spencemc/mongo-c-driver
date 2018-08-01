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


#ifndef MONGOC_GRIDFS_BUCKET_FILE_PRIVATE_H
#define MONGOC_GRIDFS_BUCKET_FILE_PRIVATE_H

#if !defined(MONGOC_COMPILATION)
#error "Only <mongoc.h> can be included directly."
#endif

BSON_BEGIN_DECLS

struct _mongoc_gridfs_bucket_file_t {
  /* Data about a specific file here */
};

typedef struct _mongoc_gridfs_bucket_file_t mongoc_gridfs_bucket_file_t;

ssize_t
mongoc_gridfs_bucket_file_writev (mongoc_gridfs_bucket_file_t *file,
                                  const mongoc_iovec_t *iov,
                                  size_t iovcnt,
                                  uint32_t timeout_msec);

BSON_END_DECLS

#endif /* MONGOC_GRIDFS_BUCKET_FILE_PRIVATE_H */
