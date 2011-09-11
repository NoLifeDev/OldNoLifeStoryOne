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
uint16_t Version = 0;
uint32_t VersionHash;
uint8_t Buf1[0x1000000];
uint8_t Buf2[0x1000000];

#pragma region ZLib Stuff
void Decompress(uint32_t inLen, uint32_t outLen){
	int err = Z_OK;
	z_stream strm;
	strm.next_in = Buf2;
	strm.avail_in = inLen;
	strm.opaque = nullptr;
	strm.zfree = nullptr;
	strm.zalloc = nullptr;
	inflateInit(&strm);
	strm.next_out = Buf1;
	strm.avail_out = outLen;
	err = inflate(&strm, Z_NO_FLUSH);
	switch(err){
	case Z_OK:
	case Z_STREAM_END:
		break;
	default:
		NLS::C("ERROR") << "I hate zlib!" << endl;
		throw(273);
	}
	if (strm.total_out != outLen) {
		NLS::C("ERROR") << "I hate zlib!" << endl;
		throw(273);
	}
	inflateEnd(&strm);
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
		new File("Data", Top);
	} else {
		C("WZ") << "Loading standard WZ file structure" << endl;
		new File("Base", Top);
	}
	return true;
}

NLS::WZ::File::File(const string& name, Node n) {
	string filename = Path+name+".wz";
	file.open(filename, file.in|file.binary);
	if (!file.is_open()) {
		C("ERROR") << "Failed to load " << name << ".wz" << endl;
		return;
		//throw(273);
	}
	Files.insert(this);
	ident.resize(4);
	file.read(const_cast<char*>(ident.c_str()), 4);
	fileSize = Read<uint64_t>(file);
	fileStart = Read<uint32_t>(file);
	file >> copyright;
	file.seekg(fileStart);
	if (!Version) {
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
		bool success = false;
		for (uint8_t j = 0; j < 2 and !success; j++) {
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
					success = true;
					break;
				}
			}
		}
		if (!success) {
			C("ERROR") << "Unable to determine WZ version" << endl;
			throw(273);
		}
		file.seekg(fileStart+2);
	} else {
		int16_t eversion = Read<int16_t>(file);
		if (eversion != EncVersion) {
			C("ERROR") << "Version of WZ file does not match existing files" << endl;
		}
	}
	new Directory(this, n);
	for (auto it = threads.begin(); it != threads.end(); it++) {
		(*it)->Wait();
		delete *it;
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
	if (count == 0) {
		sf::Thread* t = new sf::Thread([](Node n){new File(n.data->name, n);}, n);
		t->Launch();
		file->threads.insert(t);
		return;
	}
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
			if (it->second.data) {
				Resolve(it->second);
			}
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
		n.data->sprite.data->originx = n["origin"]["x"];
		n.data->sprite.data->originy = n["origin"]["y"];
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
	sprite.data->loaded = false;
	sprite.data->width = ReadCInt(file->file);
	sprite.data->height = ReadCInt(file->file);
	uint32_t w, h;
	for (w = 1; w < sprite.data->width; w <<= 1) {}
	for (h = 1; h < sprite.data->height; h <<= 1) {}
	sprite.data->twidth = (float)sprite.data->width/w;
	sprite.data->theight = (float)sprite.data->height/h;
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
	file->file.seekg(offset);
	file->file.read((char*)Buf2, length);
	int32_t f = format+format2;
	glGenTextures(1, &sprite.data->texture);
	glBindTexture(GL_TEXTURE_2D, sprite.data->texture);
	switch (f) {
	case 1:
		{
			uint32_t len = 2*sprite.data->width*sprite.data->height;
			Decompress(length, len);
			for (uint32_t i = 0; i < len; i++) {
				Buf2[i*2] = (Buf1[i]&0x0F)|((Buf1[i]&0x0F)<<4);
				Buf2[i*2+1] = (Buf1[i]&0xF0)|((Buf1[i]&0xF0)>>4);
			}
			glTexImage2D(GL_TEXTURE_2D, 0, 4, sprite.data->width, sprite.data->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, Buf2);
			break;
		}
	case 2:
		{
			uint32_t len = 4*sprite.data->width*sprite.data->height;
			Decompress(length, len);
			glTexImage2D(GL_TEXTURE_2D, 0, 4, sprite.data->width, sprite.data->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, Buf1);
			break;
		}
	case 513:
		{
			uint32_t len = 2*sprite.data->width*sprite.data->height;
			Decompress(length, len);
			glTexImage2D(GL_TEXTURE_2D, 0, 4, sprite.data->width, sprite.data->height, 0, GL_BGR, GL_UNSIGNED_SHORT_5_6_5, Buf1);
			break;
		}
	case 517:
		{
			uint32_t len = sprite.data->width*sprite.data->height/128;
			Decompress(length, len);
			uint32_t t = 0;
			for (uint32_t i = 0; i < len; i++) {
				for (uint8_t j = 0; j < 8; j++) {
					uint8_t b = ((Buf1[i]&(0x01<<(7-j)))>>(7-j))*0xFF;
					for (uint8_t k = 0; k < 16; k++) {
						Buf2[t] = b;
						Buf2[t+1] = b;
						Buf2[t+2] = b;
						Buf2[t+3] = 0xFF;
						t += 4;
					}
				}
				glTexImage2D(GL_TEXTURE_2D, 0, 4, sprite.data->width, sprite.data->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Buf2);
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

NLS::Node& NLS::Node::operator[] (const int& key) {
	return (*this)[tostring(key)];
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