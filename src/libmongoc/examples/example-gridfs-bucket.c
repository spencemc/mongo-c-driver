#include <mongoc.h>
#include <bson.h>
#include <stdio.h>
#include <stdlib.h>

int
main (int argc, char *argv[])
{
   const char *uri_string =
      "mongodb://127.0.0.1:27017/?appname=new-gridfs-example";
   mongoc_client_t *client;
   mongoc_database_t *db;
   mongoc_stream_t *file_stream;
   mongoc_stream_t *file_stream2;
   mongoc_stream_t *download_stream;
   mongoc_gridfs_bucket_t *bucket;
   mongoc_cursor_t *cursor;
   bson_t *filter;
   bool res;
   bson_value_t file_id;
   bson_error_t error;
   const bson_t *doc;
   char *str;
   ssize_t bytes_read;
   char buf[4096];
   int min_bytes = 1;

   mongoc_init ();

   if (argc != 3) {
      fprintf (stderr, "usage: %s SOURCE_FILE_PATH FILE_COPY_PATH\n", argv[0]);
      return EXIT_FAILURE;
   }

   /* 1. Make a bucket */
   client = mongoc_client_new (uri_string);
   db = mongoc_client_get_database (client, "test");

   bucket = mongoc_gridfs_bucket_new (db, NULL, NULL);

   /* 2. Insert a file called 'example.txt' into gridFS as  */
   file_stream = mongoc_stream_file_new_for_path (argv[1], O_RDONLY, 0);

   res = mongoc_gridfs_bucket_upload_from_stream (
      bucket, "my-file", file_stream, NULL, &file_id);
   if (!res) {
      mongoc_gridfs_bucket_error (bucket, &error);
      printf ("Error uploading file: %s\n", error.message);
      return EXIT_FAILURE;
   }

   /* 3. Read the file we just uploaded and print it out */
   download_stream =
      mongoc_gridfs_bucket_open_download_stream (bucket, &file_id);
   if (!download_stream) {
      mongoc_gridfs_bucket_error (bucket, &error);
      printf ("Error downloading file: %s\n", error.message);
      return EXIT_FAILURE;
   }

   bytes_read = mongoc_stream_read (download_stream, &buf, 4096, min_bytes, 0);
   while (bytes_read >= min_bytes) {
      bytes_read =
         mongoc_stream_read (download_stream, &buf, 4096, min_bytes, 0);
   }
   if (bytes_read == -1) {
      perror ("reading from GridFS bucket");
      return EXIT_FAILURE;
   }

   /* 4. Download the file in gridFS to a local file */
   file_stream2 =
      mongoc_stream_file_new_for_path (argv[2], O_CREAT | O_RDWR, 0);

   if (!file_stream2) {
      printf ("File to write to already exists\n");
      return EXIT_FAILURE;
   }

   res =
      mongoc_gridfs_bucket_download_to_stream (bucket, &file_id, file_stream2);
   if (!res) {
      mongoc_gridfs_bucket_error (bucket, &error);
      printf ("Error downloading file to stream: %s\n", error.message);
      return EXIT_FAILURE;
   }

   /* 5. List what files are available in gridFS */
   filter = bson_new ();
   cursor = mongoc_gridfs_bucket_find (bucket, filter, NULL);

   while (mongoc_cursor_next (cursor, &doc)) {
      str = bson_as_canonical_extended_json (doc, NULL);
      printf ("%s\n", str);
      bson_free (str);
   }

   /* 6. Delete the file that we added. */
   mongoc_gridfs_bucket_delete_by_id (bucket, &file_id);


   /* Done! Now let's cleanup */
   mongoc_stream_close (file_stream);
   mongoc_stream_destroy (file_stream);

   mongoc_stream_close (file_stream2);
   mongoc_stream_destroy (file_stream2);

   mongoc_stream_close (download_stream);
   mongoc_stream_destroy (download_stream);

   mongoc_cursor_destroy (cursor);
   bson_destroy (filter);
   mongoc_gridfs_bucket_destroy (bucket);

   mongoc_database_destroy (db);
   mongoc_client_destroy (client);
   mongoc_cleanup ();

   return EXIT_SUCCESS;
}