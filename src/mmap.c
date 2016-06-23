void extend_mmap_file(int fd,const unsigned long size){
    char c = '\0';
    lseek(fd, size-1, SEEK_SET);
    write(fd, &c, sizeof(char));
    lseek(fd, 0, SEEK_SET);
}
unsigned long get_mmap_filesize(const char* filename){
    if (access( filename, F_OK )==-1)return 0;
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}
int init_mmap_file(const unsigned long size, const char* datapath){
    int fd = open(datapath, O_RDWR|O_CREAT,0666);
    extend_mmap_file(fd,size);
    return fd;
}
unsigned long calculate_mmap_size(){
    unsigned long pagesize = sysconf(_SC_PAGE_SIZE);
    unsigned long size = (storage_length * sizeof(B) / pagesize + 2) * pagesize;
    return size;
}
