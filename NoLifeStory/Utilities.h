////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

inline string tostring(const double& t) {
	stringstream ss;
	ss << t;
	return ss.str();
}

inline string tostring(const int& t) {
	stringstream ss;
	ss << t;
	return ss.str();
}

inline string tostring(const int& t, const int& len) {
	stringstream ss;
	ss << t;
	string s = ss.str();
	return s.insert(0, len-s.size(), '0');
}

inline double todouble(const string& t) {
	stringstream ss;
	ss << t;
	double d;
	ss >> d;
	return d;
}

inline int toint(const string& t) {
	stringstream ss;
	ss << t;
	int d;
	ss >> d;
	return d;
}

inline double sqr(double x) {
	return x*x;
}

inline double pdis(double x1, double y1, double x2, double y2) {
	return sqrt(sqr(x1-x2)+sqr(y1-y2));
}