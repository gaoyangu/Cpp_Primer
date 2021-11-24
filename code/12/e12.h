#pragma once
#include <iostream>
#include <vector>
using namespace std;

// 练习12.6
vector<int>* createVector() {
	return new vector<int>();
}
void assignVector(vector<int>* p) {
	int a;
	while (cin >> a) {
		p->push_back(a);
	}
}
void printVector(vector<int>* p) {
	for (auto i : *p) {
		cout << i << " ";
	}
	cout << endl;
}

// 练习12.7
shared_ptr<vector<int>> cVector() {
	return make_shared<vector<int>>();
}
void aVector(shared_ptr<vector<int>> p) {
	int a; 
	while (cin >> a) {
		p->push_back(a);
	}
}
void pVector(shared_ptr<vector<int>> p) {
	for (auto i : *p) {
		cout << i << " ";
	}
	cout << endl;
}