#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<string>
#include<Windows.h>
#include<vector>


std::vector<std::pair<int, std::pair<int, int > > > vctParseRelocation;
typedef struct Section
{
	char Name[8];
	int VirtualSize;
	int RVA;
	int SizeOfRawData;
	int PoitnerToRawData;
	int POinterToRelocations;
	int PointerToLineNumber;
	WORD NumberOfRelocations;
	WORD NumberOfLineNumbers;
	int Characteristics;
	int TempOffset;
}Section;


int main()
{
	FILE* fp = fopen("MainFunctionOriginal64.exe", "rb");

	if(fp)
	{
		fseek(fp, 0, SEEK_END);
		size_t stSize = ftell(fp);

		int i32FLSize = 0x10000;

		int i32AdditionalOffset = 0;

		char* buf = new char[stSize + i32FLSize];

		fseek(fp, 0, SEEK_SET);
		fread(buf, stSize, 1, fp);

		fclose(fp);
		fp = fopen(R"(MainFunction64.exe)", "wb");
		fseek(fp, 0, SEEK_SET);


		PIMAGE_DOS_HEADER pDosH;
		PIMAGE_NT_HEADERS64 pNtH;
		PIMAGE_SECTION_HEADER pSecH;

		pDosH = (PIMAGE_DOS_HEADER)buf;
		pNtH = (PIMAGE_NT_HEADERS64)((LPBYTE)buf + pDosH->e_lfanew);

		int64_t i64FileBaseAddress = pNtH->OptionalHeader.ImageBase;
		int32_t i32EntryPoint = pNtH->OptionalHeader.AddressOfEntryPoint;// +i32AdditionalOffset;
		int32_t i32PointerToRawData = 0;
		int32_t i32RVA = 0;
		int32_t i32SizeOfRawData = 0;
		int32_t i32SizeOfCode = pNtH->OptionalHeader.SizeOfCode;
		int32_t i32SizeOfImage = pNtH->OptionalHeader.SizeOfImage;// +i32AdditionalOffset;
		int32_t i32TextSizeOfCode = 0;
		int32_t i32FileEntryPointAddress = 0;

		int32_t i32cfgRVA = 0;
		int32_t i32cfgPointerToRawData = 0;
		int32_t i32cfgSizeofRawData = 0;

		int32_t i32RelocRVA = 0;
		int32_t i32RelocPointerToRawData = 0;
		int32_t i32RelocSizeofRawData = 0;
		int32_t i32FileTextRva = 0;

		std::vector< Section> vctSection;

		int* pModifiedTextCharacteristics = (int*)0xe0000060;
		int i32FLStart = pDosH->e_lfanew + sizeof(IMAGE_NT_HEADERS64);
		i32FileEntryPointAddress = pDosH->e_lfanew + 4 + sizeof(IMAGE_FILE_HEADER) + 0x10;


		int32_t i32Start = 0;
		int32_t OriginalImageOfSize = i32SizeOfImage;


		for(int i = 0; i < pNtH->FileHeader.NumberOfSections; i++)
		{
			pSecH = (PIMAGE_SECTION_HEADER)((LPBYTE)buf + pDosH->e_lfanew + sizeof(IMAGE_NT_HEADERS64) + (i * sizeof(IMAGE_SECTION_HEADER)));

			Section Temp;

			int32_t i32SectionParse = pDosH->e_lfanew + sizeof(IMAGE_NT_HEADERS64) + (i * sizeof(IMAGE_SECTION_HEADER));

			int32_t i32OrigialSize = 0;

			memcpy((void*)&i32OrigialSize, (void*)&buf[i32SectionParse + 0x10], 4);

			if(i == pNtH->FileHeader.NumberOfSections - 1)
			{
				i32Start = pSecH->SizeOfRawData + pSecH->PointerToRawData;
				//i32Start /= 4;

				i32OrigialSize += i32FLSize;
				memcpy((void*)&buf[i32SectionParse + 0x10], (void*)&i32OrigialSize, 4);

				i32OrigialSize = 0x10000;
				memcpy((void*)&buf[i32SectionParse + 0x8], (void*)&i32OrigialSize, 4);
			}

			//buf[(int32_t)pSecH + 0x14] = pSecH->PointerToRawData + i32AdditionalOffset;
			Temp.PoitnerToRawData = pSecH->PointerToRawData;
			Temp.RVA = pSecH->VirtualAddress;
			Temp.SizeOfRawData = pSecH->SizeOfRawData;
			strcpy(Temp.Name, (const char*)pSecH->Name);
			vctSection.push_back(Temp);

			memcpy((void*)&pSecH->Characteristics, (void*)&pModifiedTextCharacteristics, 4);
			i32FLStart += sizeof(IMAGE_SECTION_HEADER);

			if(!strcmp((const char*)pSecH->Name, ".text"))
			{
				i32PointerToRawData = pSecH->PointerToRawData;// +i32AdditionalOffset;
				i32RVA = pSecH->VirtualAddress;
				i32SizeOfRawData = pSecH->SizeOfRawData;
				i32FileTextRva = pDosH->e_lfanew + sizeof(IMAGE_NT_HEADERS64) + (i * sizeof(IMAGE_SECTION_HEADER));
				i32FileTextRva += 0xc;
				//i32TextSizeOfCode=
			}
			else if(!strcmp((const char*)pSecH->Name, ".reloc"))
			{
				i32RelocRVA = pSecH->VirtualAddress;
				i32RelocPointerToRawData = pSecH->PointerToRawData;// +i32AdditionalOffset;
				i32RelocSizeofRawData = pSecH->SizeOfRawData;
			}
			else if(!strcmp((const char*)pSecH->Name, ".00cfg"))
			{
				i32cfgRVA = pSecH->VirtualAddress;
				i32cfgPointerToRawData = pSecH->PointerToRawData;// +i32AdditionalOffset;
				i32cfgSizeofRawData = pSecH->SizeOfRawData;
			}
		}

		int32_t i32RollBackEntryPoint = i32RelocRVA + i32Start - i32RelocPointerToRawData;

		int32_t* ModifiedSizeOfImage = (int32_t*)(pNtH->OptionalHeader.SizeOfImage + i32FLSize);// +i32AdditionalOffset);
		int32_t* ModifiedEntryPoint = (int32_t*)i32RollBackEntryPoint;
		WORD* NumberOfSection = (WORD*)(pNtH->FileHeader.NumberOfSections);

		memcpy((void*)&pNtH->OptionalHeader.SizeOfImage, (void*)&ModifiedSizeOfImage, 4);
		memcpy((void*)&pNtH->OptionalHeader.AddressOfEntryPoint, (void*)&ModifiedEntryPoint, 4);
		memcpy((void*)&pNtH->FileHeader.NumberOfSections, (void*)&NumberOfSection, 2);
		/*memcpy((void*)&buf[0x130], (void*)&ModifiedSizeOfImage, 4);
		memcpy((void*)&buf[0x108], (void*)&ModifiedEntryPoint, 4);
		memcpy((void*)&buf[0x224], (void*)&pModifiedTextCharacteristics, 4);
		memcpy((void*)&buf[0x24c], (void*)&pModifiedTextCharacteristics, 4);
		memcpy((void*)&buf[0x274], (void*)&pModifiedTextCharacteristics, 4);
		memcpy((void*)&buf[0x29c], (void*)&pModifiedTextCharacteristics, 4);
		memcpy((void*)&buf[0x2c4], (void*)&pModifiedTextCharacteristics, 4);
		memcpy((void*)&buf[0x2ec], (void*)&pModifiedTextCharacteristics, 4);
		memcpy((void*)&buf[0x314], (void*)&pModifiedTextCharacteristics, 4);
		memcpy((void*)&buf[0x33c], (void*)&pModifiedTextCharacteristics, 4);
		*/
		//buf[0xe6] = '\xa';
		/*Section FLSection;
		FLSection.Name[0] = '.';
		FLSection.Name[1] = 'F';
		FLSection.Name[2] = 'L';
		FLSection.Name[3] = '\x00';

		FLSection.VirtualSize = i32FLSize;
		FLSection.RVA = i32SizeOfImage;
		FLSection.SizeOfRawData = i32FLSize;
		FLSection.PoitnerToRawData = stSize + i32AdditionalOffset;
		FLSection.POinterToRelocations = 0;
		FLSection.PointerToLineNumber = 0;
		FLSection.NumberOfRelocations = 0;
		FLSection.NumberOfLineNumbers = 0;
		FLSection.Characteristics = 0xe0000020;//

		memcpy((void*)&strTemp[i32FLStart], (void*)&FLSection, sizeof(FLSection));
		i32FLStart += sizeof(IMAGE_SECTION_HEADER);
		*/
		std::vector<std::pair<int, int> > vctRelocationVector;

		int32_t i32RvaOfBlock = 0;
		int32_t i32SizeOfBlock = 0;

		int32_t i32RelocPointerToRawDataToRelocSizeOfBlock = i32RelocPointerToRawData + 4;

		memcpy((void*)&i32RvaOfBlock, (void*)&i32RelocPointerToRawData, 4);
		memcpy((void*)&i32SizeOfBlock, (void*)&i32RelocPointerToRawDataToRelocSizeOfBlock, 4);

		if(i32RelocRVA != 0)
		{
			vctRelocationVector.push_back({ i32RvaOfBlock,i32SizeOfBlock });
			while(1)
			{
				int32_t i32TempRelocPointerToRawData = 0;
				memcpy((void*)&i32TempRelocPointerToRawData, (void*)&buf[i32SizeOfBlock], 4);
				i32RelocPointerToRawData += i32TempRelocPointerToRawData;

				i32RelocPointerToRawDataToRelocSizeOfBlock = i32RelocPointerToRawData + 4;

				int32_t i32TempSizeOfBlock = 0;
				memcpy((void*)&i32TempSizeOfBlock, (void*)&buf[i32SizeOfBlock], 4);
				if(i32TempSizeOfBlock == 0)
					break;
				memcpy((void*)&i32RvaOfBlock, (void*)&i32RelocPointerToRawData, 4);
				memcpy((void*)&i32SizeOfBlock, (void*)(&i32RelocPointerToRawDataToRelocSizeOfBlock), 4);
				vctRelocationVector.push_back({ i32RvaOfBlock,i32SizeOfBlock });
			}
		}
		char cFileTextRva[2] = { 0, };
		memcpy((void*)&cFileTextRva, (void*)(&i32FileTextRva), 2);

		bool bCheckIsDllorExe = false;

		if((pNtH->FileHeader.Characteristics & 0xf000) == 0x2000)
		{
			bCheckIsDllorExe = true;
		}
		int32_t i32stSizeCnt = 0;
		//stSize += i32AdditionalOffset;
		buf[stSize + i32stSizeCnt++] = '\x50';
		buf[stSize + i32stSizeCnt++] = '\x53';
		buf[stSize + i32stSizeCnt++] = '\x51';
		buf[stSize + i32stSizeCnt++] = '\x52';
		buf[stSize + i32stSizeCnt++] = '\x57';
		buf[stSize + i32stSizeCnt++] = '\x56';
		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x50';
		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x51';
		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x52';
		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x53';
		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x54';
		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x55';
		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x56';
		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x57';


		if(bCheckIsDllorExe == true)
		{
			buf[stSize + i32stSizeCnt++] = '\x48';
			buf[stSize + i32stSizeCnt++] = '\x8b';
			buf[stSize + i32stSizeCnt++] = '\xc1';
		}
		else
		{
			buf[stSize + i32stSizeCnt++] = '\x65';
			buf[stSize + i32stSizeCnt++] = '\x48';
			buf[stSize + i32stSizeCnt++] = '\x8b';
			buf[stSize + i32stSizeCnt++] = '\x04';
			buf[stSize + i32stSizeCnt++] = '\x25';
			buf[stSize + i32stSizeCnt++] = '\x30';
			buf[stSize + i32stSizeCnt++] = '\x0';
			buf[stSize + i32stSizeCnt++] = '\x0';
			buf[stSize + i32stSizeCnt++] = '\x0';

			buf[stSize + i32stSizeCnt++] = '\x48';
			buf[stSize + i32stSizeCnt++] = '\x8b';
			buf[stSize + i32stSizeCnt++] = '\x40';
			buf[stSize + i32stSizeCnt++] = '\x60';

			buf[stSize + i32stSizeCnt++] = '\x48';
			buf[stSize + i32stSizeCnt++] = '\x8b';
			buf[stSize + i32stSizeCnt++] = '\x40';
			buf[stSize + i32stSizeCnt++] = '\x10';

			buf[stSize + i32stSizeCnt++] = '\x48';
			buf[stSize + i32stSizeCnt++] = '\x8b';
			buf[stSize + i32stSizeCnt++] = '\xf0';
		}
	//	buf[stSize + i32stSizeCnt++] = '\x18';

		char cChangeEntryPoint[4] = { 0, };
		int32_t i32CheckChangeEntryPoint = i32RollBackEntryPoint + 0xf0;

		memcpy((void*)&cChangeEntryPoint, (void*)&i32CheckChangeEntryPoint, 4);

		buf[stSize + i32stSizeCnt++] = '\x48';
		buf[stSize + i32stSizeCnt++] = '\x8b';
		buf[stSize + i32stSizeCnt++] = '\xf0';

		buf[stSize + i32stSizeCnt++] = '\x48';
		buf[stSize + i32stSizeCnt++] = '\x81';
		buf[stSize + i32stSizeCnt++] = '\xc6';

		buf[stSize + i32stSizeCnt++] = cChangeEntryPoint[0];
		buf[stSize + i32stSizeCnt++] = cChangeEntryPoint[1];
		buf[stSize + i32stSizeCnt++] = cChangeEntryPoint[2];
		buf[stSize + i32stSizeCnt++] = cChangeEntryPoint[3];

		buf[stSize + i32stSizeCnt++] = '\x48';
		buf[stSize + i32stSizeCnt++] = '\x8b';
		buf[stSize + i32stSizeCnt++] = '\xfe';

		buf[stSize + i32stSizeCnt++] = '\x48';
		buf[stSize + i32stSizeCnt++] = '\x8b';
		buf[stSize + i32stSizeCnt++] = '\x36';

		buf[stSize + i32stSizeCnt++] = '\x48';
		buf[stSize + i32stSizeCnt++] = '\x83';
		buf[stSize + i32stSizeCnt++] = '\xfe';
		buf[stSize + i32stSizeCnt++] = '\x01';// cmp rsi,1

		buf[stSize + i32stSizeCnt++] = '\x48';
		buf[stSize + i32stSizeCnt++] = '\xc7';
		buf[stSize + i32stSizeCnt++] = '\x07';
		buf[stSize + i32stSizeCnt++] = '\x01';
		buf[stSize + i32stSizeCnt++] = '\x00';
		buf[stSize + i32stSizeCnt++] = '\x00';
		buf[stSize + i32stSizeCnt++] = '\x00';

		WORD dwMoveToDecode = 0x100 - i32stSizeCnt;//
		WORD dwChangeMoveToDecode = dwMoveToDecode - 6;// -3;

		char cChangeMoveToDecode[2] = { 0 };

		memcpy((void*)&cChangeMoveToDecode, (void*)&dwChangeMoveToDecode, 2);

		buf[stSize + i32stSizeCnt++] = '\x0f';
		buf[stSize + i32stSizeCnt++] = '\x85';

		buf[stSize + i32stSizeCnt++] = cChangeMoveToDecode[0];
		buf[stSize + i32stSizeCnt++] = cChangeMoveToDecode[1];
		buf[stSize + i32stSizeCnt++] = '\x00';
		buf[stSize + i32stSizeCnt++] = '\x00';

		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x5f';

		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x5e';

		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x5d';

		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x5c';

		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x5b';

		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x5a';

		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x59';

		buf[stSize + i32stSizeCnt++] = '\x41';
		buf[stSize + i32stSizeCnt++] = '\x58';

		buf[stSize + i32stSizeCnt++] = '\x5e';
		buf[stSize + i32stSizeCnt++] = '\x5f';
		buf[stSize + i32stSizeCnt++] = '\x5a';
		buf[stSize + i32stSizeCnt++] = '\x59';
		buf[stSize + i32stSizeCnt++] = '\x5b';
		buf[stSize + i32stSizeCnt++] = '\x58';


		int32_t i32FLFuncionStart = i32stSizeCnt + i32RollBackEntryPoint;//
		int32_t i32ChangeEntryPointToOriginal = i32EntryPoint - i32FLFuncionStart - 5;// -3;

		char cOriginalEntryPoint[4] = { 0, };

		memcpy((void*)&cOriginalEntryPoint, (void*)&i32ChangeEntryPointToOriginal, 4);

		buf[stSize + i32stSizeCnt++] = '\xe9';

		buf[stSize + i32stSizeCnt++] = cOriginalEntryPoint[0];
		buf[stSize + i32stSizeCnt++] = cOriginalEntryPoint[1];
		buf[stSize + i32stSizeCnt++] = cOriginalEntryPoint[2];
		buf[stSize + i32stSizeCnt++] = cOriginalEntryPoint[3];



		/*
		buf[stSize + i32stSizeCnt++] = '\x83';
		buf[stSize + i32stSizeCnt++] = '\xc4';
		buf[stSize + i32stSizeCnt++] = '\x0c';

		buf[stSize + i32stSizeCnt++] = '\x8b';
		buf[stSize + i32stSizeCnt++] = '\xec';

		buf[stSize + i32stSizeCnt++] = '\x5d';
		*/
		/*std::string strFileName = "DllEntryPointToFLFunction.dll";
		for(int i = 0; i < strFileName.size(); i += 1)
		{
			buf[i32FLStart + 2 * i] = strFileName[i];
			buf[i32FLStart + 2 * i + 1] = '\x00';
		}*/

		int32_t i32OffsetCnt = 0x100;

		for(int32_t i = stSize + i32stSizeCnt; i < stSize + i32OffsetCnt; i++)
		{
			buf[i] = '\x0';
		}


		/*
		buf[stSize + i32OffsetCnt++] = '\x64';
		buf[stSize + i32OffsetCnt++] = '\xa1';
		buf[stSize + i32OffsetCnt++] = '\x18';
		buf[stSize + i32OffsetCnt++] = '\x0';

		buf[stSize + i32OffsetCnt++] = '\x0';
		buf[stSize + i32OffsetCnt++] = '\x0';
		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\x40';

		buf[stSize + i32OffsetCnt++] = '\x30';
		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\x40';
		buf[stSize + i32OffsetCnt++] = '\x0c';

		buf[stSize + i32OffsetCnt++] = '\x8d';
		buf[stSize + i32OffsetCnt++] = '\x58';
		buf[stSize + i32OffsetCnt++] = '\x0c';
		buf[stSize + i32OffsetCnt++] = '\x8b';

		buf[stSize + i32OffsetCnt++] = '\x13';
		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\x42';
		buf[stSize + i32OffsetCnt++] = '\x18';
		*/
		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\xd8';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\xd0';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x81';
		buf[stSize + i32OffsetCnt++] = '\xc2';
		buf[stSize + i32OffsetCnt++] = cFileTextRva[0];
		buf[stSize + i32OffsetCnt++] = cFileTextRva[1];
		buf[stSize + i32OffsetCnt++] = '\x00';
		buf[stSize + i32OffsetCnt++] = '\x00';


		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\x1a';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x83';
		buf[stSize + i32OffsetCnt++] = '\xc2';
		buf[stSize + i32OffsetCnt++] = '\x04';

		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\x0a';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x83';
		buf[stSize + i32OffsetCnt++] = '\xc2';
		buf[stSize + i32OffsetCnt++] = '\x04';

		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\x32';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x2b';
		buf[stSize + i32OffsetCnt++] = '\xce';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x03';
		buf[stSize + i32OffsetCnt++] = '\xd8';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x83';
		buf[stSize + i32OffsetCnt++] = '\xc2';
		buf[stSize + i32OffsetCnt++] = '\x10';

		buf[stSize + i32OffsetCnt++] = '\xf6';
		buf[stSize + i32OffsetCnt++] = '\x13';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x83';
		buf[stSize + i32OffsetCnt++] = '\xc3';
		buf[stSize + i32OffsetCnt++] = '\x01';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x83';
		buf[stSize + i32OffsetCnt++] = '\xe9';
		buf[stSize + i32OffsetCnt++] = '\x01';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x83';
		buf[stSize + i32OffsetCnt++] = '\xf9';
		buf[stSize + i32OffsetCnt++] = '\x00';

		buf[stSize + i32OffsetCnt++] = '\x75';
		buf[stSize + i32OffsetCnt++] = '\xf0';
//		buf[stSize + i32OffsetCnt++] = '\x00';
//		buf[stSize + i32OffsetCnt++] = '\x00';

		/*
		buf[stSize + 0x5e] = '\xe9';

		buf[stSize + 0x5f] = '\xd1';
		buf[stSize + 0x60] = '\x12';
		buf[stSize + 0x61] = '\xff';
		buf[stSize + 0x62] = '\xff';
		*///Jump To Original Entry Point

		for(int i = 0; i < vctRelocationVector.size(); i++)
		{
			int32_t i32RvaOfBlock = vctRelocationVector[i].first;
			int32_t i32DeicdeToRvaOfBlock = 0;
			memcpy((void*)&i32DeicdeToRvaOfBlock, (void*)&buf[i32RvaOfBlock], 4);
			int32_t i32Section = 0;
			for(int j = 0; j < vctSection.size() - 1; j++)
			{
				int32_t i32FromRvaOfBlock = vctSection[j].RVA;
				int32_t i32ToRvaOfBlock = vctSection[j + 1].RVA;

				if(i32FromRvaOfBlock <= i32DeicdeToRvaOfBlock && i32DeicdeToRvaOfBlock < i32ToRvaOfBlock)
				{
					i32Section = j;
					break;
				}
			}

			int32_t i32Size = 0;
			memcpy((void*)&i32Size, (void*)&buf[vctRelocationVector[i].second], 4);

			int32_t i32Start = vctRelocationVector[i].second + 4;

			for(int32_t j = 2; j < i32Size - 6; j += 2)
			{
				WORD Data = 0;
				int32_t i32RealRvaOfBlock = 0;

				memcpy((void*)&i32RealRvaOfBlock, (void*)&buf[i32RvaOfBlock], 4);
				memcpy((void*)&Data, (void*)&buf[i32Start], 2);
				if(Data == 0)
					continue;
				Data &= 0x0fff;

				int32_t i32RelocData = i32RealRvaOfBlock + Data;

	//			RelocData -= vctSection[Section].RVA;
//				RelocData += vctSection[Section].PoitnerToRawData;
				//i32RelocData += 2;

				int32_t i32FileRelocOffset = Data + i32RealRvaOfBlock - vctSection[i32Section].RVA + vctSection[i32Section].PoitnerToRawData;// +2;


				vctParseRelocation.push_back({ i32Section,{i32RelocData,i32FileRelocOffset} });

				i32Start += 2;
			}
		}
		char cFileImageBase[8] = { 0 };
		memcpy((void*)&cFileImageBase, (void*)&i64FileBaseAddress, 8);

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\xbb';
		buf[stSize + i32OffsetCnt++] = cFileImageBase[0];
		buf[stSize + i32OffsetCnt++] = cFileImageBase[1];
		buf[stSize + i32OffsetCnt++] = cFileImageBase[2];
		buf[stSize + i32OffsetCnt++] = cFileImageBase[3];
		buf[stSize + i32OffsetCnt++] = cFileImageBase[4];
		buf[stSize + i32OffsetCnt++] = cFileImageBase[5];
		buf[stSize + i32OffsetCnt++] = cFileImageBase[6];
		buf[stSize + i32OffsetCnt++] = cFileImageBase[7];

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x8b';
		buf[stSize + i32OffsetCnt++] = '\xc8';

		buf[stSize + i32OffsetCnt++] = '\x48';
		buf[stSize + i32OffsetCnt++] = '\x2b';
		buf[stSize + i32OffsetCnt++] = '\xcb';


		int32_t i32cnt = i32OffsetCnt;
		for(int32_t i = 0; i < vctParseRelocation.size(); i++)
		{
			int32_t i32RelocData = vctParseRelocation[i].second.first;

			int32_t i32InputData = i32RelocData;// +DeicdeToRvaOfBlock;
			//InputData += BaseAddress;

			char cInputData[4] = { 0, };

			memcpy((void*)&cInputData, (void*)&i32InputData, 4);
			int32_t i32FileRelocOffset = vctParseRelocation[i].second.second;
			char cType = buf[i32FileRelocOffset] & 0xff;
			char cType1 = buf[i32FileRelocOffset + 1] & 0xff;
			char cType2 = buf[i32FileRelocOffset + 2] & 0xff;
			char cType3 = buf[i32FileRelocOffset + 3] & 0xff;
			char cType4 = buf[i32FileRelocOffset + 4] & 0xff;
			char cType5 = buf[i32FileRelocOffset + 5] & 0xff;
			char cType6 = buf[i32FileRelocOffset + 6] & 0xff;
			char cType7 = buf[i32FileRelocOffset + 7] & 0xff;


			buf[stSize + i32cnt++] = '\x48';
			buf[stSize + i32cnt++] = '\x8b';
			buf[stSize + i32cnt++] = '\xd8';

			buf[stSize + i32cnt++] = '\x48';
			buf[stSize + i32cnt++] = '\x81';
			buf[stSize + i32cnt++] = '\xc3';
			buf[stSize + i32cnt++] = cInputData[0];
			buf[stSize + i32cnt++] = cInputData[1];
			buf[stSize + i32cnt++] = cInputData[2];
			buf[stSize + i32cnt++] = cInputData[3];
			//buf[stSize + i32cnt++] = cInputData[4];
			//buf[stSize + i32cnt++] = cInputData[5];
			//buf[stSize + i32cnt++] = cInputData[6];
			//buf[stSize + i32cnt++] = cInputData[7];


			buf[stSize + i32cnt++] = '\x48';
			buf[stSize + i32cnt++] = '\xbe';
			buf[stSize + i32cnt++] = cType;
			buf[stSize + i32cnt++] = cType1;
			buf[stSize + i32cnt++] = cType2;
			buf[stSize + i32cnt++] = cType3;
			buf[stSize + i32cnt++] = cType4;
			buf[stSize + i32cnt++] = cType5;
			buf[stSize + i32cnt++] = cType6;
			buf[stSize + i32cnt++] = cType7;

			buf[stSize + i32cnt++] = '\x48';
			buf[stSize + i32cnt++] = '\x03';
			buf[stSize + i32cnt++] = '\xf1';

			buf[stSize + i32cnt++] = '\x48';
			buf[stSize + i32cnt++] = '\x89';
			buf[stSize + i32cnt++] = '\x33';
			//buf[stSize + i32cnt++] = '\x10';
		}




		/*buf[stSize + cnt++] = '\x8b';// EntryPoint를 바꾸는 부분인데 Write속성이 없어서 작동이 안하는 것 같음
		buf[stSize + cnt++] = '\xc8';

		buf[stSize + cnt++] = '\x81';
		buf[stSize + cnt++] = '\xc1';
		buf[stSize + cnt++] = cChangeEntryPoint[0];
		buf[stSize + cnt++] = cChangeEntryPoint[1];
		buf[stSize + cnt++] = cChangeEntryPoint[2];
		buf[stSize + cnt++] = cChangeEntryPoint[3];

		buf[stSize + cnt++] = '\x33';
		buf[stSize + cnt++] = '\xdb';

		buf[stSize + cnt++] = '\x81';
		buf[stSize + cnt++] = '\xc3';

		buf[stSize + cnt++] = cEntryPoint[0];
		buf[stSize + cnt++] = cEntryPoint[1];
		buf[stSize + cnt++] = cEntryPoint[2];
		buf[stSize + cnt++] = cEntryPoint[3];

		buf[stSize + cnt++] = '\x89';
		buf[stSize + cnt++] = '\x19';

		buf[stSize + cnt++] = '\x61';*/



		buf[stSize + i32cnt++] = '\x41';
		buf[stSize + i32cnt++] = '\x5f';

		buf[stSize + i32cnt++] = '\x41';
		buf[stSize + i32cnt++] = '\x5e';

		buf[stSize + i32cnt++] = '\x41';
		buf[stSize + i32cnt++] = '\x5d';

		buf[stSize + i32cnt++] = '\x41';
		buf[stSize + i32cnt++] = '\x5c';

		buf[stSize + i32cnt++] = '\x41';
		buf[stSize + i32cnt++] = '\x5b';

		buf[stSize + i32cnt++] = '\x41';
		buf[stSize + i32cnt++] = '\x5a';

		buf[stSize + i32cnt++] = '\x41';
		buf[stSize + i32cnt++] = '\x59';

		buf[stSize + i32cnt++] = '\x41';
		buf[stSize + i32cnt++] = '\x58';

		buf[stSize + i32cnt++] = '\x5e';
		buf[stSize + i32cnt++] = '\x5f';
		buf[stSize + i32cnt++] = '\x5a';
		buf[stSize + i32cnt++] = '\x59';
		buf[stSize + i32cnt++] = '\x5b';
		buf[stSize + i32cnt++] = '\x58';
	/*
		buf[stSize + cnt++] = '\x83';
		buf[stSize + cnt++] = '\xec';
		buf[stSize + cnt++] = '\x08';
		*/

		int32_t i32FLLast = i32cnt + i32RollBackEntryPoint;
		int32_t i32FLfunctionToEntryPoint = i32EntryPoint - i32FLLast - 5;// -3;

		char cFLfunctionToEntryPoint[4] = { 0, };

		memcpy((void*)&cFLfunctionToEntryPoint, (void*)&i32FLfunctionToEntryPoint, 4);

		 //dll만 로딩할때


		buf[stSize + i32cnt++] = '\xe9';

		buf[stSize + i32cnt++] = cFLfunctionToEntryPoint[0];
		buf[stSize + i32cnt++] = cFLfunctionToEntryPoint[1];
		buf[stSize + i32cnt++] = cFLfunctionToEntryPoint[2];
		buf[stSize + i32cnt++] = cFLfunctionToEntryPoint[3];
		buf[stSize + i32cnt++] = '\x00';
		buf[stSize + i32cnt++] = '\x00';
		buf[stSize + i32cnt++] = '\x00';
		buf[stSize + i32cnt++] = '\x00';
		buf[stSize + i32cnt++] = '\x01';



		for(int32_t i = i32PointerToRawData; i < i32SizeOfCode; i++)
		{
			buf[i] = ~buf[i];
		}


		fwrite(buf, sizeof(char), stSize + i32FLSize, fp);
		fclose(fp);
	}

}