#ifndef _CRC32_H_
#define _CRC32_H_

unsigned long fast_crc32(unsigned long sum, unsigned char *p, unsigned long len);
unsigned long slow_crc32(unsigned long sum, unsigned char *p, unsigned long len);
unsigned long set_crc32_init_value(unsigned long start);
unsigned long crc32_mpeg2_slow(unsigned  long,unsigned char *ptr, unsigned int len);
unsigned long crc32_mpeg2_slow1(unsigned  long,unsigned char *ptr, unsigned int len);
unsigned long crc32_mpeg2_fast(unsigned  long,unsigned char *ptr, unsigned int len);
unsigned int crc32_mpeg2_fast1(unsigned long init,unsigned char *ptr, unsigned int len);

#endif
