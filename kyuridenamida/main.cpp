#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

#ifdef CLION
//std::ifstream ifs("/home/kyuridenamida/hashcode/reply2018-practice/input/example.in");
//string name = "first_adventure";
//string name = "second_adventure";
string name = "third_adventure";
//string name = "fourth_adventure";

std::ifstream ifs("/home/kyuridenamida/hashcode/reply2018-practice/input/" + name + ".in");
std::ofstream ofs("/home/kyuridenamida/hashcode/reply2018-practice/kyuridenamida/output/" + name + ".out");
#define scanf DONT_USE_SCANF
#define cin ifs
#define cout ofs
#endif

int numProvider, numService, numCountry, numProject, numRegions;
vector<string> serviceNames;
vector<string> countryNames;
map<pair<int, int>, int> rainbow;


struct Region {
    int id;
    int availablePackages;
    double packageFee;
    vector<int> unitByService;
    vector<int> latencyByCountry;
    int providerId;
    int localId;
};

struct Provider {
    int id;
    string name;
    vector<Region *> regions;
};

struct OutputItem {
    Provider *provider;
    Region *region;;
    int packageNum;
};

struct Project {
    int id;
    double basePenalty;
    int countryId;

    bool operator<(const Project &rhs) const {
        if (id < rhs.id)
            return true;
        if (rhs.id < id)
            return false;
        if (basePenalty < rhs.basePenalty)
            return true;
        if (rhs.basePenalty < basePenalty)
            return false;
        if (countryId < rhs.countryId)
            return true;
        if (rhs.countryId < countryId)
            return false;
        return neededUnitByService < rhs.neededUnitByService;
    }

    bool operator>(const Project &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const Project &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const Project &rhs) const {
        return !(*this < rhs);
    }

    vector<int> neededUnitByService;
};


double averageProjectLatency(const vector<OutputItem> &items, const Project &project) {
    vector<long long> denMap(numRegions);
    vector<long long> numMap(numRegions);
    double num = 0;
    double den = 0;
    for (auto i : items) {
        int amount = 0;
        for (int s = 0; s < numService; s++) {
            amount += i.region->unitByService[s];
        }
        num += amount * i.packageNum * i.region->latencyByCountry[project.countryId];
        den += amount * i.packageNum;
    }
    if (den == 0) {
        return 0;
    }
    return num / den;
}

double overallAvailabilityIndex(const vector<OutputItem> &items) {
    vector<long long> denMap(numService);
    vector<long long> numMap(numService);
    for (auto i : items) {
        for (int s = 0; s < numService; s++) {
            int x = i.region->unitByService[s] * i.packageNum;
            numMap[s] += x;
            denMap[s] += x * x;
        }
    }
    double ans = 0;
    for (int i = 0; i < numService; i++) {
        if (denMap[i] != 0) {
            ans += 1. * numMap[i] * numMap[i] / denMap[i];
        }
    }
    ans /= numService;
    return ans;
}

double operationalProjectCost(const vector<OutputItem> &items) {
    double ans = 0;
    for (auto i : items) {
        ans += i.region->packageFee * i.packageNum;
    }
    return ans;
}

double projectPenalty(const vector<OutputItem> &items, const Project &project) {
    vector<int> counter(numService);
    for (auto i : items) {
        for (int s = 0; s < numService; s++) {
            counter[s] += i.region->unitByService[s] * i.packageNum;
        }
    }
    double ans = 0;
    for (int i = 0; i < numService; i++) {
        if (project.neededUnitByService[i] > 0) {
            ans += 1. * (project.neededUnitByService[i] - min(project.neededUnitByService[i], counter[i])) /
                   project.neededUnitByService[i];
        }
    }
    return project.basePenalty * 1. * ans / numService;
}

double score(const vector<OutputItem> &items, const Project &project) {
    double av = overallAvailabilityIndex(items);
    double Tp = 0;
    if (av != 0) {
        Tp = averageProjectLatency(items, project) / overallAvailabilityIndex(items) * operationalProjectCost(items);
    }
    double Fp = projectPenalty(items, project);
    return 1e9 / (Tp + Fp);
}

struct Output {
    vector<OutputItem> items;
};


vector<Project> projects;

vector<Provider *> providers;
vector<Region *> allRegions;

vector<Output> readOutput() {
    std::ifstream output_ifs("/home/kyuridenamida/hashcode/reply2018-practice/output/example.out");

    string line;
    vector<Output> ops;
    while (getline(output_ifs, line) && !line.empty()) {
        vector<OutputItem> items;
        int a, b, c;
        stringstream ss(line);
        while (ss >> a >> b >> c) {
            items.push_back({providers[a], allRegions[rainbow.find({a, b})->second], c});
        }
        ops.push_back({items});

    }
    return ops;
}


Output finishProject(Project &project, vector<int> &regionUsed) {

    vector<int> has(numService);
    vector<Region *> regs = allRegions;

    auto eval = [&](const Region *region) -> double {
        if (regionUsed[region->id] == region->availablePackages) {
            return 1e9;
        }

        int meaningful = 0;
        for (int i = 0; i < numService; i++) {
            meaningful += min(region->unitByService[i], max(0, project.neededUnitByService[i] - has[i]));
        }
        if (meaningful == 0) {
            return 1e9;
        }
        return region->packageFee / meaningful;
    };


    vector<int> totUsed(numRegions);
    auto buy = [&](const Region *region) {
        regionUsed[region->id]++;
        for (int i = 0; i < numService; i++) {
            has[i] += region->unitByService[i];
        }
        totUsed[region->id]++;
    };
    int iter = 0;
    while (true) {
        vector<double> cost(numRegions);
        for (int i = 0; i < numRegions; i++) {
            cost[regs[i]->id] = eval(regs[i]);
        }
        sort(regs.begin(), regs.end(), [&](const Region *a, const Region *b) {
            return cost[a->id] < cost[b->id];
        });
        if (cost[regs[0]->id] == 1e9) {
            break;
        }
        buy(regs[0]);
        iter++;


    }
    vector<OutputItem> items;
    for (int i = 0; i < numRegions; i++) {
        if (totUsed[i] > 0) {
            items.push_back({providers[allRegions[i]->providerId], allRegions[i], totUsed[i]});
        }
    }


    return {items};
}

int main() {
    cin >> numProvider >> numService >> numCountry >> numProject;
    for (int i = 0; i < numService; i++) {
        string s;
        cin >> s;
        serviceNames.push_back(s);
    }


    for (int i = 0; i < numCountry; i++) {
        string c;
        cin >> c;
        countryNames.push_back(c);
    }
    int regionId = 0;
    vector<string> providerNames;
    vector<string> regionNames;
    for (int i = 0; i < numProvider; i++) {
        string pName;
        cin >> pName;
        providerNames.push_back(pName);
        int numSubRegion;
        cin >> numSubRegion;
        vector<Region *> regions;
        for (int r = 0; r < numSubRegion; r++) {
            string region;
            cin >> region;
            regionNames.push_back(region);
            int availablePackages;
            cin >> availablePackages;
            double packageFee;
            cin >> packageFee;
            vector<int> units;
            for (int j = 0; j < numService; j++) {
                int x;
                cin >> x;
                units.push_back(x);
            }
            vector<int> latencies;
            for (int j = 0; j < numCountry; j++) {
                int x;
                cin >> x;
                latencies.push_back(x);
            }
            rainbow[{i, r}] = regionId;
            regions.push_back(new Region{regionId++, availablePackages, packageFee, units, latencies, i, r});
        }
        allRegions.insert(allRegions.end(), regions.begin(), regions.end());
        providers.push_back(new Provider{i, pName, regions});
    }
    numRegions = regionId;

    for (int i = 0; i < numProject; i++) {
        double penalty;
        cin >> penalty;
        string country;
        cin >> country;
        int countryId = find(countryNames.begin(), countryNames.end(), country) - countryNames.begin();
        vector<int> units;
        for (int j = 0; j < numService; j++) {
            int x;
            cin >> x;
            units.push_back(x);
        }
        projects.push_back({i, penalty, countryId, units});
    }


//    auto outputs = readOutput();

//
//    for (int i = 0; i < numProject; i++) {
//        auto r = finishProject(projects[i], regionUsed);
//        for (auto item : r.items) {
////            cout << providerNames[item.provider->id] << " " << regionNames[item.region->id] << " " << item.packageNum << endl;
//            regionUsed[item.region->id] += item.packageNum;
//        }
//
////        cout << score(r.items, projects[i]) << "|" << score(outputs[i].items, projects[i])  << endl;
//    }

    vector<pair<double, Project>> scores;
    for (int i = 0; i < numProject; i++) {
        vector<int> regionUsed(numRegions, 0);
        auto r = finishProject(projects[i], regionUsed);
        auto x = score(r.items, projects[i]);
        scores.emplace_back(-x, projects[i]);
    }
    sort(scores.begin(), scores.end());
    double tot = 0;

    vector<int> regionUsed(numRegions, 0);
    vector<Output> pj(numProject);

    for (auto ss : scores) {
        auto r = finishProject(ss.second, regionUsed);
        pj[ss.second.id] = r;
        tot += score(r.items, ss.second);
    }
    cerr << std::setprecision(10) << tot / 1e9 << endl;

    for (auto r: pj) {
        for (auto item : r.items) {
            cout << item.region->providerId << " " << item.region->localId << " " << item.packageNum << " ";
        }
        cout << endl;
    }


    return 0;
}
