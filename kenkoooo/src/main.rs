use std::collections::BTreeMap;
use std::io::BufRead;

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

    let mut regions = BTreeMap::new();
    for provider_id in 0..provider_num {
        let _provider_name: String = sc.read();
        let region_num: usize = sc.read();
        for region_id in 0..region_num {
            let _region_name: String = sc.read();
            let available: i64 = sc.read();
            let cost: f64 = sc.read();
            let units: Vec<i64> = sc.vec(service_num);
            let latencies: Vec<i64> = sc.vec(country_num);
            regions.insert(
                (provider_id, region_id),
                Region {
                    id: (provider_id, region_id),
                    available,
                    cost,
                    units,
                    latencies,
                },
            );
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
}

fn calc_score(
    project: &Project,
    ans: &[((usize, usize), i64)],
    regions: &BTreeMap<(usize, usize), Region>,
) -> f64 {
    let service_num = project.units.len();

    let mut cost_sum = 0.0;
    let mut services = vec![];
    let mut latency_sum = 0;
    let mut unit_sum = 0;

    for &(id, count) in ans.iter() {
        let region = &regions[&id];
        let latency = region.latencies[project.country];
        let bought_services = region.units.iter().map(|&t| t * count).collect::<Vec<_>>();
        let cost = region.cost * count as f64;
        cost_sum += cost;

        let units = bought_services.iter().sum::<i64>();
        unit_sum += units;

        latency_sum += latency * units;
        services.push(bought_services);
    }

    let average_latency = latency_sum as f64 / unit_sum as f64;
    let mut availability_indices = vec![];
    for service_id in 0..service_num {
        let mut sum = 0;
        let mut square_sum = 0;
        for s in services.iter() {
            let unit = s[service_id];
            sum += unit;
            square_sum += unit * unit;
        }

        if square_sum == 0 {
            availability_indices.push(0.0);
        } else {
            availability_indices.push((sum * sum) as f64 / square_sum as f64);
        }
    }

    let availability_index = availability_indices.iter().sum::<f64>() / service_num as f64;

    let mut sla_penalties = vec![];
    for service_id in 0..service_num {
        let required = project.units[service_id];
        if required == 0 {
            sla_penalties.push(0.0);
        } else {
            let unit_sum = services.iter().map(|s| s[service_id]).sum::<i64>();
            let penalty = (required - unit_sum.min(required)) as f64 / required as f64;
            sla_penalties.push(penalty * project.penalty as f64);
        }
    }

    let sla_penalty = sla_penalties.iter().sum::<f64>() / sla_penalties.len() as f64;
    let score = 1e9 / (cost_sum * average_latency / availability_index.max(1.0) + sla_penalty);
    score
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
