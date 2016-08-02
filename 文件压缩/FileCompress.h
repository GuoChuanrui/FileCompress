
#pragma once
#include"HuffmanTree.h"
#include<algorithm>
#include<string>
typedef long LongType;
struct FileInfo
{
	unsigned char _ch;   //字符
	LongType _count;     //字符出现的次数
	string _code;        //字符对应的Huffman编码

	FileInfo(unsigned char ch = 0)
		:_ch(ch)
		, _count(0)
	{}

	bool operator< (const FileInfo& info)
	{
		return this->_count < info._count;
	}

	FileInfo operator+ (const FileInfo& info)
	{
		FileInfo tmp;
		tmp._count = this->_count + info._count;
		return tmp;
	}

	bool operator!= (const FileInfo& info) const
	{
		return this->_count != info._count;
	}
};


class FileCompress
{

public:
	FileCompress()
	{
		for (int i = 0; i < 256; ++i)
		{
			_infos[i]._ch = i;
		}
	}


	bool Compress(const char* filename)
	{
		assert(filename);

		//1.打开文件，统计文件中字符出现的次数
		FILE* fOut = fopen(filename, "rb");
		assert(fOut);

		long long chSize = 0;
		char ch = fgetc(fOut);
		while (ch != EOF)
		{

			++chSize;
			_infos[(unsigned char)ch]._count++;
			ch = fgetc(fOut);
		}

		//2.生成对应的Huffman编码
		HuffmanTree<FileInfo> t;
		FileInfo invalid;
		t.CreateTree(_infos, 256, invalid);
		_GenerateHuffmanCode(t.GetRootNode());  

		//3.压缩文件
		string compressFile = filename;
		compressFile += ".huffman";
		FILE* fInCompress = fopen(compressFile.c_str(), "wb");
		assert(fInCompress);
		fseek(fOut, 0, SEEK_SET);
		ch = fgetc(fOut);
		int index = 0;
		char inCh = 0;
		while (ch != EOF)
		{
			string& code = _infos[(unsigned char)ch]._code;
			for (size_t i = 0; i < code.size(); ++i)
			{
				inCh <<= 1;
				if (code[i] == '1')
				{
					inCh |= 1;
				}
				if (++index == 8)
				{
					fputc(inCh, fInCompress);
					index = 0;
					inCh = 0;
				}
			}
			ch = fgetc(fOut);
		}

		if (index != 0)    //说明inCh中还有余留的字符
		{
			inCh <<= (8 - index);
			fputc(inCh, fInCompress);
		}

		//4.写配置文件，方便后续的解压缩
		char str[128];
		string configFile = filename;
		configFile += ".config";
		FILE* fInConfig = fopen(configFile.c_str(), "wb");
		assert(fInConfig);
		
		_itoa(chSize >> 32, str, 10);
		fputs(str, fInConfig);
		fputc('\n', fInConfig);

		_itoa(chSize & 0xffffffff, str, 10);
		fputs(str, fInConfig);
		fputc('\n', fInConfig);

		for (size_t i = 0; i < 256; ++i)
		{
			string chInfo;
			if (_infos[i]._count>0)
			{
				chInfo += _infos[i]._ch;
				chInfo += ',';
				chInfo += _itoa(_infos[i]._count, str, 10);
				chInfo += '\n';
				fputs(chInfo.c_str(), fInConfig);
			}
		}

		fclose(fInConfig);
		fclose(fOut);
		fclose(fInCompress);
		return true;
	}

	bool _ReadLine(FILE* fOut, string& line)
	{
		char ch = fgetc(fOut);
		if (ch == EOF)
		{
			return false;
		}
		while (ch != EOF && ch != '\n')
		{
			line += ch;
			ch = fgetc(fOut);
		}
		return true;
	}

	//解压缩
	bool UnCompress(const char* filename)
	{
		//1.读取配置文件，重建HuffmanTree
		string configFile = filename;
		configFile += ".config";
		FILE* fOutConfig = fopen(configFile.c_str(), "rb");
		assert(fOutConfig);

		string line;
		long long chSize = 0;
		_ReadLine(fOutConfig, line);
		chSize = atoi(line.c_str());   //取到高4字节
		chSize <<= 32;   //移到高4字节上
		line.clear();

		_ReadLine(fOutConfig, line);
		chSize += atoi(line.c_str());
		line.clear();

		while (_ReadLine(fOutConfig, line))
		{
			if (!line.empty())
			{
				unsigned char ch = line[0];
				_infos[ch]._count = atoi(line.substr(2).c_str());
				line.clear();
			}
			else
			{
				line = '\n';
			}
		}

		HuffmanTree<FileInfo> t;
		FileInfo invalid;
		t.CreateTree(_infos, 256, invalid);
		HuffmanTreeNode<FileInfo>* root = t.GetRootNode();

		//2.解压缩
		string compressFile = filename;
		compressFile += ".huffman";
		FILE* fOutCompress = fopen(compressFile.c_str(), "rb");
		assert(fOutCompress);

		string uncompressFile = filename;
		uncompressFile += ".uncompress";
		FILE* fInUncompress = fopen(uncompressFile.c_str(), "wb");
		assert(fInUncompress);

		char ch = fgetc(fOutCompress);
		HuffmanTreeNode<FileInfo>* cur = root;
		int pos = 8;

		while (!feof(fOutCompress))
		{
			//while (cur)
			{
				--pos;
				if (ch & (1 << pos))
				{
					cur = cur->_right;
				}
				else
				{
					cur = cur->_left;
				}

				if (cur->_left == NULL && cur->_right == NULL)
				{
					fputc(cur->_weight._ch, fInUncompress);
					cur = root;
					if (--chSize == 0)
					{
						break;
					}
				}

				if (pos == 0)
				{
					ch = fgetc(fOutCompress);
					pos = 8;
				}
			}
		}

		fclose(fInUncompress);
		fclose(fOutCompress);
		fclose(fOutConfig);

		return true;
	}

public:
	void _GenerateHuffmanCode(HuffmanTreeNode<FileInfo>* root)
	{
		if (root == NULL)
		{
			return;
		}
		_GenerateHuffmanCode(root->_left);
		_GenerateHuffmanCode(root->_right);

		//如果当前节点是叶子节点则生成对应的huffman编码
		if (root->_left == NULL && root->_right == NULL)
		{
			HuffmanTreeNode<FileInfo>* cur = root;
			HuffmanTreeNode<FileInfo>* parent = cur->_parent;
			string& code = _infos[cur->_weight._ch]._code;

			while (parent)
			{
				if (parent->_left == cur)
				{
					code += '0';
				}
				else
				{
					code += '1';
				}
				cur = parent;
				parent = cur->_parent;
			}
			reverse(code.begin(), code.end());  //逆置
		}
	}

protected:
	FileInfo _infos[256];

};

void TestFileCompress()
{
	FileCompress fc;
	fc.Compress("In.txt");
}

void TestFileUncompress()
{
	FileCompress fc;
	fc.UnCompress("In.txt");
}