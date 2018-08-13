#include <mongoc.h>
#define MONGOC_INSIDE
#include <mongoc-gridfs-file-private.h>
#include <mongoc-client-private.h>
#include <mongoc-gridfs-private.h>

#undef MONGOC_INSIDE

#include "test-libmongoc.h"
#include "TestSuite.h"
#include "test-conveniences.h"
#include "mock_server/mock-server.h"
#include "mock_server/future.h"
#include "mock_server/future-functions.h"

void
test_create_bucket ()
{
   /* Tests creating a bucket with all opts set */
   mongoc_gridfs_bucket_t *gridfs;
   mongoc_read_prefs_t *read_prefs;
   mongoc_write_concern_t *write_concern;
   mongoc_read_concern_t *read_concern;
   mongoc_database_t *db;
   mongoc_client_t *client;
   bson_t *opts;
   char *dbname;

   client = test_framework_client_new ();
   ASSERT (client);

   dbname = gen_collection_name ("test");

   db = mongoc_client_get_database (client, dbname);

   bson_free (dbname);

   ASSERT (db);

   opts = bson_new ();

   /* write concern */
   write_concern = mongoc_write_concern_new ();
   mongoc_write_concern_set_w (write_concern, 1);
   mongoc_write_concern_append (write_concern, opts);

   /* read concern */
   read_concern = mongoc_read_concern_new ();
   mongoc_read_concern_set_level (read_concern,
                                  MONGOC_READ_CONCERN_LEVEL_LOCAL);
   mongoc_read_concern_append (read_concern, opts);

   /* other opts */
   BSON_APPEND_UTF8 (opts, "bucketName", "test-gridfs");
   BSON_APPEND_INT32 (opts, "chunkSizeBytes", 10);

   read_prefs = mongoc_read_prefs_new (MONGOC_READ_PRIMARY);

   gridfs = mongoc_gridfs_bucket_new (db, opts, read_prefs);

   ASSERT (gridfs);

   mongoc_gridfs_bucket_destroy (gridfs);
   bson_destroy (opts);
   mongoc_write_concern_destroy (write_concern);
   mongoc_read_concern_destroy (read_concern);
   mongoc_read_prefs_destroy (read_prefs);
   mongoc_database_destroy (db);
   mongoc_client_destroy (client);
}

void
test_upload_and_download ()
{
   mongoc_gridfs_bucket_t *gridfs;
   mongoc_stream_t *upload_stream;
   mongoc_stream_t *download_stream;
   bson_value_t file_id;
   mongoc_database_t *db;
   mongoc_client_t *client;
   bson_t *opts;
   /* big enough to hold all of str */
   char buf[100];
   char *str;
   char *dbname;
   bool r;

   str = "This is a test sentence with multiple chunks.";

   client = test_framework_client_new ();

   ASSERT (client);

   dbname = gen_collection_name ("test");

   db = mongoc_client_get_database (client, dbname);

   bson_free (dbname);

   ASSERT (db);

   opts = bson_new ();
   BSON_APPEND_INT32 (opts, "chunkSizeBytes", 10);

   gridfs = mongoc_gridfs_bucket_new (db, opts, NULL);
   upload_stream =
      mongoc_gridfs_bucket_open_upload_stream (gridfs, "my-file", NULL);
   r = mongoc_gridfs_bucket_get_file_id_from_stream (upload_stream, &file_id);

   ASSERT (r);
   ASSERT (upload_stream);

   /* copy str into the buffer and write it to gridfs */
   bson_snprintf (buf, sizeof (buf), "%s", str);
   mongoc_stream_write (upload_stream, buf, strlen (str), 0);
   mongoc_stream_destroy (upload_stream);

   /* reset the buffer */
   memset (buf, 0, 30);

   /* download str back into the buffer from gridfs */
   download_stream =
      mongoc_gridfs_bucket_open_download_stream (gridfs, &file_id);
   mongoc_stream_read (download_stream, buf, 100, 1, 0);

   /* compare */
   ASSERT (strcmp (buf, str) == 0);

   bson_destroy (opts);
   mongoc_stream_destroy (download_stream);
   mongoc_gridfs_bucket_destroy (gridfs);
   mongoc_database_destroy (db);
   mongoc_client_destroy (client);
}

void
test_gridfs_bucket_install (TestSuite *suite)
{
   TestSuite_AddLive (
      suite, "/GridFS/bucket/create_bucket", test_create_bucket);
   TestSuite_AddLive (
      suite, "/GridFS/bucket/upload_and_download", test_upload_and_download);
}