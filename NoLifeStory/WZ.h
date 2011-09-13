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
		Node& operator[] (const int&);
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
		void File(Node n);
		class Image {
		public:
			Image(ifstream* file, Node n, uint32_t offset);
			void Parse();
			Node n;
			uint32_t offset;
			ifstream* file;
		};
		class PNGProperty {
		public:
			PNGProperty(ifstream* file, Sprite spr);
			void Parse();
			ifstream* file;
			Sprite sprite;
			int32_t format;
			uint8_t format2;
			int32_t length;
			uint32_t offset;
		};
		class SoundProperty {
		public:
		};
		void SubProperty(ifstream* file, Node n, uint32_t offset);
		void ExtendedProperty(ifstream* file, Node n, uint32_t offset);
		void Init(const string& path);
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