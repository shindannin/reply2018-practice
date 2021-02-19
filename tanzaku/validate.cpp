#include <domain/common.h>

int main(int argc, char *argv[])
{
  cin.tie(0);
  ios::sync_with_stdio(false);

  std::ifstream in(argv[1]);
  std::ifstream ans(argv[2]);

  if (!in.is_open() || !ans.is_open()) {
    throw;
  }

  input(in);

  map<pair<int, int>, int> mp;
  for (int i = 0; i < R; i++) {
    mp[make_pair(regions[i].provider_index, regions[i].region_index)] = i;
  }

  int64_t max_value = 0;
  int64_t min_value = 1LL << 60;
  for (int p = 0; p <= P; p++) {
    string line;
    getline(ans, line);
    istringstream iss(line);

    vector<int64_t> vec;
    int v;
    while (iss >> v) {
      vec.push_back(v);
    }

    if (p == P && !vec.empty()) {
      throw;
    }

    if (vec.size() % 3 != 0) throw;
    for (int j = 0; j < vec.size(); j += 3) {
      auto pr = make_pair(vec[j], vec[j + 1]);
      if (!mp.count(pr) || vec[j + 2] <= 0) {
        throw;
      }
      max_value = std::max<int64_t>(max_value, vec[j + 2]);
      min_value = std::min<int64_t>(min_value, vec[j + 2]);
      int r = mp[pr];

      if (!can_add(p, r, vec[j + 2])) {
        // cerr << vec[j] << " " << vec[j + 1] << " " << vec[j + 2] << endl;
        cerr << r << " " << solution.bought_packages[r] << " " << vec[j + 2] << " " << regions[r].available_packages << endl;
        throw;
      }
      add(p, r, vec[j + 2]);
    }
  }
}
