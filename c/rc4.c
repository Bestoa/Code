#include <stdio.h>
#include <string.h>

struct crypto_info {
    int box[256];
    int i;
    int j;
};

int create_box(struct crypto_info *info, char *key) {
    int i = 0;
    int j = 0;
    int k = strlen(key);
    int t;
    for (i = 0; i < 256; i++) {
        info->box[i] = i;
    }
    for (i = 0; i < 256; i++) {
        j = (j + info->box[i] + (unsigned int)key[i % k]) % 256;
        int t = info->box[i];
        info->box[i] = info->box[j];
        info->box[j] = t;
    }
    info->i = 0;
    info->j = 0;
    return 0;
}

int stream(struct crypto_info *info, char *src, char *dst, int len) {
    int i, j, k;
    int t;
    i = info->i;
    j = info->j;
    for (k = 0; k < len; k++) {
        i = (i + 1) % 256;
        j = (j + info->box[i]) % 256;
        t = info->box[i];
        info->box[i] = info->box[j];
        info->box[j] = t;
        dst[k] = src[k] ^ info->box[(info->box[i] + info->box[j]) % 256] % 256;
    }
    info->i = i;
    info->j = j;
}

#define BUFFER_LEN (8192)
int main() {
    FILE *in, *out;
    struct crypto_info info;
    char src[BUFFER_LEN], dst[BUFFER_LEN];
    int len = 0;
    create_box(&info, "mohanson");
    in = fopen("src", "rb");
    out = fopen("dst", "wb");
    while (!feof(in)) {
        len = fread(src, 1, BUFFER_LEN, in);
        stream(&info, src, dst, len);
        fwrite(dst, 1, len, out);
    }
    return 0;
}
