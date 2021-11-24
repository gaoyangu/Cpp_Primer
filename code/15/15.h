#pragma once
#include <iostream>
#include <fstream>
#include <set>

using namespace std;

// 基类：按原价销售的书籍
class Quote {
public:
	Quote() = default;
	Quote(const string& book, double sales_price) : bookNo(book), price(sales_price){}

	// 返回当前对象的一份动态分配的拷贝
	virtual Quote* clone() const& { return new Quote(*this); }
	virtual Quote* clone()&& { return new Quote(move(*this)); }

	// 返回书籍的ISBN编号
	string isbn() const { return bookNo; }

	// 返回书籍的实际销售价格
	virtual double net_price(size_t n) const { return n * price; }

	virtual ~Quote() = default;
private:
	string bookNo;	// 书籍的ISBN编号
protected:
	double price = 0.0;	// 普通状态下不打折的价格
};

// 派生类：可以打折出售的书籍
class Bulk_quote : public Quote{
public:
	Bulk_quote() = default;
	Bulk_quote(const string& book, double p, size_t qty, double disc) : Quote(book, p), min_qty(qty), discount(disc){ }

	Bulk_quote* clone() const& { return new Bulk_quote(*this); }
	Bulk_quote* clone()&& { return new Bulk_quote(move(*this)); }

	double net_price(size_t n) const override;
private:
	size_t min_qty = 0;		// 适用折扣政策的最低购买量
	double discount = 0.0;	// 折扣额
};

class Disc_quote : public Quote{
public:
	Disc_quote() = default;
	Disc_quote(const string& book, double price, size_t qty, double disc): Quote(book,price), quantity(qty), discount(disc) { }

	double net_price(size_t) const = 0;

protected:
	size_t quantity = 0;		// 折扣适用的购买量
	double discount = 0.0;	// 折扣
};

double print_total(ostream& os, const Quote& item, size_t n);

class Basket {
public:
	void add_item(const shared_ptr<Quote>& sale) {
		items.insert(sale);
	}
	void add_item(const Quote& sale) {
		items.insert(shared_ptr<Quote>(sale.clone()));
	}
	void add_item(const Quote&& sale) {
		items.insert(shared_ptr<Quote>(move(sale).clone()));
	}
	// 打印每本书的总价和购物篮中所有书的总价
	double total_receipt(ostream&) const;
private:
	// 该函数用于比较 shared_ptr
	static bool compare(shared_ptr<Quote>& lhs, shared_ptr<Quote>& rhs) {
		return lhs->isbn() < rhs->isbn();
	}
	// multiset 保存多个报价，按照 compare 成员排序
	multiset<shared_ptr<Quote>, decltype(compare)*> items{ compare };
};


// 抽象类， 具体的查询类型从中派生
class Query_base {
	friend class Query;
protected:
	using line_no = TextQuery::line_no;	// 用于eval函数
	virtual ~Query_base() = default;
private:
	// eval 返回与当前Query匹配的QueryResult
	virtual QueryResult eval(const TextQuery&) const = 0;
	// rep 表示查询的一个string
	virtual string rep() const = 0;
};

// 管理Query_base继承体系的接口类
class Query {
	friend Query operator~(const Query&);
	friend Query operator|(const Query&, const Query&);
	friend Query operator&(const Query&, const Query&);
public:
	Query(const string& s) : q(new WordQuery(s)){ }

	QueryResult eval(const TextQuery& t) const {
		return q->eval(t);
	}
	string rep() const {
		return q->rep();
	}

private:
	Query(shared_ptr<Query_base> query) : q(query){ }
	shared_ptr<Query_base> q;
};
ostream& operator<<(ostream& os, const Query& query) {
	return os << query.rep();
}

class WordQuery : public Query_base {
	friend class Query;
	WordQuery(const string& s) : query_word(s){ }

	QueryResult eval(const TextQuery& t) const {
		return t.query(query_word);
	}
	string rep() const {
		return query_word;
	}

	string query_word;
};

class NotQuery : public Query_base {
	friend Query operator~(const Query& operand) {
		return  shared_ptr<Query_base>(new NotQuery(operand));
	}
	NotQuery(const Query& q) : query(q) { }

	QueryResult eval(const TextQuery& text) const {
		auto result = query.eval(text);
		auto ret_lines = make_shared<set<line_no>>();
		auto beg = result.begin();
		auto end = result.end();
		auto sz = result.get_file()->size();
		for (size_t n = 0; n != sz; n++) {
			if (beg == end || *beg != n) {
				ret_lines->insert(n);
			}
			else if(beg != end){
				++beg;
			}
		}
		return QueryResult(rep(), ret_lines, result.get_file());
	}
	string rep() const {
		return "~(" + query.rep() + ")";
	}

	Query query;
};

class BinaryQuery : public Query_base {
protected:
	BinaryQuery(const Query& l, const Query& r, string s) : lhs(l), rhs(r), opSym(s) { }
	
	string rep() const {
		return "(" + lhs.rep() + " " + opSym + " " + rhs.rep() + ")";
	}

	Query lhs, rhs;
	string opSym;
};

class AndQuery : public BinaryQuery {
	friend Query operator&(const Query& lhs, const Query& rhs) {
		return shared_ptr<Query_base>(new AndQuery(lhs, rhs));
	}
	AndQuery(const Query& left, const Query& right) : BinaryQuery(left, right, "&") { }

	QueryResult eval(const TextQuery& text) const {
		auto left = lhs.eval(text);
		auto right = lhs.eval(text);
		auto ret_lines = make_shared<set<line_no>>();
		set_intersection(left.begin(), left.end(), right.begin(), right.end(), inserter(*ret_lines, ret_lines->begin()));
		return QueryResult(rep(), ret_lines, left.get_file());
	}
};

class OrQuery : public BinaryQuery {
	friend Query operator|(const Query& lhs, const Query& rhs) {
		return shared_ptr<Query_base>(new OrQuery(lhs, rhs));
	}
	OrQuery(const Query& left, const Query& right) : BinaryQuery(left, right, "|") { }

	QueryResult eval(const TextQuery& text) const {
		auto right = rhs.eval(text);
		auto left = lhs.eval(text);
		auto ret_lines = make_shared<set<line_no>>(left.begin(), left.end());
		ret_lines->insert(right.begin(), right.end());
		return QueryResult(rep(), ret_lines, left.get_file());
	}
};