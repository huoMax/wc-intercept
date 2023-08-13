/*
 * @Author: huomax 630509357@qq.com
 * @Date: 2023-08-05 20:29:17
 * @LastEditors: huomax 630509357@qq.com
 * @LastEditTime: 2023-08-05 20:57:10
 * @FilePath: /wc-intercept/test.cc
 * @Description: 
 * 
 * Copyright (c) 2023 by huomax, All Rights Reserved. 
 */
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

bool cmp(const vector<int>& a, const vector<int>& b) {
	return a[0] < b[0]; // 按开始时间排序
}

int maxPossibility(std::vector<std::vector<int>> &interviews, int k);

int main() {
	vector<vector<int>> interviews = {{1, 2, 3}, {3, 4, 2}, {2, 4, 6}};
	int k = 2;
	int res = maxPossibility(interviews, k);
	cout << res << endl;
	return 0;
}

int maxPossibility(std::vector<std::vector<int>> &interviews, int k) {
    std::sort(interviews.begin(), interviews.end(), cmp);
	int result = 0;

	for (int i=0; i<interviews.size(); ++i) {
		int start = i;
		int tmp = 0;
		for (int j=i+1; j<interviews.size(); ++j) {
			if (interviews[j][0]<=interviews[start][1]) continue;
			tmp += interviews[start][2];
			start = j;
			// std::cout << "i:" << i << " j: " << j <<std::endl;
		}
		tmp += interviews[start][2];
		result = std::max(tmp, result);
	}

    return result;
}

