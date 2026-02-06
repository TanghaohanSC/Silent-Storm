#ifndef __MMPFORMAT_H__
#define __MMPFORMAT_H__
////////////////////////////////////////////////////////////////////////////////////////////////////
const DWORD MMP_SIGNATURE = 0x00504D4D;
#pragma pack( 4 )
struct SMMPFileHeader
{
	DWORD dwSignature;										// .mmp file signature
	NGfx::EPixelFormat format;						// format of this image
	DWORD dwAverageColor;
	int nSizeX;
	int nSizeY;
	// NOTE: 'dwLinearSize' and 'dwNumMipLevels' are mutually exclusive by the usage
	union
	{
		int nNumMipLevels;									// number of mip-map levels in the image
		int nLinearSize;										// linear size of RLE-compresed formats. 
	};
	//
	SMMPFileHeader() { memset(this, 0, sizeof(*this)); dwSignature = MMP_SIGNATURE; }
};
#pragma pack()
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // __MMPFORMAT_H__