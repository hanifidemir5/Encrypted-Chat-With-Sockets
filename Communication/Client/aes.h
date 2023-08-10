#ifndef AES_H
#define AES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define AES_BLOCK_SIZE 16
#define EXPANDED_KEY_SIZE 176
#define NONCE_SIZE 8

void randomArray(unsigned char* arr, int n);
int input(char str[], int n);
unsigned char galois_multiplication(unsigned char a, unsigned char b);
void mixColumns(unsigned char* state);
void mixColumn(unsigned char* column);
void invMixColumns(unsigned char* state);
void invMixColumn(unsigned char* column);
void aes_main(unsigned char* expandedKey, int numberOfRounds, unsigned char* state);
void aes_invMain(unsigned char* expandedKey, int numberOfRounds, unsigned char* state);
void aes_encrypt(unsigned char* input, unsigned char* output, unsigned char* expandedKey, int size, int expandedKeySize);
void aes_decrypt(unsigned char* input, unsigned char* output, unsigned char* expandedKey, int size, int expandedKeySize);
void core(unsigned char* word, int number);

#endif /* AES_H */
