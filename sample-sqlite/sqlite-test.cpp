#include <sqlite3.h>

#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>
#include <memory>

using namespace std::literals::string_view_literals;

template <typename T>
struct sqlite_deleter {
   void operator ()(T *ptr)
   {
      sqlite3_free(ptr);
   }
};

int count_results(void *ptr, int ncol, char **values, char **names)
{
   (*reinterpret_cast<int*>(ptr))++;

   return 0;
}

int main(void)
{
   printf("SQLite version: %s\n", SQLITE_VERSION);

   printf("SQLite default threading: %s\n", sqlite3_threadsafe() == 0 ? "single-threaded" :
                                        sqlite3_threadsafe() == 2 ? "multi-threaded" : "serialized");

   if(SQLITE_VERSION_NUMBER != sqlite3_libversion_number())
      fprintf(stderr, "WARNING: SQLite header has a different version than the library\n");

   sqlite3 *ppDb = nullptr;
   int errcode = SQLITE_OK;

   try {
      char *errmsg = nullptr;

      if(sqlite3_config(SQLITE_CONFIG_MULTITHREAD) != SQLITE_OK)
         throw std::runtime_error("Cannot configure SQLite to operate as multi-threaded");

      printf("Switched SQLite to operate as multi-threaded\n");

      if(sqlite3_initialize() != SQLITE_OK)
         throw std::runtime_error("SQLite cannot be initialized");

      if((errcode = sqlite3_open_v2("sqlite-test.db", &ppDb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr)) != SQLITE_OK)
         throw std::runtime_error(sqlite3_errstr(errcode));

      int result_count = 0;

      if(sqlite3_exec(ppDb, "select name from sqlite_schema where type='table' and name='test_table';", count_results, &result_count, &errmsg) != SQLITE_OK)
         throw std::runtime_error(errmsg);

      // drop the table if it already exists, so schema can be changed between tests
      if(result_count != 0) {
         if(sqlite3_exec(ppDb, "drop table test_table;", nullptr, nullptr, &errmsg) != SQLITE_OK)
            throw std::runtime_error(std::unique_ptr<char, sqlite_deleter<char>>(errmsg).get());
      }

      if(sqlite3_exec(ppDb, "create table test_table (rnum REAL NULL, inum INTEGER NOT NULL, txt TEXT NOT NULL);", nullptr, nullptr, &errmsg) != SQLITE_OK)
         throw std::runtime_error(std::unique_ptr<char, sqlite_deleter<char>>(errmsg).get());

      if(sqlite3_exec(ppDb, "insert into test_table (rnum, inum, txt) values (7.89, 123, 'abc');", nullptr, nullptr, &errmsg) != SQLITE_OK)
         throw std::runtime_error(std::unique_ptr<char, sqlite_deleter<char>>(errmsg).get());

      if(sqlite3_exec(ppDb, "insert into test_table (rnum, inum, txt) values (NULL, 456, 'xyz');", nullptr, nullptr, &errmsg) != SQLITE_OK)
         throw std::runtime_error(std::unique_ptr<char, sqlite_deleter<char>>(errmsg).get());

      // rowid is not included in the default list of selected columns
      std::string_view sql = "select rowid, * from test_table order by rowid desc;"sv;

      sqlite3_stmt *ppStmt = nullptr;

      if((errcode = sqlite3_prepare_v2(ppDb, sql.data(), (int) sql.length()+1, &ppStmt, nullptr)) != SQLITE_OK)
         throw std::runtime_error(sqlite3_errstr(errcode));

      printf("test_table:\n");

      while((errcode = sqlite3_step(ppStmt)) == SQLITE_ROW) {
         for(int i = 0; i < sqlite3_column_count(ppStmt); i++) {
            printf("%7s: ", sqlite3_column_name(ppStmt, i));
            switch(sqlite3_column_type(ppStmt, i)) {
               case SQLITE_INTEGER:
                  printf("%10d", sqlite3_column_int(ppStmt, i));
                  break;
               case SQLITE_FLOAT:
                  printf("%10.3f", sqlite3_column_double(ppStmt, i));
                  break;
               case SQLITE3_TEXT:
                  printf("%10s", sqlite3_column_text(ppStmt, i));
                  break;
               case SQLITE_NULL:
                  printf("%10s", "NULL");
                  break;
            }
         }
         printf("\n");
      }

      if(errcode != SQLITE_DONE)
         fprintf(stderr, "Encountered an error while retieving records (%s)\n", sqlite3_errstr(errcode));

      if((errcode = sqlite3_finalize(ppStmt)) != SQLITE_OK)
         fprintf(stderr, "Cannot finalize a prepared statement (%s)\n", sqlite3_errstr(errcode));

      if((errcode = sqlite3_close(ppDb)) != SQLITE_OK)
         fprintf(stderr, "Cannot close the test SQLite database (%s)\n", sqlite3_errstr(errcode));

      if((errcode = sqlite3_shutdown()) != SQLITE_OK)
         fprintf(stderr, "Cannot shut down SQLite (%s)\n", sqlite3_errstr(errcode));

      return EXIT_SUCCESS;
   }
   catch (const std::exception& err) {
      fprintf(stderr, "ERROR: %s\n", err.what());
   }

   if(ppDb) {
      if(sqlite3_close(ppDb) != SQLITE_OK)
         fprintf(stderr, "Failed to close the test SQLite databasen\n");
   }

   return EXIT_FAILURE;
}
