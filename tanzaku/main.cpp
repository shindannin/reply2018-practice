#include <iostream>
#include <map>
#include <vector>

#include "random.h"

using namespace std;

XorShiftL rng;

// input
struct Region {
  int provider_index;
  int region_index;
  int region_id;
  int available_packages;
  double package_unit_cost;
  vector<int> units_of_service_per_package;
  vector<int> latency;

  int sum_units; // units_of_service_per_packageの総和
  int bought_packages;
};

struct Project {
  int penalty;
  int country;
  vector<int> units;

  // 購入
  vector<int> buy; // buy[i] : region i から買ったユニット数
};

int S, C, P, R;
map<string, int> country;
vector<Region> regions;
vector<Project> projects;

struct ProjectScore {
  double average_latency_numerator;
  double average_latency_denominator;
  vector<double> overall_availability_index_sum_q2;
  vector<double> overall_availability_index_sum_q;
  double overall_availability_index = 0;
  double operational_project_cost;
  vector<int> sum_units;
  double fp;

  ProjectScore()
  {
    average_latency_numerator = 0;
    average_latency_denominator = 0;

    overall_availability_index = 0;
    operational_project_cost = 0;

    overall_availability_index_sum_q2.resize(S);
    overall_availability_index_sum_q.resize(S);

    overall_availability_index = 0;
    operational_project_cost = 0;
    sum_units.resize(S);
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

      double unit = proj.units[i] - std::min<int>(proj.units[i], sum_units[i]);
      double fs = proj.penalty * unit / proj.units[i];

      // cerr << __LINE__ << " " << i << " " << fs << " " << proj.units[i] << " " << sum_units[i] << endl;
      fp += fs;
    }
    fp /= S;
  }

  void add(Project &proj, Region &region, int delta)
  {
    if (delta == 0 || delta + region.bought_packages < 0 || delta + region.bought_packages > region.available_packages) {
      return;
    }

    region.bought_packages += delta;
    proj.buy[region.region_id] += delta;

    double u = double(region.sum_units) * delta;
    average_latency_numerator += region.latency[proj.country] * u;
    average_latency_denominator += u;

    overall_availability_index = 0;
    for (int i = 0; i < S; i++) {
      double q = delta * region.units_of_service_per_package[i];
      sum_units[i] += q;
      overall_availability_index_sum_q[i] += q;

      if (delta > 0) {
        overall_availability_index_sum_q2[i] += q * q;
      } else {
        overall_availability_index_sum_q2[i] -= q * q;
      }

      double sum_q = overall_availability_index_sum_q[i];
      double sum_q2 = overall_availability_index_sum_q2[i];
      overall_availability_index += sum_q * sum_q / sum_q2;
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
      average_project_latency = average_latency_numerator / average_latency_denominator;
    }

    // cerr << __LINE__ << ": " << average_latency_numerator << " " << average_latency_denominator << " " << average_project_latency << endl;

    // cerr << __LINE__ << ": " << overall_availability_index << endl;

    // cerr << __LINE__ << ": " << operational_project_cost << endl;
    // cerr << __LINE__ << ": " << fp << endl;

    double tp = 0;
    if (overall_availability_index > 0) {
      tp = average_project_latency / overall_availability_index * operational_project_cost;
    }

    return 1e9 / (tp + fp);
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
      cin >> region.available_packages >> region.package_unit_cost;
      region.units_of_service_per_package.resize(S);
      region.latency.resize(C);
      region.sum_units = 0;
      for (int k = 0; k < S; k++) {
        cin >> region.units_of_service_per_package[k];
        region.sum_units += region.units_of_service_per_package[k];
      }
      for (int k = 0; k < C; k++) {
        cin >> region.latency[k];
      }

      region.bought_packages = 0;
      regions.push_back(region);
      R++;
    }
  }

  projects.resize(P);
  scores.resize(P);
  for (int i = 0; i < P; i++) {
    cin >> projects[i].penalty;

    cin >> tmp;
    projects[i].country = country[tmp];
    projects[i].units.resize(S);
    for (int j = 0; j < S; j++) {
      cin >> projects[i].units[j];
    }
    projects[i].buy.resize(R);
    scores[i].init(projects[i]);
  }
}

double calc_score()
{
  double score = 0;

  for (const auto &p : projects) {
    // Average project latency
    double numerator = 0;
    double denominator = 0;
    for (int i = 0; i < R; i++) {
      auto &r = regions[i];
      double u = double(r.sum_units) * p.buy[i];
      numerator += r.latency[p.country] * u;
      denominator += u;
    }
    double average_project_latency = 0;
    if (denominator > 0) {
      average_project_latency = numerator / denominator;
    }
    // cerr << __LINE__ << ": " << numerator << " " << denominator << " " << average_project_latency << endl;

    // Overall availability index
    double overall_availability_index = 0;
    for (int i = 0; i < S; i++) {
      double sum_q = 0;
      double sum_q2 = 0;
      for (int j = 0; j < R; j++) {
        auto &r = regions[j];
        double q = p.buy[j] * r.units_of_service_per_package[i];
        sum_q += q;
        sum_q2 += q * q;
      }
      overall_availability_index += sum_q * sum_q / sum_q2;
    }
    overall_availability_index /= S;
    // cerr << __LINE__ << ": " << overall_availability_index << endl;

    // Operational Project Cost
    double operational_project_cost = 0;
    for (int j = 0; j < R; j++) {
      auto &r = regions[j];
      operational_project_cost += p.buy[j] * r.package_unit_cost;
    }
    // cerr << __LINE__ << ": " << operational_project_cost << endl;

    double tp = 0;
    if (overall_availability_index > 0) {
      tp = average_project_latency / overall_availability_index * operational_project_cost;
    }

    // SLA penalty
    double fp = 0;
    for (int i = 0; i < S; i++) {
      if (p.units[i] == 0) continue;

      int sum_units = 0;
      for (int j = 0; j < R; j++) {
        auto &r = regions[j];
        sum_units += p.buy[j] * r.units_of_service_per_package[i];
      }

      double unit = p.units[i] - std::min<int>(p.units[i], sum_units);
      double fs = p.penalty * unit / p.units[i];
      fp += fs;
    }
    fp /= S;

    double score_per_project = 1e9 / (tp + fp);
    score += score_per_project;
    // cerr << __LINE__ << ": " << tp << " " << fp << " " << score_per_project << endl;
  }

  // cerr << __LINE__ << ": " << score << endl;
  return score;
}

void example()
{
  // projects[0].buy[0] = 60;
  // projects[0].buy[4] = 1;
  // projects[0].buy[5] = 8;
  // projects[0].buy[6] = 1;
  // projects[0].buy[7] = 10;

  // projects[1].buy[1] = 3;
  // projects[1].buy[3] = 1;
  // projects[1].buy[4] = 5;

  // projects[2].buy[1] = 2;
  // projects[2].buy[3] = 9;
  // projects[2].buy[6] = 1;

  // projects[3].buy[6] = 4;
  // projects[3].buy[7] = 4;

  projects[4].buy[1] = 95;
  projects[4].buy[2] = 10;
  projects[4].buy[4] = 69;
  projects[4].buy[5] = 17;
  projects[4].buy[6] = 24;
  projects[4].buy[7] = 1;
  projects[4].buy[8] = 50;

  calc_score();
}

void example2()
{
  // scores[0].add(projects[0], regions[0], 60);
  // scores[0].add(projects[0], regions[4], 1);
  // scores[0].add(projects[0], regions[5], 8);
  // scores[0].add(projects[0], regions[6], 1);
  // scores[0].add(projects[0], regions[7], 10);

  // scores[1].add(projects[1], regions[1], 3);
  // scores[1].add(projects[1], regions[3], 1);
  // scores[1].add(projects[1], regions[4], 5);

  // scores[2].add(projects[2], regions[1], 2);
  // scores[2].add(projects[2], regions[3], 9);
  // scores[2].add(projects[2], regions[6], 1);

  // scores[3].add(projects[3], regions[6], 4);
  // scores[3].add(projects[3], regions[7], 4);

  // cerr << "====" << endl;
  // for (int i = 0; i < S; i++) {
  //   cerr << projects[4].units[i] << endl;
  // }

  scores[4].add(projects[4], regions[1], 95);
  scores[4].add(projects[4], regions[2], 10);
  scores[4].add(projects[4], regions[4], 69);
  scores[4].add(projects[4], regions[5], 17);
  scores[4].add(projects[4], regions[6], 24);
  scores[4].add(projects[4], regions[7], 1);
  scores[4].add(projects[4], regions[8], 50);

  // cerr << "====" << endl;

  // cout << scores[4].get_score() << endl;

  // for (auto &s : scores) {
  //   cout << s.get_score() << endl;
  // }
}

int main()
{
  cin.tie(0);
  ios::sync_with_stdio(false);
  input();

  // example();
  // example2();

  vector<int> pi;
  for (int i = 0; i < P; i++) {
    pi.push_back(i);
  }

  auto best = projects;
  auto prevProjects = projects;
  auto prevRegions = regions;
  auto prevScores = scores;
  double bestScore = 0;
  for (int iter = 0; iter < 100; iter++) {
    projects = prevProjects;
    regions = prevRegions;
    scores = prevScores;

    rng.shuffle(pi);
    double sc = 0;
    for (int i : pi) {
      for (int k = 0; k < 1000; k++) {
        int j = rng.nextInt(R);
        int need = 0;
        for (int s = 0; s < S; s++) {
          need = std::max(need, regions[j].units_of_service_per_package[s] - scores[i].sum_units[s]);
        }
        need = std::min(need, regions[j].available_packages - regions[j].bought_packages);
        scores[i].add(projects[i], regions[j], need);
      }
      sc += scores[i].get_score();
    }

    if (bestScore < sc) {
      bestScore = sc;
      best = projects;
    }
  }

  for (auto &p : best) {
    bool first = true;
    for (int i = 0; i < R; i++) {
      if (p.buy[i] > 0) {
        if (!first) cout << " ";
        first = false;
        cout << regions[i].provider_index << " " << regions[i].region_index << " " << p.buy[i];
      }
    }
    cout << endl;
  }
}
