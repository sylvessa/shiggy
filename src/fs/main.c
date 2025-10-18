#include "globals.h"

int findex_end = 0;
File* findex[FS_MAX_FILE_COUNT];

Sector* init_sector() {
    Sector* fs = malloc(sizeof(Sector));
    fs->next = END_SECTOR;
    for (unsigned int i = 0; i < FS_SECTOR_DATA_SIZE; ++i) {
        // delete potentional data in the sector
        fs->data[i] = 0;
    }
    return fs;
}

int file_count() {
    return findex_end;
}

char* file_get_name(int id) {
    if (id >= 0 && id < findex_end && findex[id] != NULL)
        return findex[id]->name;
    return NULL;
}

int file_make(char* name) {
    // todo: check for validity

    // alloc
    File* fp = malloc(sizeof(File));
    strcpy(fp->name, name);
    fp->first_sector = init_sector();

    findex[findex_end] = fp;
    findex_end += 1;

    return OK;
}

void fs_init() {
    file_make("fuck.txt");
    file_make("fuc2k.txt");
}