#include "15_oop.h"

double print_total(std::ostream& os, const Quote& item, std::size_t n) {
	double ret = item.net_price(n);
	os << "ISBN: " << item.isbn() << " # sold: " << n << " total due: " << ret << endl;
	return ret;
}

double Bulk_quote::net_price(size_t n) const {
	if (n >= min_qty) {
		return n * (1 - discount) * price;
	}
	return n * price;
}

double Basket::total_receipt(ostream& os) const {
	double sum = 0.0;	// 保存实时计算出的价格
	// iter 指向 isbn 相同的一批元素中的第一个
	// upper_bound 返回一个迭代器，该迭代器指向这批元素的尾后位置
	for (auto iter = items.cbegin(); iter != items.cend(); iter = items.upper_bound(*iter)) {
		sum += print_total(os, **iter, items.count(*iter));
	}
	os << "Total Sale: " << sum << endl;
	return sum;
}