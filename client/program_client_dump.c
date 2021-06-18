#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if (argc < 6) {
    printf("Masukkan argumen yang benar untuk melakukan dump database\n");
    printf("Format: ./[program_dump_database] -u [username] -p [password] [nama_database] > [nama_file_backup]\n");
    return EXIT_FAILURE;
  }

  char username[NAME_MAX];
  char password[NAME_MAX];
  char database[NAME_MAX];
  char filename[NAME_MAX];
  for (int i = 0; i < argc; i++) {
    if (!strcmp(argv[i], "-u")) {
      strcpy(username, argv[i + 1]);
    } else if (!strcmp(argv[i], "-p")) {
      strcpy(password, argv[i + 1]);
      strcpy(database, argv[i + 2]);
    }
  }

  char current_dir[NAME_MAX];
  if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
    perror("Tidak dapat mendapatkan path sekarang ini!");
    return EXIT_FAILURE;
  }

  char database_dir[PATH_MAX];
  sprintf(database_dir, "%s/%s", current_dir, database);

  DIR *openned_dir;
  openned_dir = opendir(database_dir);
  if (openned_dir == NULL) {
    perror("Database tidak ditemukan!");
    return EXIT_FAILURE;
  }

  struct dirent *item;
  while ((item = readdir(openned_dir)) != NULL) {
    if (!strcmp(item->d_name, ".") || !strcmp(item->d_name, "..")) {
      continue;
    }

    char item_dir[PATH_MAX];
    sprintf(item_dir, "%s/%s", database_dir, item->d_name);

    FILE *item_file;
    item_file = fopen(item_dir, "r");
    if (item_file == NULL) {
      perror("Tidak dapat membuka file!");
      continue;
    }

    printf("DROP TABLE %s;\n", item->d_name);

    char *txt = NULL;
    size_t len = 0;
    ssize_t txt_len;
    bool is_column_name = true;
    
    int col_counter = 0;
    bool is_col_num[50];
    while ((txt_len = getline(&txt, &len, item_file)) != -1) {
      char query[PATH_MAX];

      if (is_column_name) {
        sprintf(query, "CREATE TABLE %s (", item->d_name);

        char buff[NAME_MAX];
        strcpy(buff, txt);

        char column[50];
        char *substr = strtok(buff, ";");
        while(substr != NULL) {
          for (int i = 0; i < strlen(substr); i++) {
            if (i == strlen(substr) - 1) {
              column[i] = '\0';
              strcat(query, column);

              if (substr[i] == 'i') {
                strcat(query, " int");
                is_col_num[col_counter] = true;
              } else if (substr[i] == 's') {
                strcat(query, " string");
                is_col_num[col_counter] = false;
              }

              col_counter++;
            } else {
              column[i] = substr[i];
            }
          }

          substr = strtok(NULL, ";");
          if (substr != NULL && strlen(substr)) {
            strcat(query, ", ");
          } else {
            col_counter--;
            query[strlen(query) - strlen(", ")] = '\0';
            strcat(query, ");\n");
          }
        }

        is_column_name = false;
      } else {
        sprintf(query, "INSERT INTO %s (", item->d_name);

        char buff[NAME_MAX];
        strcpy(buff, txt);

        int val_counter = 0;
        int char_counter = 0;
        char val[NAME_MAX];
        for (int i = 0; i < strlen(buff); i++) {
          if (buff[i] != ';') {
            val[char_counter] = buff[i];
          } else if (buff[i] == ';') {
            if (buff[i - 1] == '\\') {
              val[char_counter] = buff[i];
            } else {
              char data[NAME_MAX];
              strcpy(data, val);
              data[char_counter] = '\0';

              if (is_col_num[val_counter]) {
                strcat(query, data);
              } else {
                strcat(query, "'");
                strcat(query, data);
                strcat(query, "'");
              }

              val_counter++;
              if (val_counter == col_counter) {
                strcat(query, ");");
              } else {
                strcat(query, ", ");
                char_counter = -1;
              }
            }
          }

          char_counter++;
        }
      }

      printf("%s\n", query);
    }
    printf("\n");

    fclose(item_file);
    if (txt) {
      free(txt);
    }
  }

  return EXIT_SUCCESS;
}
