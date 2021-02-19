#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <vector>

#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

#include <sa/time_manager.h>

#include "random.h"

using namespace std;

XorShiftL rng;

// input
struct Region {
  int provider_index;
  int region_index;
  int region_id;
  int64_t available_packages;
  int64_t package_unit_cost;
  vector<int64_t> units_of_service_per_package;
  vector<int64_t> latency;

  int64_t sum_units; // units_of_service_per_packageの総和
};

struct Project {
  int project_id;
  int64_t penalty;
  int country;
  vector<int64_t> units;
};

struct Solution {
  vector<int64_t> bought_packages; // bought_packages[i]  : region i が購入された総パッケージ数

  // 購入
  vector<vector<uint16_t>> buy; // buy[i][j] : Project[i] が region j から買ったユニット数

  template <class Archive>
  void save(Archive &ar) const
  {
    ar(bought_packages, buy);
  }

  template <class Archive>
  void load(Archive &ar)
  {
    ar(bought_packages, buy);
  }
};

Solution solution;

int S, C, P, R;
map<string, int> country;
vector<Region> regions;
vector<Project> projects;

struct ProjectScore {
  int64_t average_latency_numerator;
  int64_t average_latency_denominator;
  double overall_availability_index;
  int64_t operational_project_cost;
  vector<int64_t> sum_units;

  vector<vector<uint16_t>> q;
  vector<vector<uint16_t>> q2;
  vector<int64_t> sum_q;
  vector<int64_t> sum_q2;
  double fp;

  template <class Archive>
  void save(Archive &ar) const
  {
    ar(average_latency_numerator, average_latency_denominator, overall_availability_index,
       operational_project_cost, sum_units, q, q2, sum_q, sum_q2, fp);
  }

  template <class Archive>
  void load(Archive &ar)
  {
    ar(average_latency_numerator, average_latency_denominator, overall_availability_index,
       operational_project_cost, sum_units, q, q2, sum_q, sum_q2, fp);
  }

  ProjectScore()
  {
    average_latency_numerator = 0;
    average_latency_denominator = 0;

    overall_availability_index = 0;
    operational_project_cost = 0;

    overall_availability_index = 0;
    operational_project_cost = 0;
    q.assign(S, vector<uint16_t>(R));
    q2.assign(S, vector<uint16_t>(R));
    sum_q.resize(S);
    sum_q2.resize(S);
  }

  void init(Project &proj)
  {
    update_fp(proj);
  }

  void update_fp(Project &proj)
  {
    fp = 0;
    for (int i = 0; i < S; i++) {
      if (proj.units[i] == 0) continue;

      int64_t unit = proj.units[i] - std::min<int64_t>(proj.units[i], sum_q[i]);
      double fs = proj.penalty * unit / double(proj.units[i]);

      // cerr << __LINE__ << " " << i << " " << fs << " " << proj.units[i] << " " << sum_units[i] << endl;
      fp += fs;
    }
    fp /= S;
    fp *= 100; // tp側が100倍になっているので、それにあわせるために100倍
  }

  void add(Project &proj, Region &region, int64_t delta)
  {
    auto &bought = solution.bought_packages[region.region_id];
    auto &buy = solution.buy[proj.project_id][region.region_id];

    if (delta == 0 || delta + int64_t(buy) < 0 || delta + bought > region.available_packages) {
      return;
    }

    bought += delta;
    buy += delta;

    auto u = region.sum_units * delta;
    average_latency_numerator += region.latency[proj.country] * u;
    average_latency_denominator += u;

    if (average_latency_denominator < 0) throw;

    overall_availability_index = 0;
    for (int i = 0; i < S; i++) {
      auto d = delta * region.units_of_service_per_package[i];

      auto &v = q[i][region.region_id];
      sum_q[i] -= int64_t(v);
      sum_q2[i] -= int64_t(v) * int64_t(v);

      v += d;

      sum_q[i] += int64_t(v);
      sum_q2[i] += int64_t(v) * int64_t(v);

      if (sum_q2[i] > 0) {
        overall_availability_index += sum_q[i] * sum_q[i] / double(sum_q2[i]);
      }
    }

    overall_availability_index /= S;

    // Operational Project Cost
    operational_project_cost += delta * region.package_unit_cost;

    update_fp(proj);
  }

  double get_score()
  {
    double average_project_latency = 0;
    if (average_latency_denominator > 0) {
      average_project_latency = double(average_latency_numerator) / average_latency_denominator;
    }

    double tp = 0;
    if (overall_availability_index > 0) {
      tp = average_project_latency / overall_availability_index * operational_project_cost;
    }

    double score = 1e9 / (tp + fp) * 1e2;
    // if (std::isnan(score)) {
    if (!std::isfinite(score)) {
      cerr << __LINE__ << ": " << average_latency_numerator << " " << average_latency_denominator << " " << average_project_latency << endl;
      cerr << __LINE__ << ": " << overall_availability_index << endl;
      cerr << __LINE__ << ": " << operational_project_cost << endl;
      cerr << __LINE__ << ": " << fp << endl;
      cerr << __LINE__ << ": " << tp << endl;
      cerr << __LINE__ << ": " << score << endl;
    }

    return score;
  }
};

vector<ProjectScore> scores;

void input()
{
  string tmp;

  int V;
  cin >> V >> S >> C >> P;

  for (int i = 0; i < S; i++) {
    cin >> tmp;
  }

  for (int i = 0; i < C; i++) {
    cin >> tmp;
    country[tmp] = i;
  }

  R = 0;
  for (int i = 0; i < V; i++) {
    int num_regions;
    cin >> tmp >> num_regions;
    for (int j = 0; j < num_regions; j++) {
      cin >> tmp;

      Region region;
      region.provider_index = i;
      region.region_index = j;
      region.region_id = R;

      double cost;
      cin >> region.available_packages >> cost;
      if (region.available_packages > 65000) throw;
      region.package_unit_cost = int64_t(cost * 100 + 0.5);

      region.units_of_service_per_package.resize(S);
      region.sum_units = 0;
      for (int k = 0; k < S; k++) {
        cin >> region.units_of_service_per_package[k];
        region.sum_units += region.units_of_service_per_package[k];
      }

      region.latency.resize(C);
      for (int k = 0; k < C; k++) {
        cin >> region.latency[k];
      }

      regions.push_back(region);
      R++;
    }
  }

  solution.bought_packages.resize(R);
  solution.buy.assign(P, vector<uint16_t>(R));

  projects.resize(P);
  scores.resize(P);
  for (int i = 0; i < P; i++) {
    projects[i].project_id = i;

    cin >> projects[i].penalty;

    cin >> tmp;
    if (!country.count(tmp)) {
      throw;
    }

    projects[i].country = country[tmp];
    projects[i].units.resize(S);
    for (int j = 0; j < S; j++) {
      cin >> projects[i].units[j];
    }
    scores[i].init(projects[i]);
  }
}

void dump_service_distribution()
{
  // サービス
  cout << "project\tservice\tunits" << endl;
  for (int i = 0; i < P; i++) {
    for (int j = 0; j < S; j++) {
      cout << i << "\t" << j << "\t" << projects[i].units[j] << endl;
    }
  }
}

int main()
{
  cin.tie(0);
  ios::sync_with_stdio(false);
  input();

  // dump_service_distribution();
  // return 0;

  auto add = [](int p, int r, int delta) {
    scores[p].add(projects[p], regions[r], delta);
  };

  auto can_add = [](int p, int r, int delta) {
    auto b = solution.bought_packages[r] + delta;
    return int64_t(solution.buy[p][r]) + delta >= 0 && b >= 0 && b <= regions[r].available_packages;
  };

  // {
  //   std::ifstream is("out.cereal", std::ios::binary);
  //   if (is.is_open()) {
  //     cerr << "loading" << endl;
  //     cereal::BinaryInputArchive archive(is);
  //     archive(solution, scores);
  //   }
  // }

  // double score = 0;
  // for (int i = 0; i < P; i++) {
  //   // if (i != 0) continue;
  //   // cerr << i << " " << scores[i].get_score() << endl;
  //   score += scores[i].get_score();
  //   // score += scores[i].get_score2();
  // }

  // int iter = 0;
  // for (auto c : candidates) {
  //   int p, r;
  //   double _;
  //   tie(p, r, _) = c;
  //   double cur_score = scores[p].get_score();
  //   double prev_score = cur_score;
  //   int v = 0;
  //   while (1) {
  //     add(p, r, 1);
  //     double new_score = scores[p].get_score();
  //     if (v++ > 0 && cur_score >= new_score) {
  //       add(p, r, -1);
  //       break;
  //     }
  //     cur_score = new_score;
  //   }
  //   score += cur_score - prev_score;

  //   if (iter++ % 1000000 == 0) {
  //     cerr << "cur score " << score << endl;
  //   }
  // }

  // for (int delta = 1024; delta > 0; delta /= 2) {
  // for (int delta = 1; delta > 0; delta /= 2) {
  //   for (int i = 0; i < 200000000 / 10; i++) {
  //     while (1) {
  //       int pi = rng.nextInt(P);
  //       int ri = rng.nextInt(R);
  //       int d = rng.nextInt(11) - 6;
  //       if (d != 0 && can_add(pi, ri, d)) {
  //         double cur_score = scores[pi].get_score();
  //         add(pi, ri, d);
  //         double new_score = scores[pi].get_score();
  //         // if (cur_score - 100000 > new_score) { // 1
  //         // if (cur_score - 1000 > new_score) { // 2
  //         // if (cur_score - delta > new_score) { // 2
  //         if (cur_score - delta > new_score) { // 2
  //           add(pi, ri, -d);
  //         } else {
  //           score -= cur_score;
  //           score += new_score;
  //         }

  //         if (i % 100000 == 0) {
  //           cerr << "cur score " << i << " " << score << endl;
  //         }

  //         // if ((i + 1) % 10000000 == 0) {
  //         //   cerr << "saving" << endl;
  //         //   std::ofstream os("out.cereal", std::ios::binary);
  //         //   cereal::BinaryOutputArchive archive(os);
  //         //   archive(solution, scores);
  //         // }

  //         break;
  //       }
  //     }
  //   }
  // }

  // vector<double> project_bests;
  // for (int project = 0; project < P; project++) {
  //   const int p = project;

  //   TemperatureManager temp_manager(100, 0);
  //   ConstantTimeManager time_manager(2000000);

  //   double cur_score = scores[p].get_score();
  //   while (1) {
  //     auto t = time_manager.get_time();

  //     if (t >= 1) {
  //       break;
  //     }

  //     auto temp = temp_manager.get_temp(t);
  //     while (1) {
  //       int r = rng.nextInt(R);
  //       int d = rng.nextInt(13) - 6;

  //       if (regions[r].latency[projects[p].country] >= 1500) {
  //         continue;
  //       }

  //       if (d != 0 && can_add(p, r, d)) {
  //         add(p, r, d);
  //         double new_score = scores[p].get_score();

  //         double diff = -(new_score - cur_score);
  //         if (diff <= -temp * rng.nextLog()) {
  //           cur_score = new_score;
  //         } else {
  //           add(p, r, -d);
  //         }

  //         // if ((i + 1) % 10000000 == 0) {
  //         //   cerr << "saving" << endl;
  //         //   std::ofstream os("out.cereal", std::ios::binary);
  //         //   cereal::BinaryOutputArchive archive(os);
  //         //   archive(solution, scores);
  //         // }

  //         break;
  //       }
  //     }
  //   }

  //   cerr << p << ": " << cur_score << endl;
  //   project_bests.push_back(cur_score);

  //   int used_region_count = 0;
  //   for (int r = 0; r < R; r++) {
  //     if (solution.buy[p][r]) {
  //       used_region_count++;
  //     }
  //     add(p, r, -solution.buy[p][r]);
  //   }
  //   cerr << used_region_count << " / " << R << endl;
  // }

  vector<double> cur_scores(P);
  vector<pair<int64_t, int>> unit_project_pair;
  for (int p = 0; p < P; p++) {
    cur_scores[p] = scores[p].get_score();
    int64_t sum_unit = std::accumulate(projects[p].units.begin(), projects[p].units.end(), int64_t(0));
    unit_project_pair.emplace_back(sum_unit, p);
  }
  sort(begin(unit_project_pair), end(unit_project_pair));

  for (int pi = 0; pi < P; pi++) {
    const int p = unit_project_pair[pi].second;
    TemperatureManager temp_manager(10, 0);
    ConstantTimeManager time_manager(100000 * 2);

    for (int64_t iter = 0;; iter++) {
      auto t = time_manager.get_time();

      if (t >= 1) {
        break;
      }

      auto temp = temp_manager.get_temp(t);
      // int p = rng.nextInt(P);
      int r = rng.nextInt(R);
      int d = rng.nextInt(13) - 6;

      if (regions[r].latency[projects[p].country] >= 1500) {
        continue;
      }

      if (d != 0 && can_add(p, r, d)) {
        add(p, r, d);
        double new_score = scores[p].get_score();

        double diff = -(new_score - cur_scores[p]);
        if (diff <= -temp * rng.nextLog()) {
          cur_scores[p] = new_score;
        } else {
          add(p, r, -d);
        }

        // if ((i + 1) % 10000000 == 0) {
        //   cerr << "saving" << endl;
        //   std::ofstream os("out.cereal", std::ios::binary);
        //   cereal::BinaryOutputArchive archive(os);
        //   archive(solution, scores);
        // }
      }

      // if (iter % 1000000 == 0) {
      //   auto sum_score = std::accumulate(cur_scores.begin(), cur_scores.end(), int64_t(0));
      //   cerr << iter << " " << t << " " << sum_score << endl;
      // }
    }

    auto sum_score = std::accumulate(cur_scores.begin(), cur_scores.end(), int64_t(0));
    cerr << pi << " " << cur_scores[p] << " " << sum_score << endl;
  }

  {
    cerr << "saving" << endl;
    std::ofstream os("out.cereal", std::ios::binary);
    cereal::BinaryOutputArchive archive(os);
    archive(solution, scores);
  }

  // cerr << score << endl;

  // vector<int> used(R);
  for (int i = 0; i < P; i++) {
    bool first = true;
    for (int j = 0; j < R; j++) {
      // if (i != 0) continue;
      // if (solution.buy[i][j] < 0) throw;
      // used[j] += solution.buy[i][j];
      // cerr << used[j] << " " << solution.bought_packages[j] << " " << regions[j].available_packages << endl;
      // if (used[j] > regions[j].available_packages) throw;
      if (solution.buy[i][j] > 0) {
        if (!first) cout << " ";
        first = false;
        cout << regions[j].provider_index << " " << regions[j].region_index << " " << solution.buy[i][j];
      }
    }
    cout << endl;
  }

  // for (int i = 0; i < R; i++) {
  //   cerr << used[i] << " " << solution.bought_packages[i] << " " << regions[i].available_packages << endl;
  //   if (used[i] != solution.bought_packages[i]) throw;
  //   if (solution.bought_packages[i] > regions[i].available_packages) throw;
  // }
}
