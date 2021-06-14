#include <dirent.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void main(int argc, char *argv[]) {
  if (argc < 8) {
    printf("Masukkan argumen yang benar untuk melakukan dump database\n");
    printf("Format: ./[program_dump_database] -u [username] -p [password] \
      [nama_database] > [nama_file_backup]\n");
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
    } else if (!strcmp(argv[i], ">")) {
      strcpy(filename, argv[i + 1]);
    }
  }

  char current_dir[NAME_MAX];
  if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
    perror("Tidak dapat mendapatkan path sekarang ini!");
    exit(EXIT_FAILURE);
  }

  char database_dir[PATH_MAX];
  sprintf(database_dir, "%s/%s", current_dir, database);

  DIR *openned_dir;
  openned_dir = opendir(database_dir);
  if (openned_dir == NULL) {
    perror("Database tidak ditemukan!");
    exit(EXIT_FAILURE);
  }

  char backup_file_dir[PATH_MAX];
  sprintf(backup_file_dir, "%s/%s", current_dir, filename);
  if (!access(backup_file_dir, F_OK)) {
    perror("File backup sudah ada!");
    exit(EXIT_FAILURE);
  }

  FILE * backup_file;
  backup_file = fopen(backup_file_dir, "a");
  if (backup_file == NULL) {
    perror("Tidak dapat membuat file backup!");
    exit(EXIT_FAILURE);
  }

  struct dirent *item;
  while ((item = readdir(openned_dir)) != NULL) {
    if (!strcmp(item->d_name, ".") || !strcmp(item->d_name, "..")) {
      continue;
    }

    char item_dir[PATH_MAX];
    sprintf(item_dir, "%s/%s", current_dir, item->d_name);

    FILE * item_file;
    item_file = fopen(item_dir, "r");
    if (item_file == NULL) {
      perror("Tidak dapat membuka file!");
      continue;
    }

    fprintf(backup_file, "DROP TABLE %s;\n", item->d_name);

    char * text = NULL;
    size_t length = 0;
    bool is_column_name = true;
    // kurang membaca file per table nya
  }

  fclose(backup_file);
  exit(EXIT_SUCCESS);
}
