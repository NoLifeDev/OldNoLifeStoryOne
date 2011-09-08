////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

namespace NLS {
	//Node classes
	class Node {
	public:
		Node();
		Node(const Node&);
		Node& operator= (const Node&);
		Node& operator[] (const string&);
		Node& operator[] (const char[]);
		Node& g(const string&);
		map<string, Node>::iterator Begin();
		map<string, Node>::iterator End();
		operator bool();
		operator string();
		operator double();
		operator int();
		operator Sprite();
		Node& operator= (const string&);
		Node& operator= (const double&);
		Node& operator= (const int&);
		class NodeData* data;
	};
	namespace WZ {
		extern Node Top;
		extern Node Empty;
		//And now lets define them somehow
		class File {
		public:
			File(const string& name);
			File(const string& name, bool beta);
			ifstream file;
			uint32_t Hash(uint16_t enc, uint16_t real);
			string ident;
			uint64_t fileSize;
			uint32_t fileStart;
			string copyright;
		};
		class Directory {
		public:
			Directory(File* file, Node n);
		};
		class Image {
		public:
			Image(File* file, Node n, uint32_t offset);
			void Parse();
			Node n;
			string name;
			uint32_t offset;
			File* file;
		};
		class SubProperty {
		public:
			SubProperty(File* file, Node n, uint32_t offset);
		};
		class ExtendedProperty {
		public:
			ExtendedProperty(File* file, Node n, uint32_t offset, uint32_t eob);
		};
		class PNGProperty {
		public:
			PNGProperty(File* file);
			void Parse();
		};
		class SoundProperty {
		public:
		};
		//Functions
		bool Init(const string& path);
	}
	class NodeData {
	public:
		NodeData();
		string stringValue;
		double floatValue;
		int intValue;
		Sprite sprite;
		Node parent;
		string name;
		map <string, Node> children;
		WZ::Image* image;
	private:
		NodeData(const NodeData&);
		NodeData& operator= (const NodeData&);
	};
};