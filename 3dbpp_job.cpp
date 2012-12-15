#include <3dbpp.h>

struct configcomp {
	static bool compare(config_t c1, config_t c2, double tolerance) {
		double diff = fabs(c1.density() - c2.density())/max(c1.density(), c2.density());
		return (diff < tolerance && c1.is_bound() && c2.is_bound());
	}
};


binpack_job::binpack_job() {
	package_key = -1;
	is_singlepackage = 0;
	h_max = 0.0f;
	completed = 0;
}

binpack_job::~binpack_job() { }

int binpack_job:: is_singlepack() {
	return is_singlepackage;
}

double binpack_job::get_height() {
	return h_max;
}

int binpack_job::get_packagekey() {
	return package_key;
}

void binpack_job::set_singlepackage(database* db, int _key, int _hmax) {
	tolerance = param.get("single_package_layering_threshold");
	is_singlepackage = 1;
	package_key = _key;
	h_max = _hmax;
	d1 = db->clone();
	d1.config_map.clear();
	d1.layer_map.clear();

	vector<int> key(d1.package.size(), 0);
	vector<int> pattern(3, 0);
	vector<int> orientation(1, 0);
	key[package_key] = 1;
	d1.insert(key, pattern, orientation);
	orientation[0] = 1;
	d1.insert(key, pattern, orientation);
}

void binpack_job::set_multiplepackage(database db1, database db2, double _hmax) {
	tolerance = param.get("multiple_package_layering_threshold");
	h_max = _hmax;
	d1 = db1.clone();
	d2 = db2.clone();
}

void binpack_job::singlepackageconfig() {
	multimap<key_, config_t>::iterator it1;
	multimap<key_, config_t>::iterator it2;

	for (it1 = d1.config_map.begin(); it1 != d1.config_map.end(); it1++) {
		for (it2 = d1.config_map.begin(); it2 != d1.config_map.end(); it2++) {
			for (uint i = 0; i < 3; i++) {
				config_t c1 = (*it1).second;
				config_t c2 = (*it2).second;
				for (int j = 0; j < 2; j++) {
					if (j == 0) c1.change_orientation();
					c2.set_origin(c1.get_corner(i));
					c1.add(&d1, c2);

					if (configcomp::compare(c1, c2, tolerance) && c1.get_height() <= h_max	&& c2.get_height() <= h_max) {
						d1.insert(c1.get_key(), c1.get_pattern(), c1.get_orientation());
					}
				}
			}
		}
	}
	d = d1.clone();
}

void binpack_job::multiplepackageconfig() {
	multimap<key_, config_t>::iterator it1;
	multimap<key_, config_t>::iterator it2;

	long int n = 0;
	for (it1 = d1.config_map.begin(); it1 != d1.config_map.end(); it1++) {
		for (it2 = d2.config_map.begin(); it2 != d2.config_map.end(); it2++) {
			n++;
			for (uint i = 0; i < 3; i++) {
				config_t c1 = (*it1).second;
				config_t c2 = (*it2).second;
				for (int j = 0; j < 2; j++) {
					if (j == 0) c1.change_orientation(&d1);
					c2.set_origin(c1.get_corner(i));
					c1.add(&d1, c2);

					if (configcomp::compare(c1, c2, tolerance) && c1.get_height() <= h_max && c2.get_height() <= h_max) {
						d1.insert(c1.get_key(), c1.get_pattern(), c1.get_orientation());
					}
				}
			}
		}

		if(n > (int) param.get("max_configmap_ittr")) {
			cout << RED"Reached computation limit!"NORMAL << endl;
			break;
		}
	}

	for (it1 = d1.config_map.begin(); it1 != d1.config_map.end(); it1++) {
		for (it2 = d1.config_map.begin(); it2 != d1.config_map.end(); it2++) {
			n++;
			for (uint i = 0; i < 3; i++) {
				config_t c1 = (*it1).second;
				config_t c2 = (*it2).second;
				for (int j = 0; j < 2; j++) {
					if (j == 0) c1.change_orientation(&d1);
					c2.set_origin(c1.get_corner(i));
					c1.add(&d1, c2);

					if (configcomp::compare(c1, c2, tolerance) && c1.get_height() <= h_max	&& c2.get_height() <= h_max) {
						d1.insert(c1.get_key(), c1.get_pattern(), c1.get_orientation());
					}
				}
			}
		}

		if(n > (int) param.get("max_configmap_ittr")) {
			cout << RED"Reached computation limit!"NORMAL << endl;
			break;
		}
	}

	d = d1.clone();
}

void binpack_job::run() {
	if (completed) return;
	if(is_singlepackage)
		singlepackageconfig();
	else {
		multiplepackageconfig();
	}
	completed = 1;
}
