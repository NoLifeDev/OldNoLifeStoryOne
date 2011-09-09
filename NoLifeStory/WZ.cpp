////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////
#include "Global.h"
#include "Keys.h"

set<NLS::WZ::File*> Files;
string Path;
NLS::Node NLS::WZ::Top;
NLS::Node NLS::WZ::Empty;
uint8_t *WZKey = 0;
uint8_t BMSKey[0xFFFF];
uint8_t *WZKeys[] = {GMSKey, BMSKey};
uint8_t GMSKeyIV[4] = {0x4D, 0x23, 0xC7, 0x2B};
uint32_t OffsetKey = 0x581C3F6D;
int16_t EncVersion;
uint16_t Version;
uint32_t VersionHash;

#pragma region ZLib Stuff
uint8_t* Decompress(uint8_t* inBuffer, uint32_t inLen, uint32_t outLen){
	const uint32_t inChunk = 4096;
	uint32_t inBufferIndex = 0;
	const uint32_t outChunk = 4096;
	uint32_t outBufferIndex = 0;
	uint8_t* outBuffer = new uint8_t[outLen];
	int err = Z_OK;
	z_stream strm;
	strm.next_in = nullptr;
	strm.avail_in = 0;
	strm.opaque = nullptr;
	strm.zfree = nullptr;
	strm.zalloc = nullptr;
	inflateInit(&strm);
	do {
		strm.next_in = inBuffer+inBufferIndex;
		strm.avail_in = (inLen-inBufferIndex > inChunk ? inChunk : inLen-inBufferIndex);
		inBufferIndex += strm.avail_in;
		if (!strm.avail_in) {
			break;
		}
		strm.next_out = outBuffer;
		strm.avail_out = outLen;
		err = inflate(&strm, Z_NO_FLUSH);
		switch(err){
		case Z_NEED_DICT:
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
		case Z_BUF_ERROR:
			NLS::C("ERROR") << "I hate zlib!" << endl;
			throw(273);
			break;
		}
	} while (err != Z_STREAM_END);
	inflateEnd(&strm);
	return outBuffer;
}
#pragma endregion

#pragma region File Reading Stuff
template <class T>
inline T Read(ifstream& file) {
	T v;
	file.read((char*)&v, sizeof(v));
	return v;
}

template <class T>
inline T Read(NLS::WZ::File* file) {
	T v;
	file->file.read((char*)&v, sizeof(v));
	return v;
}

inline int32_t ReadCInt(ifstream& file) {
	int8_t a = Read<int8_t>(file);
	if (a != -128) {
		return a;
	} else {
		int32_t b = Read<int32_t>(file);
		return b;
	}
}

inline uint32_t ReadOffset(NLS::WZ::File* file) {
	uint32_t p = file->file.tellg();
	p = (p-file->fileStart)^0xFFFFFFFF;
	p *= VersionHash;
	p -= OffsetKey;
	p = (p<<(p&0x1F))|(p>>(32-p&0x1F));
	uint32_t more = Read<uint32_t>(file);
	p ^= more;
	p += file->fileStart*2;
	return p;
}

inline string ReadEncString(ifstream& file) {
	int8_t slen = Read<int8_t>(file);
	if (slen == 0) {
		return string();
	} else if (slen > 0) {
		int32_t len;
		if (slen == 127) {
			len = Read<int32_t>(file);
		} else {
			len = slen;
		}
		if (len <= 0) {
			return string();
		}
		string s(len, '\0');
		uint16_t mask = 0xAAAA;
		for (int i = 0; i < len; i++) {
			uint16_t enc = Read<uint16_t>(file);
			enc ^= mask;
			enc ^= (WZKey[i*2+1]<<8)+WZKey[i*2];
			mask++;
			s[i] = enc;
		}
		return s;
	} else {
		int32_t len;
		if (slen == -128) {
			len = Read<int32_t>(file);
		} else {
			len = -slen;
		}
		if (len <= 0) {
			return string();
		}
		string s(len, '\0');
		uint8_t mask = 0xAA;
		for (int i = 0; i < len; i++) {
			uint8_t enc = Read<uint8_t>(file);
			enc ^= mask;
			enc ^= WZKey[i];
			mask++;
			s[i] = enc;
		}
		return s;
	}
}

inline void ReadEncFast(ifstream& file) {
	int8_t slen = Read<int8_t>(file);
	if (slen == 0) {return;}
	if (slen > 0) {
		int32_t len;
		if (slen == 127) {
			len = Read<int32_t>(file);
		} else {
			len = slen;
		}
		if (len <= 0) {return;}
		file.seekg(len*2, ios_base::cur);
	} else {
		int32_t len;
		if (slen == -128) {
			len = Read<int32_t>(file);
		} else {
			len = -slen;
		}
		if (len <= 0) {return;}
		file.seekg(len, ios_base::cur);
	}
}

inline string ReadString(ifstream& file, uint32_t offset) {
	uint8_t a = Read<uint8_t>(file);
	switch (a) {
	case 0x00:
	case 0x73:
		return ReadEncString(file);
	case 0x01:
	case 0x1B:
		{
			int32_t off = Read<int32_t>(file);
			offset += off;
			uint32_t p = file.tellg();
			file.seekg(offset);
			string s = ReadEncString(file);
			file.seekg(p);
			return s;
		}
	default:
		return string();
	}
}

inline string ReadStringOffset(ifstream& file, uint32_t offset) {
	uint32_t p = file.tellg();
	file.seekg(offset);
	string s = ReadEncString(file);
	file.seekg(p);
	return s;
}
#pragma endregion

#pragma region Parsing Stuff
bool NLS::WZ::Init(const string& path) {
	memset(BMSKey, 0, 0xFFFF);
	Path = path;
	Top.data = new NodeData();
	ifstream test(path+"Data.wz");
	bool beta = test.is_open();
	test.close();
	if (beta) {
		C("WZ") << "Loading beta WZ file structure" << endl;
		new File("Data", true);
	} else {
		C("WZ") << "Loading standard WZ file structure" << endl;
		new File("Base", false);
		for (auto it = Top.Begin(); it != Top.End(); it++) {
			new File(it->first);
		}
	}
	C("WZ") << "Great spot to break" << endl;
	return true;
}

NLS::WZ::File::File(const string& name) {
	string filename = Path+name+".wz";
	file.open(filename, file.in|file.binary);
	C("WZ") << "Loading file: " << filename << endl;
	if (!file.is_open()) {
		C("ERROR") << "Failed to load WZ file" << endl;
		throw(273);
	}
	Files.insert(this);
	ident.resize(4);
	file.read(const_cast<char*>(ident.c_str()), 4);
	fileSize = Read<uint64_t>(file);
	fileStart = Read<uint32_t>(file);
	file >> copyright;
	file.seekg(fileStart);
	int16_t eversion = Read<int16_t>(file);
	if (eversion != EncVersion) {
		C("ERROR") << "Version of WZ file does not match existing files" << endl;
	}
	new Directory(this, Top.g(name));
}
NLS::WZ::File::File(const string& name, bool beta) {//TODO: Finish this!
	string filename = Path+name+".wz";
	file.open(filename, file.in|file.binary);
	C("WZ") << "Loading file: " << filename << endl;
	if (!file.is_open()) {
		C("ERROR") << "Failed to load WZ file" << endl;
		throw(273);
	}
	Files.insert(this);
	ident.resize(4);
	file.read(const_cast<char*>(ident.c_str()), 4);
	fileSize = Read<uint64_t>(file);
	fileStart = Read<uint32_t>(file);
	file >> copyright;
	file.seekg(fileStart);
	EncVersion = Read<int16_t>(file);
	int32_t count = ReadCInt(file);
	uint32_t c = 0;
	for (int k = 0; k < count; k++) {
		uint8_t type = Read<uint8_t>(file);
		if (type == 3) {
			ReadEncFast(file);
			ReadCInt(file);
			ReadCInt(file);
			Read<uint32_t>(file);
			continue;
		} else if (type == 4) {
			ReadEncFast(file);
			ReadCInt(file);
			ReadCInt(file);
			c = file.tellg();
			break;
		} else {
			C("ERROR") << "Malformed WZ structure" << endl;
			throw(273);
		}
	}
	if (c == 0) {
		C("ERROR") << "Unable to find a top level .img for hash verification" << endl;
	}
	for (uint8_t j = 0; j < 2; j++) {
		WZKey = WZKeys[j];
		for (Version = 0; Version < 256; Version++) {
			VersionHash = Hash(EncVersion, Version);
			if (VersionHash) {
				file.clear();
				file.seekg(c);
				uint32_t offset = ReadOffset(this);
				if (offset > fileSize) {
					continue;
				}
				file.seekg(offset);
				uint8_t a = Read<uint8_t>(file);
				if(a != 0x73) {
					continue;
				}
				string s = ReadEncString(file);
				if (s != "Property") {
					continue;
				}
				C("WZ") << "Detected WZ version: " << Version << endl;
				goto wzdone;
			}
		}
	}
	C("ERROR") << "Unable to determine WZ version" << endl;
	throw(273);
	wzdone:
	file.seekg(fileStart+2);
	if (beta) {
		new Directory(this, Top);
	} else {
		new Directory(this, Top.g(name));
	}
}

uint32_t NLS::WZ::File::Hash(uint16_t enc, uint16_t real) {
	string s = tostring(real);
	int l = s.size();
	uint32_t hash = 0;
	for (int i = 0; i < l; i++) {
		hash = 32*hash+s[i]+1;
	}
	uint32_t result = 0xFF^(hash>>24)^(hash<<8>>24)^(hash<<16>>24)^(hash<<24>>24);
	if (result == enc) {
		return hash;
	} else {
		return 0;
	}
}

NLS::WZ::Directory::Directory(File* file, Node n) {
	int32_t count = ReadCInt(file->file);
	set<pair<string, uint32_t>> dirs;
	for (int i = 0; i < count; i++) {
		string name;
		uint8_t type = Read<uint8_t>(file);
		if (type == 1) {
			file->file.seekg(10, ios_base::cur);
			continue;
		} else if (type == 2) {
			int32_t s = Read<int32_t>(file);
			uint32_t p = file->file.tellg();
			file->file.seekg(file->fileStart+s);
			type = Read<uint8_t>(file);
			name = ReadEncString(file->file);
			file->file.seekg(p);
		} else if (type == 3) {
			name = ReadEncString(file->file);
		} else if (type == 4) {
			name = ReadEncString(file->file);
		} else {
			C("ERROR") << "Wat?" << endl;
			throw(273);
		}
		int32_t fsize = ReadCInt(file->file);
		int32_t checksum = ReadCInt(file->file);
		uint32_t offset = ReadOffset(file);
		if (type == 3) {
			dirs.insert(pair<string, uint32_t>(name, offset));
		} else if (type == 4) {
			name.erase(name.size()-4);
			new Image(file, n.g(name), offset);
		} else {
			C("ERROR") << "Wat?" << endl;
			throw(273);
		}
	}
	for (auto it = dirs.begin(); it != dirs.end(); it++) {
		file->file.seekg(it->second);
		new Directory(file, n.g(it->first));
	}
	delete this;
}

NLS::WZ::Image::Image(File* file, Node n, uint32_t offset) {
	this->n = n;
	n.data->image = this;
	this->offset = offset;
	this->file = file;
}

void NLS::WZ::Image::Parse() {
	file->file.seekg(offset);
	uint8_t a = Read<uint8_t>(file);
	assert(a == 0x73);
	string s = ReadEncString(file->file);
	assert(s == "Property");
	uint16_t b = Read<uint16_t>(file);
	assert(b == 0);
	new SubProperty(file, n, offset);
	function <void(Node&)> Resolve = [&](Node& n) {
		string s = n;
		if (s.substr(0, 5) == "|UOL|") {
			s.erase(0, 5);
			C("WZ") << "Resolving UOL: " << s << endl;
			//TODO: Actually resolve the UOL
		}
		for (auto it = n.Begin(); it != n.End(); it++) {
			Resolve(it->second);
		}
	};
	Resolve(n);
	delete this;
}

NLS::WZ::SubProperty::SubProperty(File* file, Node n, uint32_t offset) {
	int32_t count = ReadCInt(file->file);
	for (int i = 0; i < count; i++) {
		string name = ReadString(file->file, offset);
		uint8_t a = Read<uint8_t>(file);
		switch (a) {
		case 0x00:
			n.g(name);
			break;
		case 0x0B:
		case 0x02:
			n.g(name) = Read<uint16_t>(file);
			break;
		case 0x03:
			n.g(name) = ReadCInt(file->file);
			break;
		case 0x04:
			if (Read<uint8_t>(file) == 0x80) {
				n.g(name) = Read<float>(file);
			}
			break;
		case 0x05:
				n.g(name) = Read<double>(file);;
				break;
		case 0x08:
			n.g(name) = ReadString(file->file, offset);
			break;
		case 0x09:
			{
				uint32_t temp = Read<uint32_t>(file);
				temp += file->file.tellg();
				new ExtendedProperty(file, n.g(name), offset, temp);
				file->file.seekg(temp);
				break;
			}
		default:
			C("ERROR") << "Unknown Property type" << endl;
			throw(273);
		}
	}
	delete this;
}

NLS::WZ::ExtendedProperty::ExtendedProperty(File* file, Node n, uint32_t offset, uint32_t eob) {
	string name;
	uint8_t a = Read<uint8_t>(file);
	if (a == 0x1B) {
		int32_t inc = Read<int32_t>(file);
		uint32_t pos = offset+inc;
		uint32_t p = file->file.tellg();
		file->file.seekg(pos);
		name = ReadEncString(file->file);
		file->file.seekg(p);
	} else {
		name = ReadEncString(file->file);
	}
	if (name == "Property") {
		file->file.seekg(2, ios_base::cur);
		new SubProperty(file, n, offset);
	} else if (name == "Canvas") {
		file->file.seekg(1, ios_base::cur);
		uint8_t b = Read<uint8_t>(file);
		if (b == 1) {
			file->file.seekg(2, ios_base::cur);
			new SubProperty(file, n, offset);
		}
		n.data->sprite.data = new SpriteData;
		n.data->sprite.data->png = new PNGProperty(file, n.data->sprite);
	} else if (name == "Shape2D#Vector2D") {
		n.g("x") = ReadCInt(file->file);
		n.g("y") = ReadCInt(file->file);
	} else if (name == "Shape2D#Convex2D") {
		int32_t ec = ReadCInt(file->file);
		for (int i = 0; i < ec; i++) {
			new ExtendedProperty(file, n.g(name), offset, eob);
		}
	} else if (name == "Sound_DX8") {
		//TODO: Do something with the mp3 property
	} else if (name == "UOL") {
		file->file.seekg(1, ios_base::cur);
		uint8_t b = Read<uint8_t>(file);
		switch (b) {
		case 0:
			n.g(name) = string("|UOL|")+ReadEncString(file->file);
			break;
		case 1:
			{
				uint32_t off = Read<uint32_t>(file);
				n.g(name) = string("|UOL|")+ReadStringOffset(file->file, offset+off);
				break;
			}
		default:
			C("ERROR") << "Unknown UOL type" << endl;
			throw(273);
		}
	} else {
		C("ERROR") << "Unknown ExtendedProperty type" << endl;
		throw(273);
	}
	delete this;
}

NLS::WZ::PNGProperty::PNGProperty(File* file, Sprite spr) {
	this->file = file;
	sprite = spr;
	sprite.data->width = ReadCInt(file->file);
	sprite.data->height = ReadCInt(file->file);
	format = ReadCInt(file->file);
	format2 = Read<uint8_t>(file);
	file->file.seekg(4, ios_base::cur);
	length = Read<int32_t>(file);
	if (length <= 0) {
		C("ERROR") << "What sort of shit is this?" << endl;
		throw(273);
	}
	offset = file->file.tellg();
	offset++;
}

void NLS::WZ::PNGProperty::Parse() {
	uint8_t *data = new uint8_t[length];
	file->file.seekg(offset);
	file->file.read((char*)data, length);
	int32_t f = format+format2;
	glGenTextures(1, &sprite.data->texture);
	glBindTexture(GL_TEXTURE_2D, sprite.data->texture);
	switch (f) {
	case 1:
		{
			uint32_t len = 2*sprite.data->width*sprite.data->height;
			uint8_t *sub = Decompress(data, length, len);
			uint8_t *pixels = new uint8_t[len*2];
			for (uint32_t i = 0; i < len; i++) {
				pixels[i*2] = sub[i]&0x0F|(sub[i]&0x0F<<4);
				pixels[i*2+1] = sub[i]&0xF0|(sub[i]&0xF0>>4);
			}
			glTexImage2D(GL_TEXTURE_2D, 0, 4, sprite.data->width, sprite.data->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
			break;
		}
	case 2:
		{
			uint32_t len = 4*sprite.data->width*sprite.data->height;
			uint8_t *pixels = Decompress(data, length, len);
			glTexImage2D(GL_TEXTURE_2D, 0, 4, sprite.data->width, sprite.data->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
			break;
		}
	case 513:
		{
			uint32_t len = 2*sprite.data->width*sprite.data->height;
			uint8_t *pixels = Decompress(data, length, len);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, sprite.data->width, sprite.data->height, 0, GL_BGR, GL_UNSIGNED_SHORT_5_6_5, pixels);
			break;
		}
	case 517:
		{
			uint32_t len = sprite.data->width*sprite.data->height/128;
			uint8_t *sub = Decompress(data, length, len);
			uint8_t *pixels = new uint8_t[len*512];
			uint32_t t = 0;
			for (uint32_t i = 0; i < len; i++) {
				for (uint8_t j = 0; j < 8; j++) {
					uint8_t b = ((sub[i]&(0x01<<(7-j)))>>(7-j))*0xFF;
					for (uint8_t k = 0; k < 16; k++) {
						pixels[t] = b;
						pixels[t+1] = b;
						pixels[t+2] = b;
						pixels[t+3] = 0xFF;
						t += 4;
					}
				}
				glTexImage2D(GL_TEXTURE_2D, 0, 4, sprite.data->width, sprite.data->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			}
			break;
		}
	default:
		C("ERROR") << "Unknown sprite format" << endl;
		throw(273);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	sprite.data->loaded = true;
}
#pragma endregion

#pragma region Node Stuff
NLS::Node::Node() {
	data = 0;
}

NLS::Node::Node(const Node& other) {
	data = other.data;
}

NLS::Node& NLS::Node::operator= (const Node& other) {
	data = other.data;
	return *this;
}

NLS::Node& NLS::Node::operator[] (const string& key) {
	if (data) {
		if (data->image) {
			data->image->Parse();
			data->image = 0;
		}
		return data->children[key];
	} else {
		return WZ::Empty;
	}
}

NLS::Node& NLS::Node::operator[] (const char key[]) {
	if (data) {
		if (data->image) {
			data->image->Parse();
			data->image = 0;
		}
		return data->children[key];
	} else {
		return WZ::Empty;
	}
}

NLS::Node& NLS::Node::g(const string& key) {
	assert(data);
	Node& n = data->children[key];
	n.data = new NodeData();
	n.data->parent = *this;
	n.data->name = key;
	return n;
}

map<string, NLS::Node>::iterator NLS::Node::Begin() {
	assert(data);
	return data->children.begin();
}

map<string, NLS::Node>::iterator NLS::Node::End() {
	assert(data);
	return data->children.end();
}

NLS::Node::operator bool() {
	return (bool)data;
}

NLS::Node::operator string() {
	if (!data) {
		return string();
	}
	return data->stringValue;
}

NLS::Node::operator double() {
	if (!data) {
		return 0;
	}
	return data->floatValue;
}

NLS::Node::operator int() {
	if (!data) {
		return 0;
	}
	return data->intValue;
}

NLS::Node::operator NLS::Sprite() {
	if (!data) {
		return Sprite();
	}
	return data->sprite;
}

NLS::Node& NLS::Node::operator= (const string& v) {
	assert(data);
	data->intValue = toint(v);
	data->floatValue = todouble(v);
	data->stringValue = v;
	return *this;
}

NLS::Node& NLS::Node::operator= (const double& v) {
	assert(data);
	data->intValue = v;
	data->floatValue = v;
	data->stringValue = tostring(v);
	return *this;
}

NLS::Node& NLS::Node::operator= (const int& v) {
	assert(data);
	data->intValue = v;
	data->floatValue = v;
	data->stringValue = tostring(v);
	return *this;
}

NLS::NodeData::NodeData() {
	image = 0;
	intValue = 0;
	floatValue = 0;
}
#pragma endregion