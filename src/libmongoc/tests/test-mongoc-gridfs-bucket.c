#include <mongoc.h>
#define MONGOC_INSIDE
#include <mongoc-gridfs-file-private.h>
#include <mongoc-client-private.h>
#include <mongoc-gridfs-private.h>
#include <bson-types.h>

#undef MONGOC_INSIDE

#include "test-libmongoc.h"
#include "TestSuite.h"
#include "test-conveniences.h"
#include "mock_server/mock-server.h"
#include "mock_server/future.h"
#include "mock_server/future-functions.h"
#include "json-test.h"

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

bson_t *
parse_chunk_data(bson_t* chunk){
   bson_iter_t iter;
   bson_iter_t inner;
   bson_t* result;
   bson_value_t* value;
   bson_t* hex_doc;
   char* hex;
   int hex_len;
   char* key;

   result = bson_new();

   bson_iter_init(&iter, chunk);
   while(bson_iter_next(&iter)) {
      value = bson_iter_value(&iter);
      key = bson_iter_key(&iter);

      if(value->value_type == BSON_TYPE_DOCUMENT){
         hex_doc = bson_new_from_data(bson_iter_value(&iter)->value.v_doc.data, bson_iter_value(&iter)->value.v_doc.data_len);

         if(bson_iter_init_find(&inner, hex_doc, "$hex")){
            hex = bson_iter_value(&inner)->value.v_utf8.str;
            hex_len = bson_iter_value(&inner)->value.v_utf8.len;
            BSON_APPEND_BINARY(result, key, BSON_SUBTYPE_BINARY, hex, hex_len);
         }
      } else {
         BSON_APPEND_VALUE(result, key, value);
      }

   }
   return result;
}

void setup_gridfs_collections(mongoc_database_t* db, bson_t* data)
{
   mongoc_collection_t* files;
   mongoc_collection_t* chunks;
   bson_iter_t iter;
   bson_iter_t inner;
   bool r;

   files = mongoc_database_get_collection(db, "files");
   chunks = mongoc_database_get_collection(db, "chunks");

   if(bson_iter_init_find(&iter, data, "files")){
      bson_t* docs = bson_new_from_data(bson_iter_value(&iter)->value.v_doc.data, bson_iter_value(&iter)->value.v_doc.data_len);

      bson_iter_init(&inner, docs);
      while(bson_iter_next(&inner)){
         bson_t* doc = bson_new_from_data(bson_iter_value(&inner)->value.v_doc.data, bson_iter_value(&inner)->value.v_doc.data_len);
         printf("%s\n", bson_as_json(doc, NULL));
         r = mongoc_collection_insert_one(files, doc, NULL, NULL, NULL);
         ASSERT(r);
         bson_destroy(doc);
      }

      bson_destroy(docs);
   }

   if(bson_iter_init_find(&iter, data, "chunks")){
      bson_t* docs = bson_new_from_data(bson_iter_value(&iter)->value.v_doc.data, bson_iter_value(&iter)->value.v_doc.data_len);

      bson_iter_init(&inner, docs);
      while(bson_iter_next(&inner)){
         bson_t* doc = bson_new_from_data(bson_iter_value(&inner)->value.v_doc.data, bson_iter_value(&inner)->value.v_doc.data_len);
         bson_t* chunk = parse_chunk_data(doc);
         printf("%s\n", bson_as_json(chunk, NULL));
         r = mongoc_collection_insert_one(chunks, chunk, NULL, NULL, NULL);
         ASSERT(r);
         bson_destroy(doc);
      }

      bson_destroy(docs);
   }

}

static void
test_gridfs_cb (bson_t *scenario)
{
   mongoc_gridfs_bucket_t *gridfs;
   mongoc_database_t *db;
   mongoc_client_t *client;
   bson_iter_t iter;
   char *dbname;


   /* Make a gridfs on generated db */
   dbname = gen_collection_name ("test");
   client = test_framework_client_new ();
   db = mongoc_client_get_database (client, dbname);
   gridfs = mongoc_gridfs_bucket_new (db, NULL, NULL);

   /* Insert the data */
   if(bson_iter_init_find(&iter, scenario, "data")){
      bson_t* data = bson_new_from_data(bson_iter_value(&iter)->value.v_doc.data, bson_iter_value(&iter)->value.v_doc.data_len);
      setup_gridfs_collections(db, data);
      bson_destroy(data);
   }


   /* Do the operations */
   



   bson_free (dbname);
   mongoc_database_destroy(db);
}

static void
test_all_spec_tests (TestSuite *suite)
{
   char resolved[PATH_MAX];

   test_framework_resolve_path (JSON_DIR "/gridfs", resolved);
   install_json_test_suite (suite, resolved, &test_gridfs_cb);
}

void
test_gridfs_bucket_install (TestSuite *suite)
{
   test_all_spec_tests (suite);
   TestSuite_AddLive (
      suite, "/GridFS/bucket/create_bucket", test_create_bucket);
   TestSuite_AddLive (
      suite, "/GridFS/bucket/upload_and_download", test_upload_and_download);

}