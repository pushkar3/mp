#ifndef PALLETIZING_H_
#define PALLETIZING_H_

#include <stdio.h>
#include <math.h>
#include <vector>
#include <map>
#include <3dbpp.h>

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

	std::vector<int> add(std::vector<int> vec1, std::vector<int> vec2) {
		std::vector<int> ret(vec1.size());
		for (uint i = 0; i < vec1.size(); i++) {
			ret[i] = vec1[i] + vec2[i];
		}
		return ret;
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
		std::multimap<key, pattern>::iterator itd;
		double d = distance(order);
		double min = 99999.99f;
		std::vector<int> min_vec;
		std::vector<int> min_pattern;
		std::vector<int> min_dims;
		std::vector<int> pallet_key(db->package_info.size(), 0);
		std::vector<int> pallet_pattern;
		std::vector<int> pallet_dimensions(db->package_info.size(), 0);
		int layer_d = 0;

		while (d > 0) {
			itp = db->layer_pattern.begin();
			itd = db->layer_dimensions.begin();
			for (; itp!= db->layer_pattern.end(); itp++, itd++) {
				double cost = distance(diff(order, (*itp).first));

				if (cost >=0 && cost < min) {
					min = cost;
					min_vec = (*itp).first;
					min_pattern = (*itp).second;
					min_dims = (*itd).second;
				}
			}

			show("min_vec", min_vec);

			order = diff(order, min_vec);
			pallet_key = add(pallet_key, min_vec);

			int c = 0;
			for (uint i = 0; i < pallet_key.size(); i++) {
				for (uint j = 0; j < min_vec[i]; j++) {
					pallet_pattern.push_back(min_pattern[c*3+0]);
					pallet_pattern.push_back(min_pattern[c*3+1]);
					pallet_pattern.push_back(min_pattern[c*3+2] + layer_d);

					int w = min_pattern[c * 3 + 0] + db->package_info[i].w;
					int h = min_pattern[c * 3 + 1] + db->package_info[i].h;
					int d = min_pattern[c * 3 + 2] + db->package_info[i].d;

					if (w < pallet_dimensions[0]) pallet_dimensions[0] = w;
					if (h < pallet_dimensions[1]) pallet_dimensions[0] = h;
					if (d < pallet_dimensions[2]) pallet_dimensions[0] = d;

					c++;
				}
			}

			for (uint i = 0; i < order.size(); i++) {
				if(order[i] < 0) order[i] = 0;
			}

			d = distance(order);
			layer_d += min_dims[2];
		}

		db->layer_pattern.insert(std::pair<key, pattern>(pallet_key, pallet_pattern));
		db->layer_cost.insert(std::pair<key, cost>(pallet_key, 0.0f));
		db->layer_dimensions.insert(std::pair<key, pattern>(pallet_key, pallet_dimensions));
		std::cout << "Exported pallet" << std::endl;
		db->exportdb();

	}

};

#endif /* PALLETIZING_H_ */
