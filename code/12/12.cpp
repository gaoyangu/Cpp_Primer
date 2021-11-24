#include <iostream>

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>

#include <fstream>
#include <sstream>

#include<algorithm>

#include <memory>

#include "e12.h"

using namespace std;

class StrBlobPtr;

//程序需要在多个对象间共享数据
class StrBlob {
public:
	friend class StrBlobPtr;

	typedef vector<string>::size_type size_type;	// 无符号整型

	StrBlob(): data(make_shared<vector<string>>()) { }
	StrBlob(initializer_list<string> il) : data(make_shared<vector<string>>(il)) { }

	size_type size() const { return data->size(); }
	bool empty() const { return data->empty(); }

	void push_back(const string& t) {
		data->push_back(t); 
	}
	void pop_back() {
		check(0, "pop_back on empty StrBlob");
		data->pop_back();
	}
	string& front() const{
		check(0, "front on empty StrBlob");
		return data->front();
	}
	string& back() const{
		check(0, "back on empty StrBlob");
		return data->back();
	}
	StrBlobPtr begin() {
		return StrBlobPtr(*this);
	}
	StrBlobPtr end() {
		auto ret = StrBlobPtr(*this, data->size());
		return ret;
	}
private:
	shared_ptr<vector<string>> data;
	void check(size_type i, const string& msg) const{
		if (i >= data->size()) {
			throw out_of_range(msg);
		}
	}
};
class StrBlobPtr {
public:
	StrBlobPtr() : curr(0) {}
	StrBlobPtr(StrBlob& a, size_t sz = 0) : wptr(a.data), curr(sz) {}
	string& deref() const {
		auto p = check(curr, "dereference past end");
		return (*p)[curr];
	}
	StrBlobPtr& incr() {	// 前缀递增
		check(curr, "increment past end of StrBlobPtr");
		++curr;
		return *this;
	}
private:
	shared_ptr<vector<string>> check(size_t i, const string& msg) const {
		auto ret = wptr.lock();
		if (!ret) {
			throw runtime_error("unbound StrBlobPtr");
		}
		if (i >= ret->size()) {
			throw out_of_range(msg);
		}
		return ret;
	}
	weak_ptr<vector<string>> wptr;
	size_t curr;
};


string make_plural(size_t ctr, const string& word, const string& ending)
{
	return (ctr > 1) ? word + ending : word;
}
// 文本查询程序
// 允许用户在一个给定文件中查询单词
// 查询结果：单词在文件中出现的次数及其所在行的列表
// 如果一个单词在一行中出现多次，此行只列一次
// 行按照升序输出
class QueryResult {
	friend ostream& print(ostream& os, const QueryResult& qr) {
		os << qr.sought << " occurs " << qr.lines->size() << " " << make_plural(qr.lines->size(), "time", "s") << endl;
		for (auto num : *qr.lines) {
			os << "\t(line " << num + 1 << ") " << *(qr.file->begin() + num) << endl;
		}
		return os;
	}
public:
	using line_no = vector<string>::size_type;
	QueryResult(string s, shared_ptr<set<line_no>> p, shared_ptr<vector<string>> f) : sought(s), lines(p), file(f){

	}
private:
	string sought;
	shared_ptr<set<line_no>> lines;
	shared_ptr<vector<string>> file;
};
class TextQuery {
public:
	using line_no = vector<string>::size_type;
	TextQuery(ifstream& is) : file(new vector<string>) {
		string text;
		while (getline(is, text)) {	//对文件中的每一行
			file->push_back(text);
			int n = file->size() - 1;	//当前行号

			//将行文本分解为单词
			istringstream line(text);
			string word;
			while (line >> word) {
				auto& lines = wm[word];
				if (!lines) {
					lines.reset(new set<line_no>);
				}
				lines->insert(n);
			}
		}
	}
	QueryResult query(const string& sought) const {
		static shared_ptr<set<line_no>> nodata(new set<line_no>);
		auto loc = wm.find(sought);
		if (loc == wm.end()) {
			return QueryResult(sought, nodata, file);
		}
		else {
			return QueryResult(sought, loc->second, file);
		}
	}
private:
	shared_ptr<vector<string>> file;	// 输入文件
	map<string, shared_ptr<set<line_no>>> wm;	// 每个单词到它所在的行号的集合的映射
};
void runQueries(ifstream& infile) {
	// 保存文件并建立查询map
	TextQuery tq(infile);
	while (true) {
		cout << "enter word to look for, or q to quit: ";
		string s;
		if (!(cin >> s) || s == "q") break;
		print(cout, tq.query(s)) << endl;
	}
}


int main()
{
	shared_ptr<string> p1;
	shared_ptr<list<int>> p2;
	// 如果 p1 不为空，检查它是否指向一个空string
	if (p1 && p1->empty())
		*p1 = "hi";

	shared_ptr<int> p3 = make_shared<int>(42);
	shared_ptr<string> p4 = make_shared<string>(10, '9');
	shared_ptr<int> p5 = make_shared<int>();
	auto p6 = make_shared<vector<string>>();

	auto p = make_shared<int>(42);	//p 指向的对象只有 p 一个引用者
	auto q(p);	//p 和 q 指向相同对象，此对象有两个引用者
	auto r = make_shared<int>(42);	//r 指向的 int 只有一个引用者
	r = p;	//给 r 赋值，令它指向另一个地址
			//递增 q 指向的对象的引用计数
			//递减 r 原来指向的对象的引用计数
			//r 原来指向的对象已没有引用者，会自动释放

	//程序需要在多个对象间共享数据
	StrBlob b1;
	{
		StrBlob b2 = { "a", "an", "the" };
		b1 = b2;
		b2.push_back("the");
	}
	cout << b1.size();

	auto pV = createVector();
	assignVector(pV);
	printVector(pV);
	delete pV;

	auto pV2 = cVector();
	aVector(pV2);
	pVector(pV2);

	cin.get();
}