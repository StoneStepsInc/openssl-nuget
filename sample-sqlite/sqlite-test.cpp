#include <sqlite3.h>

#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <string_view>

using namespace std::literals::string_view_literals;

int count_results(void *ptr, int ncol, char **values, char **names)
{
   (*reinterpret_cast<int*>(ptr))++;

   return 0;
}

int main(void)
{
   sqlite3 *ppDb = nullptr;

   try {
      char *errmsg = nullptr;

      if(sqlite3_open("sqlite-test.db", &ppDb) != SQLITE_OK)
         throw std::runtime_error("Cannot open the test SQLite database");

      int result_count = 0;

      if(sqlite3_exec(ppDb, "select name from sqlite_schema where type='table' and name='test_table';", count_results, &result_count, &errmsg) != SQLITE_OK)
         throw std::runtime_error(errmsg);

      if(result_count == 0) {
         if(sqlite3_exec(ppDb, "create table test_table (txt TEXT);", nullptr, nullptr, &errmsg) != SQLITE_OK)
            throw std::runtime_error(errmsg);
      }

      if(sqlite3_exec(ppDb, "insert into test_table (txt) values ('abc');", nullptr, nullptr, &errmsg) != SQLITE_OK)
         throw std::runtime_error(errmsg);

      if(sqlite3_exec(ppDb, "insert into test_table (txt) values ('xyz');", nullptr, nullptr, &errmsg) != SQLITE_OK)
         throw std::runtime_error(errmsg);

      // rowid is not included in the default list of selected columns
      std::string_view sql = "select rowid, * from test_table order by rowid desc;"sv;

      int errcode = SQLITE_OK;
      sqlite3_stmt *ppStmt = nullptr;

      if((errcode = sqlite3_prepare_v2(ppDb, sql.data(), (int) sql.length()+1, &ppStmt, nullptr)) != SQLITE_OK)
         throw std::runtime_error(sqlite3_errstr(errcode));

      while(sqlite3_step(ppStmt) != SQLITE_DONE) {
         for(int i = 0; i < sqlite3_column_count(ppStmt); i++) {
            printf("%10s: ", sqlite3_column_name(ppStmt, i));
            switch(sqlite3_column_type(ppStmt, i)) {
               case SQLITE_INTEGER:
                  printf("%d", sqlite3_column_int(ppStmt, i));
                  break;
               case SQLITE_FLOAT:
                  printf("%f", sqlite3_column_double(ppStmt, i));
                  break;
               case SQLITE_TEXT:
                  printf("%s", sqlite3_column_text(ppStmt, i));
                  break;
               case SQLITE_NULL:
                  printf("NULL");
                  break;
            }
         }
         printf("\n");
      }

      if(sqlite3_finalize(ppStmt) != SQLITE_OK)
         throw std::runtime_error(errmsg);

      if(sqlite3_close(ppDb) != SQLITE_OK)
         fprintf(stderr, "Failed to close the test SQLite database");

      return EXIT_SUCCESS;
   }
   catch (const std::exception& err) {
      fprintf(stderr, "ERROR: %s\n", err.what());
   }

   if(ppDb) {
      if(sqlite3_close(ppDb) != SQLITE_OK)
         fprintf(stderr, "Failed to close the test SQLite database");
   }

   return EXIT_FAILURE;
}
