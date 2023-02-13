
#ifndef _HISTOGRAM_H_
#define	_HISTOGRAM_H_

#define GRAY_LEVEL_NUMBER	256

void getHistogram(unsigned char* pbBuffer, int nBufferSize, int* pnHistogram);
void HistogramEqualizer(unsigned char* pbSrcBuffer, unsigned char* pbDesBuffer, int nBufferSize);

#endif