use rand::distributions::Uniform;
use rand::{thread_rng, Rng};
use std::collections::BTreeMap;
use std::time::Instant;

fn main() {
    let (r, w) = (std::io::stdin(), std::io::stdout());
    let mut sc = IO::new(r.lock(), w.lock());

    let provider_num: usize = sc.read();
    let service_num: usize = sc.read();
    let country_num: usize = sc.read();
    let project_num: usize = sc.read();

    let _services: Vec<String> = sc.vec(service_num);
    let countries: Vec<String> = sc.vec(country_num);
    let countries =
        countries
            .into_iter()
            .enumerate()
            .fold(BTreeMap::new(), |mut map, (index, country)| {
                map.insert(country, index);
                map
            });

    let mut regions = vec![];
    let mut region_id_map = BTreeMap::new();
    for provider_id in 0..provider_num {
        let _provider_name: String = sc.read();
        let region_num: usize = sc.read();
        for region_id in 0..region_num {
            let _region_name: String = sc.read();
            let available: i64 = sc.read();
            let cost: f64 = sc.read();
            let units: Vec<i64> = sc.vec(service_num);
            let latencies: Vec<i64> = sc.vec(country_num);
            regions.push(Region {
                id: (provider_id, region_id),
                available,
                cost,
                units,
                latencies,
            });
            region_id_map.insert((provider_id, region_id), regions.len() - 1);
        }
    }

    let mut projects = vec![];
    for _ in 0..project_num {
        let penalty: i64 = sc.read();
        let country: String = sc.read();
        let country = countries[&country];
        let units: Vec<i64> = sc.vec(service_num);
        projects.push(Project {
            penalty,
            country,
            units,
        });
    }

    let project_states = solve(&projects, &regions);
    for project_id in 0..project_num {
        for (i, (&region_id, &count)) in project_states[project_id].bought_count.iter().enumerate()
        {
            if i > 0 {
                print!(" ");
            }
            let (pid, rid) = regions[region_id].id;
            print!("{} {} {}", pid, rid, count);
        }
        println!();
    }
}

fn solve(projects: &[Project], regions: &[Region]) -> Vec<ProjectState> {
    let service_num = regions[0].units.len();
    let region_num = regions.len();
    let project_num = projects.len();
    let mut project_states = vec![];
    let mut cur_scores = vec![];
    let mut total_score = 0.0;
    for project in projects {
        project_states.push(ProjectState {
            cost_sum: 0.0,
            unit_sum: 0,
            latency_sum: 0,
            bought_count: BTreeMap::new(),
            bought_units: vec![0; service_num],
            availability_sum: vec![0; service_num],
            availability_square_sum: vec![0; service_num],
        });
        let initial_score = 1e9 / project.penalty as f64;
        cur_scores.push(initial_score);
        total_score += initial_score;
    }

    let mut remain_packages = vec![];
    for region in regions {
        remain_packages.push(region.available);
    }

    let mut rng = thread_rng();
    let region_dist = Uniform::from(0..region_num);
    let project_dist = Uniform::from(0..project_num);

    let start = Instant::now();
    let mut prev = Instant::now();
    for turn in 1.. {
        let mut candidate = None;
        let mut increase_score = 0.0;

        for _ in 0..TRIAL {
            let region_id = rng.sample(region_dist);
            if remain_packages[region_id] == 0 {
                continue;
            }

            let region = &regions[region_id];
            let project_id = rng.sample(project_dist);
            let project = &projects[project_id];
            let (next_state, next_score) =
                project_states[project_id].update(region_id, region, project);
            let cur_score = cur_scores[project_id];
            let d_score = next_score - cur_score;
            if increase_score < d_score {
                increase_score = d_score;
                candidate = Some((next_state, project_id, region_id));
            }
        }

        if let Some((next_state, project_id, region_id)) = candidate {
            cur_scores[project_id] += increase_score;
            total_score += increase_score;
            project_states[project_id] = next_state;
            assert!(remain_packages[region_id] > 0);
            remain_packages[region_id] -= 1;
        } else {
            break;
        }

        if prev.elapsed().as_millis() > 1000 {
            eprintln!("turn={} total_score={}", turn, total_score);
            prev = Instant::now();
        }

        if start.elapsed().as_millis() > TIME_LIMIT {
            break;
        }
    }

    eprintln!("total_score={}", total_score);
    project_states
}

const TRIAL: usize = 100000;
const TIME_LIMIT: u128 = 30_000;

struct ProjectState {
    cost_sum: f64,
    unit_sum: i64,
    latency_sum: i64,
    bought_count: BTreeMap<usize, i64>,
    bought_units: Vec<i64>,
    availability_sum: Vec<i64>,
    availability_square_sum: Vec<i64>,
}

impl ProjectState {
    fn update(&self, region_id: usize, region: &Region, project: &Project) -> (ProjectState, f64) {
        let service_num = region.units.len();
        let add_unit_sum = region.units.iter().sum::<i64>();

        let country = project.country;
        let cost_sum = self.cost_sum + region.cost;
        let unit_sum = self.unit_sum + add_unit_sum;
        let latency_sum = self.latency_sum + add_unit_sum * region.latencies[country];
        let average_latency = latency_sum as f64 / unit_sum as f64;
        let mut bought_units = self.bought_units.clone();
        for service_id in 0..service_num {
            bought_units[service_id] += region.units[service_id];
        }

        let mut sla_penalty_sum = 0.0;
        for service_id in 0..service_num {
            let required = project.units[service_id];
            if required != 0 {
                let lack = (required - bought_units[service_id]).max(0);
                let penalty = lack as f64 / required as f64;
                sla_penalty_sum += penalty * project.penalty as f64;
            }
        }
        let sla_penalty = sla_penalty_sum / service_num as f64;

        let mut availability_sum = self.availability_sum.clone();
        let mut availability_square_sum = self.availability_square_sum.clone();
        for service_id in 0..service_num {
            let cur_count = bought_units[service_id];
            let prev_count = cur_count - region.units[service_id];

            availability_sum[service_id] -= prev_count;
            availability_sum[service_id] += cur_count;

            availability_square_sum[service_id] -= prev_count * prev_count;
            availability_square_sum[service_id] += cur_count * cur_count;
        }

        let mut availability_index = 0.0;
        for service_id in 0..service_num {
            let sum = availability_sum[service_id];
            let square_sum = availability_square_sum[service_id];
            availability_index += (sum * sum) as f64 / square_sum as f64;
        }

        availability_index /= service_num as f64;

        let mut bought_count = self.bought_count.clone();
        *bought_count.entry(region_id).or_insert(0) += 1;
        let score = 1e9 / (cost_sum * average_latency / availability_index.max(1.0) + sla_penalty);
        let next_state = ProjectState {
            cost_sum,
            unit_sum,
            latency_sum,
            bought_count,
            bought_units,
            availability_sum,
            availability_square_sum,
        };

        (next_state, score)
    }
}

#[derive(Debug)]
struct Project {
    penalty: i64,
    country: usize,
    units: Vec<i64>,
}

#[derive(Debug)]
struct Region {
    id: (usize, usize),
    available: i64,
    cost: f64,
    units: Vec<i64>,
    latencies: Vec<i64>,
}

pub struct IO<R, W: std::io::Write>(R, std::io::BufWriter<W>);

impl<R: std::io::Read, W: std::io::Write> IO<R, W> {
    pub fn new(r: R, w: W) -> Self {
        Self(r, std::io::BufWriter::new(w))
    }
    pub fn write<S: ToString>(&mut self, s: S) {
        use std::io::Write;
        self.1.write_all(s.to_string().as_bytes()).unwrap();
    }
    pub fn read<T: std::str::FromStr>(&mut self) -> T {
        use std::io::Read;
        let buf = self
            .0
            .by_ref()
            .bytes()
            .map(|b| b.unwrap())
            .skip_while(|&b| b == b' ' || b == b'\n' || b == b'\r' || b == b'\t')
            .take_while(|&b| b != b' ' && b != b'\n' && b != b'\r' && b != b'\t')
            .collect::<Vec<_>>();
        unsafe { std::str::from_utf8_unchecked(&buf) }
            .parse()
            .ok()
            .expect("Parse error.")
    }
    pub fn vec<T: std::str::FromStr>(&mut self, n: usize) -> Vec<T> {
        (0..n).map(|_| self.read()).collect()
    }
    pub fn chars(&mut self) -> Vec<char> {
        self.read::<String>().chars().collect()
    }
}
