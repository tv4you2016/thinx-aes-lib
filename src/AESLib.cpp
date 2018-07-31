#include "AESLib.h"

uint8_t AESLib::getrnd()
{
   uint8_t really_random = *(volatile uint8_t *)0x3FF20E44;
   return really_random;
}

void AESLib::gen_iv(byte  *iv) {
    for (int i = 0 ; i < N_BLOCK ; i++ ) {
        iv[i]= (byte) 0xFF; // getrnd();
    }
}

String AESLib::decrypt(String msg, byte key[], byte my_iv[]) {

  aes.set_key(key, sizeof(key));

  // Base64-decode message to `encrypted` array which stores the ciphertext
  int len = msg.length();
  Serial.printf("Decrypting message %s of length %i \n", msg.c_str(), len);
  char encrypted[len]; // will be always shorter than Base64
  int b64len = base64_decode(encrypted, (char*)msg.c_str(), msg.length());

  // printf("Decoded %i bytes as %s \n", b64len, encrypted);

  // AES decrypt into calculated and allocated working buffer
  byte out[2*len];
  aes.do_aes_decrypt((byte *)encrypted, b64len, out, key, 128, (byte *)my_iv);

  // Calculate and allocate required Base64 buffer size
  int outDataLen = strlen((char*)out);
  int outLen = base64_dec_len((char*)out, outDataLen);
  char message[outLen+2]; // trailing zero for cstring?

  // Finally Base64-decode the decrypted message and make it a C-strings
  int baseLen = base64_decode(message, (char *)out, outDataLen);
  message[baseLen] = '\0'; // ensure trailing zero after cstring
  // printf("Out o:%i/b:%i bytes: %s \n", outDataLen, baseLen, message);

  // clean();

  return String(message);
}

/* Suggested size for the plaintext buffer is 1/2 length of `msg` */
void AESLib::decrypt64(char * msg, char * plain, byte key[], byte my_iv[]) {

  aes.set_key(key, sizeof(key));

  // Base64-decode message to `encrypted` array which stores the ciphertext
  int msgLen = strlen(msg);
  Serial.printf("Decrypting message of length %i \n", msgLen);
  char encrypted[msgLen]; // will be always shorter than Base64
  int b64len = base64_decode(encrypted, msg, msgLen);

  // printf("Decoded %i bytes as %s \n", b64len, encrypted);

  // AES decrypt into calculated and allocated working buffer
  byte out[2*msgLen];
  aes.do_aes_decrypt((byte *)encrypted, b64len, out, key, 128, (byte *)my_iv);

  // Calculate and allocate required Base64 buffer size
  int outDataLen = strlen((char*)out);
  int outLen = base64_dec_len((char*)out, outDataLen);
  char message[outLen+2]; // trailing zero for cstring?

  // Finally Base64-decode the decrypted message and make it a C-strings
  int baseLen = base64_decode(message, (char *)out, outDataLen);
  message[baseLen] = '\0'; // ensure trailing zero after cstring

  //clean();

  // Copy from working to plaintext buffer, may deprecate to save RAM.
  strcpy(plain, message);
  //clean();
}

/* Returns byte array decoded and decrypted. */
void AESLib::decrypt(char * msg, char * plain, byte key[], byte my_iv[]) {

  aes.set_key(key, sizeof(key));

  // Base64-decode message to `encrypted` array which stores the ciphertext
  int msgLen = strlen(msg);
  Serial.printf("Decrypting message %s of length %i \n", msg, msgLen);
  char encrypted[msgLen]; // will be always shorter than Base64
  int b64len = base64_decode(encrypted, msg, msgLen);

  // printf("Decoded %i bytes as %s \n", b64len, encrypted);

  // AES decrypt into calculated and allocated working buffer
  byte out[2*msgLen];
  aes.do_aes_decrypt((byte *)encrypted, b64len, out, key, 128, (byte *)my_iv);

  // Calculate and allocate required Base64 buffer size
  int outDataLen = strlen((char*)out);
  int outLen = base64_dec_len((char*)out, outDataLen);
  char message[outLen+2]; // trailing zero for cstring?

  strcpy(plain, message);
}

String AESLib::encrypt(String msg, byte key[], byte my_iv[]) {

  aes.set_key(key, sizeof(key));

  // Calculate required length and pad the plaintext for 16bit AES
  int msgLen = strlen(msg.c_str());

  // Encode data before encryption
  char b64data[base64_enc_len(msgLen)];
  int b64len = base64_encode(b64data, (char*)msg.c_str(), msgLen);

  // Add PKCS7 padding
  int paddedLen = b64len + (N_BLOCK - (b64len % N_BLOCK)) + 1;
  byte padded[paddedLen];
  aes.padPlaintext(b64data, padded);

  // Encrypt using AES 128bit
  char out[b64len];
  byte cipher[2*b64len];
  aes.do_aes_encrypt((byte *)padded, paddedLen, cipher, key, 128, my_iv);

  // Encode data to Base64 so it can be returned as String (or written to char*)
  base64_encode(out, (char *)cipher, aes.get_size() );

  return String((char*)out);
}

/* Returns message encrypted and base64 encoded to be used as string. */
void AESLib::encrypt64(char * msg, char * output, byte key[], byte my_iv[]) {

  aes.set_key(key, sizeof(key));

  // Calculate required length and pad the plaintext for 16bit AES
  int msgLen = strlen(msg);

  // Encode data before encryption
  char b64data[base64_enc_len(msgLen)];
  int b64len = base64_encode(b64data, (char*)msg, msgLen);

  // Add PKCS7 padding
  int paddedLen = b64len + (N_BLOCK - (b64len % N_BLOCK)) + 1;
  byte padded[paddedLen];
  aes.padPlaintext(b64data, padded);

  // Encrypt using AES 128bit
  byte cipher[2*b64len];
  aes.do_aes_encrypt((byte *)padded, paddedLen, cipher, key, 128, my_iv);

  // Encode data to Base64 so it can be returned as String (or written to char*)
  char out2[4*b64len];
  base64_encode(out2, (char *)cipher, aes.get_size() );

  strcpy(output, (char*)out2);
}

/* Returns message encrypted only to be used as byte array. */
void AESLib::encrypt(char * msg, char * output, byte key[], byte my_iv[]) {

  aes.set_key(key, sizeof(key));

  // Calculate required length and pad the plaintext for 16bit AES
  int msgLen = strlen(msg);

  // Encode data before encryption
  char b64data[base64_enc_len(msgLen)];
  int b64len = base64_encode(b64data, (char*)msg, msgLen);

  // Add PKCS7 padding
  int paddedLen = b64len + (N_BLOCK - (b64len % N_BLOCK)) + 1;
  byte padded[paddedLen];
  aes.padPlaintext(b64data, padded);

  // Encrypt using AES 128bit
  byte cipher[2*b64len];
  aes.do_aes_encrypt((byte *)padded, paddedLen, cipher, key, 128, my_iv);

  strcpy(output, (char*)cipher);
}

void AESLib::clean() {
  aes.clean();
}
