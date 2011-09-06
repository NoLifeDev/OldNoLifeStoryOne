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
uint8_t GMSKeyIV[4] = {0x4D, 0x23, 0xC7, 0x2B};
uint32_t OffsetKey = 0x581C3F6D;

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
	p = (p-file->head->fileStart)^0xFFFFFFFF;
	p *= file->head->versionHash;
	p -= OffsetKey;
	p = (p<<(p&0x1F))|(p>>(32-p&0x1F));
	uint32_t more = Read<uint32_t>(file);
	p ^= more;
	p += file->head->fileStart*2;
	return p;
}

inline string ReadEncString(ifstream& file) {
	int8_t slen = Read<int8_t>(file);
	if (slen == 0) {
		return string();
	} else if (slen > 0) {
		int32_t len;
		if (slen == 0x7F) {
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
bool NLS::WZ::Init(string path) {
	Path = path;
	Top.data = new NodeData();
	ifstream test(path+"Data.wz");
	bool beta = test.is_open();
	test.close();
	if (beta) {
		C("WZ") << "Loading beta WZ files" << Endl;
		WZKey = new uint8_t[0xFFFF];
		memset(WZKey, 0, 0xFFFF);
		new File("Data");
	} else {
		C("WZ") << "Loading standard WZ files" << Endl;
		WZKey = GMSKey;
		new File("Base");
		new File("Character");
		new File("Effect");
		new File("Etc");
		new File("Item");
		new File("Map");
		new File("Mob");
		new File("Morph");
		new File("Npc");
		new File("Quest");
		new File("Reactor");
		new File("Skill");
		new File("Sound");
		new File("String");
		new File("TamingMob");
		new File("UI");
	}
	return true;
}

NLS::WZ::File::File(string name) {
	string filename = Path+name+".wz";
	file.open(filename, file.in|file.binary);
	C("WZ") << "Loading file: " << filename << Endl;
	if (!file.is_open()) {
		C("ERROR") << "Failed to load WZ file" << Endl;
		throw(273);
	}
	Files.insert(this);
	head = new Header(this);
	version = 0;
	for (uint16_t i = 40; i < 120; i++) {
		uint32_t vh = Hash(head->version, i);
		if (vh) {
			//TODO: Add proper support for detecting which version is the correct version
			if (version) {
				C("ERROR") << "Conflicting WZ versions: " << version << " and " << i << Endl;
				throw(273);
			}
			head->versionHash = vh;
			version = i;
		}
	}
	if (name == "Data") {
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

NLS::WZ::Header::Header(File* file) {
	char s1[4];
	file->file.read(s1, 4);
	ident.assign(s1, 4);
	fileSize = Read<uint64_t>(file);
	fileStart = Read<uint32_t>(file);
	file->file >> copyright;
	file->file.seekg(fileStart);
	version = Read<int16_t>(file);
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
			file->file.seekg(file->head->fileStart+s);
			type = Read<uint8_t>(file);
			name = ReadEncString(file->file);
			file->file.seekg(p);
		} else if (type == 3) {
			name = ReadEncString(file->file);
		} else if (type == 4) {
			name = ReadEncString(file->file);
		} else {
			continue;
		}
		int32_t fsize = ReadCInt(file->file);
		int32_t checksum = ReadCInt(file->file);
		uint32_t offset = ReadOffset(file);
		if (type == 3) {
			dirs.insert(pair<string, uint32_t>(name, offset));
		} else {
			name.erase(name.size()-4);
			new Image(file, n.g(name), offset);
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
			C("WZ") << "Resolving UOL: " << s << Endl;
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
			C("ERROR") << "Unknown Property type" << Endl;
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
		//TODO: Do something with the png property
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
			C("ERROR") << "Unknown UOL type" << Endl;
			throw(273);
		}
	} else {
		C("ERROR") << "Unknown ExtendedProperty type" << Endl;
		throw(273);
	}
	delete this;
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