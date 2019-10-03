#ifndef _idgen_idgen_h_
#define _idgen_idgen_h_

#include <utilfuncs/utilfuncs.h>
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <istream>
#include <sstream>

/*
	How it works:
		maintains map, MP, containing: [name_of_idtype] -> (largest_id_issued, (list_of_deleted_id's))
		prepare(name) -> creates MP[name], fail if name exists
		reset(name) -> create or clear MP[name] if it exists
		newid(name) -> returns either first deleted id, or incremented largest id
		delid(name, id) -> adds id to end of deleted ids list
*/

template<typename T> struct IDGen
{
	using IDType=T;
	typedef std::vector<IDType> IDList;
	std::map<std::string, std::pair<IDType, IDList> > MP;

	IDGen() { clear(); }
	~IDGen() {clear(); }
	IDGen(const IDGen &I) { (*this)=I; }
	
	IDGen<T>& operator=(const IDGen<T> &I) { MP=I.MP; }
	
	void clear() { MP.clear(); }
	
	//streams intended for saving & restoring...
	friend std::ostream& operator<<(std::ostream &os, const IDGen &I)
	{
		std::stringstream ss;
		if (I.MP.size())
		{ for (auto p:I.MP) {
				size_t i, n;
				n=p.second.second.size();
				ss << '[' << p.first << ',' << p.second.first << ',' << '(';
				if (n>0)
				{
					for (i=0; i<(n-1); i++) ss << p.second.second[i] << ',';
					ss << p.second.second[i];
				}
				ss << ")]";
		}}
		else ss << "[,,()]"; //or just return ""?
		os << ss.str();
		return os;
	}

	friend std::istream& operator>>(std::istream &is, IDGen &I)
	{
		std::string s, t;
		char c;
		IDType x;
		IDList v;
		bool berr=false;
		while ((is >> c)&&!berr)
		{
			s.clear(); x=0; v.clear();
			if (c=='[')
			{
				while (c!=',') { is >> c; if (c!=',') s+=c; }
				if (c==',')
				{
					is >> c;
					t.clear();
					while ((c!=',')&&!berr) { if (std::isdigit(c)) { t+=c; is >> c; } else berr=true; }
					if (!berr&&!t.empty()) x=stot<IDType>(t);
					if (!berr&&(c==','))
					{
						is >> c;
						if (c=='(')
						{
							do
							{
								is >> c;
								t.clear();
								while ((c!=',')&&(c!=')')&&!berr) { if (std::isdigit(c)) { t+=c; is >> c; } else berr=true; }
								if (!berr&&!t.empty()) v.push_back(stot<IDType>(t));
							}  while (!berr&&(c!=')'));
						}  else berr=true;
						is >> c;
						if (!berr&&(c==']')&&!s.empty()) { I.MP[s]=std::make_pair(x, v); } //ignore no-name valid buffer
					} else berr=true;
				} else berr=true;
			} else berr=true;
		}
		if (berr) I.MP.clear();
		return is;
	}
	
	bool prepare(const std::string &name) //mostly to prevent typo's when using new- & delid(..)
	{
		if (MP.find(name)!=MP.end()) return false; //exists - prevent dupes
		MP[name]={};
		return true;
	}
	
	void reset(const std::string &name)
	{
		MP[name]={};
	}
	
	IDType newid(const std::string &name)
	{
		if (MP.find(name)==MP.end()) return 0;
		if (!MP[name].second.empty()) { IDType id=MP[name].second[0]; MP[name].second.erase(MP[name].second.begin()); return id; }
		return (++MP[name].first);
	}
	
	void delid(const std::string &name, IDType id)
	{
		if (MP.find(name)==MP.end()) return;
		if ((id<=0)||(id>MP[name].first)) return;
		MP[name].second.push_back(id);
	}

};







#endif
