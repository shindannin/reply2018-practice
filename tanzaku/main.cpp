#include <iostream>
#include <map>
#include <vector>

using namespace std;

// input
struct Region {
  int provider_id;
  int region_id;
  int available_packages;
  double package_unit_cost;
  vector<int> units_of_service_per_package;
  vector<int> latency;

  int sum_units; // units_of_service_per_packageの総和
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
      R++;
      cin >> tmp;

      Region region;
      region.provider_id = i;
      region.region_id = j;
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

      regions.push_back(region);
    }
  }

  projects.resize(P);
  for (int i = 0; i < P; i++) {
    cin >> projects[i].penalty;

    cin >> tmp;
    projects[i].country = country[tmp];
    projects[i].units.resize(S);
    for (int j = 0; j < S; j++) {
      cin >> projects[i].units[j];
    }
    projects[i].buy.resize(R);
  }
}

double calc_score()
{
  double score = 0;

  int index = 0;
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
  projects[0].buy[0] = 60;
  projects[0].buy[4] = 1;
  projects[0].buy[5] = 8;
  projects[0].buy[6] = 1;
  projects[0].buy[7] = 10;

  projects[1].buy[1] = 3;
  projects[1].buy[3] = 1;
  projects[1].buy[4] = 5;

  projects[2].buy[1] = 2;
  projects[2].buy[3] = 9;
  projects[2].buy[6] = 1;

  projects[3].buy[6] = 4;
  projects[3].buy[7] = 4;

  projects[4].buy[1] = 95;
  projects[4].buy[2] = 10;
  projects[4].buy[4] = 69;
  projects[4].buy[5] = 17;
  projects[4].buy[6] = 24;
  projects[4].buy[7] = 1;
  projects[4].buy[8] = 50;

  calc_score();
}

int main()
{
  cin.tie(0);
  ios::sync_with_stdio(false);
  input();
  example();
}
