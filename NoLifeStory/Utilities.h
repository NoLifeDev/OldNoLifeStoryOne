////////////////////////////////////////////////////
// This file is part of NoLifeStory.              //
// Please see SuperGlobal.h for more information. //
////////////////////////////////////////////////////

#define PI 3.1415926535897932384626433832795
#define degtorad 0.01745329251994329576923690768489
#define radtodeg 57.295779513082320876798154814105

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

inline double pdir(double x1, double y1, double x2, double y2) {
	return radtodeg*atan2(y2-y1, x2-x1);
}

inline double ldx (double len, double dir) {
	return cos(dir*degtorad)*len;
}

inline double ldy (double len, double dir) {
	return sin(dir*degtorad)*len;
}

inline double angdif (double angle1, double angle2) {
	return fmod(fmod(angle1-angle2, 360)+540, 360)-180;
}

inline double sign (double x) {
	return x>0?1:x<0?-1:0;
}