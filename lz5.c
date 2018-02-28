// blatantly "borrowed" from https://github.com/bmarquismarkail/SFFv2

#include "prism/lz5.h"

static void naivememcpy(uint8_t* dst, uint32_t *dstpos, uint32_t ofs, uint32_t len)
{
	int run;
	for (run = len; run > 0; run--)
	{
		dst[*dstpos] = dst[((*dstpos) - ofs)];
		(*dstpos)++;
	}
}


static void copyLZ(uint8_t* dst, uint32_t* dstpos, uint32_t ofs, uint32_t len)
{
	naivememcpy(dst, dstpos, ofs, len);
}

static uint32_t processSLZ(uint8_t* dst, const uint8_t* src, uint32_t* dstpos, uint32_t* srcpos, uint8_t *recyclebyte, uint8_t* recyclecount)
{
	//Process a short LZ packet.
	(*recyclebyte) |= ((src[*srcpos] & 0xC0) >> ((*recyclecount) * 2));
	(*recyclecount)++;
	//the answer used to determine the short packet is the copy length.
	//check and see if this is the forth short LZ packet being processed.
	if (*recyclecount == 4)
	{
		//if so, the recycle buffer has the offset of decompressed data for copying.
		copyLZ(dst, dstpos, ((*recyclebyte) + 1), (src[*srcpos] & 0x3F) + 1);
		(*srcpos)++;
		(*recyclecount) = 0;
		(*recyclebyte) = 0;
	}
	//else read another byte. that is the offset of decompressed data for copying.
	else
	{
		copyLZ(dst, dstpos, (src[*srcpos + 1] + 1), (src[*srcpos] & 0x3F) + 1);
		(*srcpos) += 2;
	}
	return 0;
}

static uint32_t processLLZ(uint8_t* dst, const uint8_t* src, uint32_t* dstpos, uint32_t *srcpos)
{
	//Process a long LZ packet.
	//the byte read before ANDed by 0xC0 is the top 2 bits of the offset
	//read another byte. That is the bottom 8 bits of the offset.
	uint16_t offset = ((src[*srcpos] & 0xC0) << 2) | (src[*srcpos + 1]);
	//read one more byte. That is the copy length.
	//Now copy using the offset - 3
	copyLZ(dst, dstpos, offset + 1, (src[*srcpos + 2]) + 3);
	(*srcpos) += 3;
	return 0;
}

static uint32_t processLZ(uint8_t* dst, uint8_t* src, uint32_t* dstpos, uint32_t *srcpos, uint8_t* recyclebyte, uint8_t* recyclecount)
{

	//Process an LZ Packet by ANDing the first byte by 0x3F.
	if (src[*srcpos] & 0x3F)
		//if that equation is nonzero, it is a short LZ packet
		processSLZ(dst, src, dstpos, srcpos, recyclebyte, recyclecount);
	//else it is a long LZ packet.
	else processLLZ(dst, src, dstpos, srcpos);

	return 0;
}




static uint32_t processSRLE(uint8_t* dst, uint8_t* src, uint32_t* dstpos, uint32_t* srcpos)
{
	int run;
	for (run = 0; run < (src[*srcpos] >> 5); run++)
	{
		dst[*dstpos] = (src[*srcpos] & 0x1F);
		(*dstpos)++;
	}
	(*srcpos)++;
	return 0;
}

static uint32_t processLRLE(uint8_t* dst, uint8_t* src, uint32_t* dstpos, uint32_t* srcpos)
{
	int run;
	for (run = 0; run < (src[(*srcpos) + 1] + 8); run++)
	{
		dst[*dstpos] = (src[*srcpos] & 0x1F);
		(*dstpos)++;
	}
	(*srcpos) += 2;
	return 0;
}

static uint32_t processRLE(uint8_t* dst, uint8_t* src, uint32_t *dstpos, uint32_t *srcpos)
{

	//Process an RLE Packet by ANDing the first byte by 0xC0 and checking if it's 0
	if (src[*srcpos] & 0xE0)
		processSRLE(dst, src, dstpos, srcpos);
	else processLRLE(dst, src, dstpos, srcpos);
	return 0;
}

void decompressLZ5(uint8_t* tDst, uint8_t* tSrc, uint32_t tSourceLength)
{
	uint32_t dstpos = 0;
	uint32_t srcpos = 0;
	uint8_t  recbyt = 0;
	uint8_t  reccount = 0;

	while (srcpos < tSourceLength)
	{
		//read a control packet and process it bit by bit
		//if the bit is 0, it is an RLE Packet, otherwise it is an LZ packet.
		uint8_t ctrlbyt = tSrc[srcpos];
		srcpos++;
		int b;
		for (b = 0; (b < 8); b++)
		{
			if (!(ctrlbyt & (1 << b)))
				processRLE(tDst, tSrc, &dstpos, &srcpos);
			else processLZ(tDst, tSrc, &dstpos, &srcpos, &recbyt, &reccount);
			if (srcpos >= tSourceLength) break;
		}
	}

}
