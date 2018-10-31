#include "stdafx.h"
#include "Direct_Access_Image.h"
#include "Resample.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <algorithm>

#define SIZE_UCHAR		8
#define MAX_CHAR		128
#define MAX_UCHAR		255
#define MASK_1			0x01
#define M_BZ_SMALL		0
#define M_BZ_VERB		0
#define M_BZ_BLK_SIZE	9
#define M_BZ_WORK_FACT	0
#define MIN_IMG_WIDTH	2
#define MIN_IMG_HEIGHT	2

template <typename U, typename T>
void Write(U* buf, T val) {
	*reinterpret_cast<T*>(buf) = val;
}

template <typename T, typename U>
T Read(U* buf) {
	return *reinterpret_cast<T*>(buf);
}

template <typename T>
void TestPrint(T* vec, unsigned int num) {
	for (unsigned int i = 0; i < num; i++) {
		std::cout << std::forward<T>(vec[i]) << "\n";
	}
}

struct BitVector {
private:
	std::vector<unsigned char> vec;
	unsigned int size;
public:
	BitVector() : size(0) {}
	BitVector(std::vector<unsigned char> data, unsigned int size) : 
		size(size),
		vec(data)
	{}

	void Add(unsigned char sign) {
		if (size % SIZE_UCHAR == 0) {
			vec.push_back(0);
		}
		unsigned char& byte = vec.back();
		byte |= (sign << (SIZE_UCHAR - (size % SIZE_UCHAR + 1)));
		size++;
	}

	unsigned char operator[](int index) {
		unsigned int byteIndex = index / SIZE_UCHAR;
		unsigned int offset = index % SIZE_UCHAR;
		return (vec[byteIndex]>> (SIZE_UCHAR - offset - 1)) & MASK_1;
	}

	unsigned int GetSize() {
		return size;
	}

	std::vector<unsigned char>& GetBitVector() {
		return vec;
	}
};

struct Pyramid {
	virtual unsigned char* GetCompressedData() const = 0;
	virtual unsigned int GetCompressedSize() const = 0;
	virtual unsigned int GetUncompressedSize() const = 0;
	virtual unsigned char GetNumLevels() const = 0;
	virtual KImage* GetTopImage() const = 0;
	virtual unsigned int Downsample(unsigned int) const = 0;
	virtual std::pair<unsigned int, unsigned int> GetDims() const = 0;
	virtual ~Pyramid(){}
};

struct ResidualPyramid : public Pyramid {
private:
	unsigned char* data;
	KImage* topImage;
	unsigned int compressedSize;
	unsigned int uncompressedSize;
	unsigned char numLevels;
	std::pair<unsigned int, unsigned int> dims;
public:
	ResidualPyramid() :
		data(nullptr),
		compressedSize(0),
		uncompressedSize(0),
		numLevels(0),
		dims(std::make_pair(0, 0)),
		topImage(nullptr) {}

	ResidualPyramid(unsigned char* d, unsigned int compSize, unsigned int uncompSize, unsigned char nl, 
		std::pair<unsigned int, unsigned int> dims, KImage* topImg) :
		data(d),
		compressedSize(compSize),
		uncompressedSize(uncompSize),
		numLevels(nl),
		dims(dims),
		topImage(topImg) {}

	unsigned char* GetCompressedData() const override {
		return data;
	}
	unsigned int GetCompressedSize() const override {
		return compressedSize;
	}
	unsigned int GetUncompressedSize() const override {
		return uncompressedSize;
	}
	unsigned char GetNumLevels() const override {
		return numLevels;
	}
	unsigned int Downsample(unsigned int dim) const override {
		return unsigned int(dim / 3.0f + 0.5);
	}
	KImage* GetTopImage() const override {
		return topImage;
	}
	std::pair<unsigned int, unsigned int> GetDims() const override {
		return dims;
	}
	~ResidualPyramid() override {
		if (data != nullptr) {
			delete data;
		}
		if (topImage != nullptr) {
			delete topImage;
		}
	}
};

Pyramid* Compress(KImage* image) {
	unsigned char* data;
	KImage* topImageData;
	unsigned int dataSize;
	BitVector signVec;
	std::pair<unsigned int, unsigned int> dims;
	unsigned char numLevels = 0;
	std::vector<unsigned char> tmpData;
	std::vector<unsigned int> header;
	dims = std::make_pair(image->GetWidth(), image->GetHeight());

	int newWidth = image->GetWidth();
	int newHeight = image->GetHeight();
	while (newWidth > MIN_IMG_WIDTH && newHeight > MIN_IMG_HEIGHT) {

		newWidth = int(ResidualPyramid().Downsample(image->GetWidth()));
		newHeight = int(ResidualPyramid().Downsample(image->GetHeight()));

		if (newWidth > MIN_IMG_WIDTH && newHeight > MIN_IMG_HEIGHT) {

			numLevels++;
		
			KImage* downsampledImage = new KImage(newWidth, newHeight, SIZE_UCHAR);
			Resample(image, downsampledImage, FILTER_LANCZOS3);
		
			KImage upsampledImage(image->GetWidth(), image->GetHeight(), SIZE_UCHAR);
			Resample(downsampledImage, &upsampledImage, FILTER_LANCZOS3);

			auto data1 = image->GetDataMatrix();
			auto data2 = upsampledImage.GetDataMatrix();

			for (int i = 0; i < image->GetHeight(); i++) {
				for (int j = 0; j < image->GetWidth(); j++) {
					short diff =  data1[i][j] - data2[i][j];
					if (diff + MAX_CHAR > MAX_UCHAR || diff + MAX_CHAR < 0) {
						header.push_back(tmpData.size());
						tmpData.push_back(std::abs(diff));
						
						unsigned char sign = diff < 0 ? 1 : 0;
						signVec.Add(sign);
						
						continue;
					}
					tmpData.push_back(diff + MAX_CHAR);
				}
			}
			if (image->GetWidth() != dims.first && image->GetHeight() != dims.second) {
				delete image;
			}
			image = downsampledImage;
		}
	}

	topImageData = image;
	dataSize = sizeof(unsigned int) + 
		header.size() * sizeof(unsigned int) + signVec.GetBitVector().size() + tmpData.size();
	data = new unsigned char[dataSize];

	Write(data, header.size());
		
	unsigned int offset = sizeof(unsigned int);
	for (unsigned int i = 0; i < header.size(); i++) {
		Write(data + offset, header[i]);
		offset += sizeof(unsigned int);
	}

	for (auto el : signVec.GetBitVector()) {
		Write(data + offset, el);
		offset += sizeof(unsigned char);
	}

	for (unsigned int i = 0; i < tmpData.size(); i++) {
		Write(data + offset, tmpData[i]);
		offset += sizeof(unsigned char);
	}

	char* dest = new char[dataSize];
	unsigned int destLen = dataSize;
	auto ret = BZ2_bzBuffToBuffCompress(
		dest,
		&destLen,
		(char*)data,
		dataSize,
		M_BZ_BLK_SIZE,
		M_BZ_VERB,
		M_BZ_WORK_FACT
	);
	if (ret == BZ_OK) {
		std::cout << "BZIP2 COMPRESSION OK\n";
	}

	return new ResidualPyramid((unsigned char*)dest, destLen, dataSize, numLevels, dims, image);
}

KImage* Decompress(Pyramid* residual) {
	unsigned int sourceLen = residual->GetCompressedSize();
	unsigned int destLen = residual->GetUncompressedSize();
	char* dest = new char[destLen];

	auto ret = BZ2_bzBuffToBuffDecompress(
		dest,
		&destLen,
		(char*)residual->GetCompressedData(),
		sourceLen,
		M_BZ_SMALL,
		M_BZ_VERB
	);

	if (ret == BZ_OK) {
		std::cout << "BZIP2 DECOMPRESSION OK" << "\n\n";
	}

	unsigned char* data = (unsigned char*) dest;
	unsigned int size = destLen;
	auto dims = residual->GetDims();
	unsigned int numPositions = Read<unsigned int>(data);
	unsigned int offset = sizeof(unsigned int);

	std::vector<unsigned int> positions(numPositions);
	if (numPositions != 0) {
		std::memcpy(&positions[0], data + offset, numPositions * sizeof(unsigned int));
		offset += (numPositions * sizeof(unsigned int));
	}

	unsigned int numSigns = std::ceil(numPositions / float(SIZE_UCHAR));
	std::vector<unsigned char> signs(numSigns);
	if (numSigns != 0) {
		std::memcpy(&signs[0], data + offset, numSigns);
		offset += (numSigns * sizeof(unsigned char));
	}

	std::vector<short> res;
	for (unsigned int i = offset; i < size; i++) {
		
		res.push_back((short)Read<unsigned char>(data + i) - MAX_CHAR);
	}

	unsigned int index = 0;
	BitVector bv(signs, numPositions);
	for (auto el : positions) {
		res[el] += MAX_CHAR;
		res[el] = bv[index++] == 0 ? res[el] : -res[el];
	}

	std::vector<std::pair<unsigned int, unsigned int>> dimVec;
	for (unsigned int i = 0; i < residual->GetNumLevels(); i++) {
		dimVec.push_back(dims);
		dims = std::make_pair(residual->Downsample(dims.first), 
			residual->Downsample(dims.second));
	}

	std::reverse(dimVec.begin(), dimVec.end());

	offset = res.size();
	KImage* pImage = residual->GetTopImage();
	for (unsigned int di = 0; di < dimVec.size(); di++) {
		auto dim = dimVec[di];
		offset -= (dim.first * dim.second);
		KImage* upsampledImage = new KImage(dim.first, dim.second, SIZE_UCHAR);
		Resample(pImage, upsampledImage, FILTER_LANCZOS3);
		auto data = upsampledImage->GetDataMatrix();
		for (int i = 0; i < upsampledImage->GetHeight(); i++) {
			for (int j = 0; j < upsampledImage->GetWidth(); j++) {
				data[i][j] += res[offset + i * upsampledImage->GetWidth() + j];
			}
		}
		delete pImage;
		pImage = upsampledImage;
	}

	return pImage;
}

void WriteCompressed(Pyramid* p, const std::wstring& file) {
	auto compressedDataSize = p->GetCompressedSize();
	auto uncompressedDataSize = p->GetUncompressedSize();
	auto compressedData = p->GetCompressedData();
	auto dimsOrig = p->GetDims();
	auto dimsTop = std::make_pair(p->GetTopImage()->GetWidth(), p->GetTopImage()->GetHeight());
	auto numLevels = p->GetNumLevels();
	auto topImageData = p->GetTopImage()->GetDataMatrix();

	std::vector<unsigned char> vec;
	for (unsigned int i = 0; i < p->GetTopImage()->GetHeight(); i++) {
		for (unsigned int j = 0; j < p->GetTopImage()->GetWidth(); j++) {
			vec.push_back(topImageData[i][j]);
		}
	}

	std::ofstream out(file, std::ios::binary);
	out.write("PYR", 3 * sizeof(unsigned char));
	out.write((char*)(&dimsOrig.first), sizeof(unsigned int));
	out.write((char*)(&dimsOrig.second), sizeof(unsigned int));
	out.write((char*)(&numLevels), sizeof(unsigned char));
	out.write((char*)(&compressedDataSize), sizeof(unsigned int));
	out.write((char*)(&uncompressedDataSize), sizeof(unsigned int));
	out.write((char*)(&dimsTop.first), sizeof(unsigned int));
	out.write((char*)(&dimsTop.second), sizeof(unsigned int));
	out.write((char*)(&vec[0]), vec.size() * sizeof(unsigned char));
	out.write((char*)compressedData, compressedDataSize * sizeof(unsigned char));
	out.close();
}

Pyramid* ReadCompressed(const std::wstring& file) {
	unsigned char ident[3];
	unsigned int compressedDataSize;
	unsigned int uncompressedDataSize;
	unsigned char* compressedData;
	unsigned char* topData;
	unsigned char numLevels;
	std::pair<unsigned int, unsigned int> dimsOrig;
	std::pair<unsigned int, unsigned int> dimsTop;

	std::ifstream in(file, std::ios::binary);
	in.read((char*)(ident), 3 * sizeof(unsigned char));
	in.read((char*)(&dimsOrig.first), sizeof(unsigned int));
	in.read((char*)(&dimsOrig.second), sizeof(unsigned int));
	in.read((char*)(&numLevels), sizeof(unsigned char));
	in.read((char*)(&compressedDataSize), sizeof(unsigned int));
	in.read((char*)(&uncompressedDataSize), sizeof(unsigned int));
	in.read((char*)(&dimsTop.first), sizeof(unsigned int));
	in.read((char*)(&dimsTop.second), sizeof(unsigned int));
	topData = new unsigned char[dimsTop.first * dimsTop.second];
	compressedData = new unsigned char[compressedDataSize];
	in.read((char*)topData, dimsTop.first * dimsTop.second * sizeof(unsigned char));
	in.read((char*)compressedData, compressedDataSize * sizeof(unsigned char));
	in.close();

	KImage* topImg = new KImage(dimsTop.first, dimsTop.second, SIZE_UCHAR);
	for (unsigned int i = 0; i < dimsTop.second; i++) {
		for (unsigned int j = 0; j < dimsTop.first; j++) { 
			topImg->GetDataMatrix()[i][j] = topData[i * dimsTop.first + j];
		}
	}

	return new ResidualPyramid(compressedData, compressedDataSize, 
		uncompressedDataSize, numLevels, dimsOrig, topImg);
}

void TestPrintFile(unsigned char* d, unsigned int size, const std::string& file) {
	std::ofstream out(file);
	for (unsigned int i = 0; i < size; i++) {
		out << (int)d[i] << "\n";
	}
	out.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 4)
	{
		_tprintf(_T("Invalid program usage, correct syntax is: %s <Input Folder> <Output Folder Compressed> <Output Folder Deompressed> <CR>!\n"), argv[0]);
		getchar();
		return -1;
	}

	TCHAR szInputPath[_MAX_PATH], szOutputComp[_MAX_PATH], szFileMask[_MAX_PATH], szFileName[_MAX_PATH], szOutputDecomp[_MAX_PATH];
	_tcscpy_s(szInputPath, argv[1]);
	_tcscpy_s(szOutputComp, argv[2]);
	_tcscpy_s(szOutputDecomp, argv[3]);
	_stprintf_s(szFileMask, _MAX_PATH, _T("%s\\*.TIF"), szInputPath);
	_tfinddata_t FindData;
	intptr_t handleI, handleJ;
	for (handleI = handleJ = _tfindfirst(szFileMask, &FindData); handleI != -1; handleI = _tfindnext(handleJ, &FindData)) {
		if ((FindData.attrib & _A_SUBDIR) != 0)
			continue;

		_stprintf_s(szFileName, _MAX_PATH, _T("%s\\%s"), szInputPath, FindData.name);
		KImage *pImage = new KImage(szFileName);
		if (!pImage->IsValid() || pImage->GetBPP() != SIZE_UCHAR)
		{
			delete pImage;
			continue;
		}
		
		std::wcout << "Current image: " << std::wstring(szFileName) << "\n";

		Pyramid* p = Compress(pImage);
		std::wstring inName(szFileName);
		std::wstring outComp(std::wstring(szOutputComp) + 
			inName.substr(7, inName.size() - 10) + std::wstring(_T("pyr")));
		std::wstring outDecomp(std::wstring(szOutputDecomp) + 
			inName.substr(7, inName.size()));
		WriteCompressed(p, outComp);

		auto compFromFile = ReadCompressed(outComp);
		auto decomp = Decompress(compFromFile);
		decomp->SaveAs(outDecomp.c_str());

		delete pImage;
		delete p;
		delete decomp;
	}
	_findclose(handleJ);

	return 0;
}