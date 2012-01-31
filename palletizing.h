#ifndef PALLETIZING_H_
#define PALLETIZING_H_

#include <stdio.h>
#include <math.h>
#include <3dbpp_settings.h>
#include <vector>
#include <map>

class palletizing {
	database* db;

	double distance(std::vector<int> vec) {
		double d = 0;
		for (int i = 0; i < vec.size(); i++) {
			d += (vec[i] * vec[i]);
		}
		return sqrt(d);
	}

	double angle(std::vector<int> vec1, std::vector<int> vec2) {
		double a = 0;
		if (vec1.size() != vec2.size()) {
			std::cerr << "Vector sizes are not equal" << std::endl;
			return 0;
		}
		double d1 = distance(vec1);
		double d2 = distance(vec2);
		for (int i = 0; i < vec1.size(); i++) {
			a += vec1[i] * vec2[i];
		}

		return acos(a / (d1 * d2));
	}

	std::vector<int> diff(std::vector<int> vec1, std::vector<int> vec2) {
		std::vector<int> ret(vec1.size());
		if (vec1.size() != vec2.size()) {
			std::cerr << "Vector sizes are not equal" << std::endl;
			return ret;
		}

		for (int i = 0; i < vec1.size(); i++) {
			ret[i] = vec1[i] - vec2[i];
		}
	}

	void show(const char* str, std::vector<int> vec) {
		std::cout << str << " ";
		for (int i = 0; i < vec.size(); i++) {
			std::cout << vec[i] << " ";
		}
		std::cout << std::endl;
	}

public:
	palletizing(database* _d) {
		db = _d;
	}

	~palletizing() {

	}

	void solve() {
		std::vector<int> order;
		for (int i = 0; i < db->package_info.size(); i++) {
			int order_key = 0;
			for (int j = 0; j < db->order_info.size(); j++) {
				if (db->order_info[j].id == db->package_info[i].id) {
					order_key = db->order_info[j].quantity;
					break;
				}
			}
			order.push_back(order_key);
		}

		std::multimap<key, pattern>::iterator itp;
		double d = distance(order);
		double min = 99999.99f;
		std::vector<int> min_vec;
		int layer_height = 0;
		int max_height = 0;

		while (d > 0) {
			max_height = 0;

			for (itp = db->layer_pattern.begin(); itp!= db->layer_pattern.end(); itp++) {
				double cost = distance(diff(order, (*itp).first));
				// todo
				// check the cost > 0
				if (cost < min) {
					min = cost;
					min_vec = (*itp).first;
				}
			}

			show("min_vec", min_vec);

			int n = 0;
			std::vector<int> pos = db->layer_pattern[min_vec];
			for (int i = 0; i < min_vec.size(); i++) {
				for(int j = 0; j < min_vec[i]; j++) {
					int x = pos[n++];
					int y = pos[n++];
					int z = pos[n++]+layer_height;
					package p(i, db->package_info[i].w,
							db->package_info[i].h, db->package_info[i].d,
							x, y, z);
					db->packlist.push_back(p);
				}

				if(min_vec[i] > 0 && max_height < db->package_info[i].h)
					max_height = db->package_info[i].h;
			}

			order = diff(order, min_vec);

			for (int i = 0; i < order.size(); i++) {
				if(order[i] < 0) order[i] = 0;
			}

			d = distance(order);
			layer_height += max_height;
		}

	}

};

#endif /* PALLETIZING_H_ */
