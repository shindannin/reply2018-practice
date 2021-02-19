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

void input(std::istream &in)
{
  string tmp;

  int V;
  in >> V >> S >> C >> P;

  for (int i = 0; i < S; i++) {
    in >> tmp;
  }

  for (int i = 0; i < C; i++) {
    in >> tmp;
    country[tmp] = i;
  }

  R = 0;
  for (int i = 0; i < V; i++) {
    int num_regions;
    in >> tmp >> num_regions;
    for (int j = 0; j < num_regions; j++) {
      in >> tmp;

      Region region;
      region.provider_index = i;
      region.region_index = j;
      region.region_id = R;

      double cost;
      in >> region.available_packages >> cost;
      if (region.available_packages > 65000) throw;
      region.package_unit_cost = int64_t(cost * 100 + 0.5);

      region.units_of_service_per_package.resize(S);
      region.sum_units = 0;
      for (int k = 0; k < S; k++) {
        in >> region.units_of_service_per_package[k];
        region.sum_units += region.units_of_service_per_package[k];
      }

      region.latency.resize(C);
      for (int k = 0; k < C; k++) {
        in >> region.latency[k];
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

    in >> projects[i].penalty;

    in >> tmp;
    if (!country.count(tmp)) {
      throw;
    }

    projects[i].country = country[tmp];
    projects[i].units.resize(S);
    for (int j = 0; j < S; j++) {
      in >> projects[i].units[j];
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

int main(int argc, char *argv[])
{
  cin.tie(0);
  ios::sync_with_stdio(false);

  std::ifstream in(argv[1]);
  input(in);

  {
    std::ifstream is("out.cereal", std::ios::binary);
    if (is.is_open()) {
      cerr << "loading" << endl;
      cereal::BinaryInputArchive archive(is);
      archive(solution, scores);
    }
  }

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
