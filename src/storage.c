void storage_prepare(){
    unsigned long exist_filesize = get_mmap_filesize(storage_path);
    unsigned long size = exist_filesize == 0 ? calculate_mmap_size() : exist_filesize;
    mmap_fd = init_mmap_file(size, storage_path);
    mmap_ptr = (B*)mmap(0,size,PROT_READ|PROT_WRITE, MAP_PRIVATE, mmap_fd, 0); 

    // scan max storage_index
    unsigned long long i;for(i=0;i<storage_length;i++){
        B* b = &(mmap_ptr[i]);
        if(b->status == B_SET) {
            cached_keys_append(b->key,i);
        }
        else if (b->status == B_EMPTY) { storage_index = i; break; }
    }
}

void storage_extend(){
    unsigned long old_size = calculate_mmap_size();
    munmap(mmap_ptr,old_size);
    storage_length += 1000000;
    unsigned long size = calculate_mmap_size();
    extend_mmap_file(mmap_fd,size);
    mmap_ptr = (B*)mmap(0,size,PROT_READ|PROT_WRITE, MAP_SHARED, mmap_fd, 0); 
}

void storage_set(const char* key, const char* val) {
    int idx = cached_keys_find(key);
    if (idx != -1) {
        B* b = &(mmap_ptr[idx]);
        strcpy(b->val,val);
        msync(b,sizeof(B),MS_ASYNC);
    } else {
        if (storage_index + 1 > storage_length) {
            storage_extend();
        }
        B* b = &(mmap_ptr[storage_index]);
        strcpy(b->key,key);
        strcpy(b->val,val);
        b->status = B_SET;
        msync(b,sizeof(B),MS_ASYNC);
        cached_keys_append(key,storage_index);
        storage_index++;
    }
}
char* storage_get(const char* key) {
    int idx = cached_keys_find(key);
    if (idx != -1) {
        B* b = &(mmap_ptr[idx]);
        b->status = B_DEL;
        msync(b,sizeof(B),MS_ASYNC);
        cached_keys_remove(key);
        return b->val;
    }
    return NULL;
}
