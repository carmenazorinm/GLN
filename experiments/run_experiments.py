#!/usr/bin/env python3
"""
Cómo ejeuctarlo:
  python3 gln_runner.py --exe ~/experiments/experiment_g_conditions \
    --mode sweep --param n --Ns 32,64,128,256 --base-t 6 --base-z 256 --base-beta 3 \
    --runs 3 --threads 4 --out results.csv --plots plots

  python3 gln_runner.py --exe ~/experiments/experiment_g_conditions \
    --mode grid --Ns 64,128 --Ts 6,10 --Zs 256,1024 --Betas 3,4 \
    --runs 3 --threads 8 --out results.csv --plots plots

  python3 gln_runner.py --exe ~/experiments/experiment_g_conditions \
    --mode preset --preset mini --runs 2 --threads 4 --out results.csv
"""
import argparse
import csv
import itertools
import math
import os
import re
import shlex
import statistics
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed

# ---------------- presets ----------------
PRESETS = {
    "mini": {
        "Ns": [32, 64],
        "Ts": [6],
        "Zs": [256],
        "Betas": [3],
        "mode": "sweep",
        "param": "n",
        "runs": 2
    },
    "fast": {
        "Ns": [64, 128, 256],
        "Ts": [6, 10],
        "Zs": [256, 1024],
        "Betas": [3],
        "mode": "grid",
        "runs": 3
    },
    "big": {
        "Ns": [50, 75, 100,150,200,250,300],
        "Ts": [5,10,20,30,40,50],
        "Zs": [256, 1024, 2048, 4096],
        "Betas": [1],
        "mode": "grid",
        "runs": 3
    },
    "density_time_sweep": {
        "Ns": [50],
        "Ts": [0.2*50,0.35*50,0.5*50,0.6*50,0.7*50,0.8*50],
        "Zs": [256],
        "Betas": [3],
        "mode": "sweep",
        "param": "t",
        "runs": 10,
    },
    "density_time_sweep_n100": {
        "Ns": [100],
        "Ts": [0.2*100,0.35*100,0.5*100,0.6*100,0.7*100,0.8*100],
        "Zs": [256],
        "Betas": [3],
        "mode": "sweep",
        "param": "t",
        "runs": 10,
    },
    "density_time_sweep_n200": {
        "Ns": [200],
        "Ts": [0.2*200,0.35*200,0.5*200,0.6*200,0.7*200,0.8*200],
        "Zs": [256],
        "Betas": [3],
        "mode": "sweep",
        "param": "t",
        "runs": 10,
    },
    "density_time_sweep_n400": {
        "Ns": [400],
        "Ts": [0.2*400,0.35*400,0.5*400,0.6*400,0.7*400,0.8*400],
        "Zs": [256],
        "Betas": [1],
        "mode": "sweep",
        "param": "t",
        "runs": 10,
    },
    "density_zetas_sweep_n100_b40": {
        "Ns": [100],
        "Ts": [0.4*100],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density_zetas_sweep_n50_b40": {
        "Ns": [50],
        "Ts": [0.4*50],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density_zetas_sweep_n200_b40": {
        "Ns": [200],
        "Ts": [0.4*200],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density_zetas_sweep_n400_b40": {
        "Ns": [400],
        "Ts": [0.4*400],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density0.6_zetas_sweep_n100_b40": {
        "Ns": [100],
        "Ts": [0.6*100],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density0.6_zetas_sweep_n50_b40": {
        "Ns": [50],
        "Ts": [0.6*50],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density0.6_zetas_sweep_n200_b40": {
        "Ns": [200],
        "Ts": [0.6*200],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density0.6_zetas_sweep_n400_b40": {
        "Ns": [400],
        "Ts": [0.6*400],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },

    "density0.8_zetas_sweep_n100_b40": {
        "Ns": [100],
        "Ts": [0.8*100],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density0.8_zetas_sweep_n50_b40": {
        "Ns": [50],
        "Ts": [0.8*50],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density0.8_zetas_sweep_n200_b40": {
        "Ns": [200],
        "Ts": [0.8*200],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    },
    "density0.8_zetas_sweep_n400_b40": {
        "Ns": [400],
        "Ts": [0.8*400],
        "Zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "Betas": [3],
        "mode": "sweep",
        "param": "z",
        "runs": 10,
    }
}

# CSV,n,t,z,beta,seed,c1_bits,c2_bits,pubkey_bits,time_keygen_s,time_encrypt_s,time_decrypt_s,success
CSV_LINE_RE = re.compile(
    r"^CSV,(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),([0-9.]+),([0-9.]+),([0-9.]+),([01])$"
)

# SUMMARY: n=.. t=.. z=.. g_bits=.. c1_bits=.. c2_bits=.. pubkey_bits=.. keygen_s=.. enc_s=.. dec_s=.. ok=.
SUMMARY_RE = re.compile(
    r"SUMMARY:.* n=(\d+)\s+t=(\d+)\s+z=(\d+)\s+g_bits=(\d+)\s+"
    r"c1_bits=(\d+)\s+c2_bits=(\d+)\s+pubkey_bits=(\d+)\s+"
    r"keygen_s=([0-9.]+)\s+enc_s=([0-9.]+)\s+dec_s=([0-9.]+)\s+ok=([01])"
)

def parse_exec_output(out: str):
    """Devuelve dict con n,t,z,beta,seed,times,ok,g_bits,c1_bits,c2_bits,pubkey_bits."""
    res = {
        "n": None, "t": None, "z": None, "beta": None, "seed": None,
        "time_keygen": None, "time_encrypt": None, "time_decrypt": None,
        "ok": None,
        "g_bits": None,
        "c1_bits": None, "c2_bits": None, "pubkey_bits": None,
        "raw": out
    }
    for line in out.splitlines():
        line = line.strip()
        m = CSV_LINE_RE.match(line)
        if m:
            res["n"] = int(m.group(1))
            res["t"] = int(m.group(2))
            res["z"] = int(m.group(3))
            res["beta"] = int(m.group(4))
            res["seed"] = int(m.group(5))
            res["g_bits"] = int(m.group(6))
            res["c1_bits"] = int(m.group(7))
            res["c2_bits"] = int(m.group(8))
            res["pubkey_bits"] = int(m.group(9))
            res["time_keygen"] = float(m.group(10))
            res["time_encrypt"] = float(m.group(11))
            res["time_decrypt"] = float(m.group(12))
            res["ok"] = (m.group(13) == "1")
            continue
        m2 = SUMMARY_RE.search(line)
        if m2:
            res["n"] = int(m2.group(1))
            res["t"] = int(m2.group(2))
            res["z"] = int(m2.group(3))
            res["g_bits"] = int(m2.group(4))
            res["c1_bits"] = int(m2.group(5))
            res["c2_bits"] = int(m2.group(6))
            res["pubkey_bits"] = int(m2.group(7))
            res["time_keygen"] = float(m2.group(8))
            res["time_encrypt"] = float(m2.group(9))
            res["time_decrypt"] = float(m2.group(10))
            res["ok"] = (m2.group(11) == "1")
            continue

    if res["time_keygen"] is None:
        m = re.search(r"KeyGen completed in\s+([0-9.]+)\s*s", out)
        if m: res["time_keygen"] = float(m.group(1))
    if res["time_encrypt"] is None:
        m = re.search(r"Encryption completed in\s+([0-9.]+)\s*s", out)
        if m: res["time_encrypt"] = float(m.group(1))
    if res["time_decrypt"] is None:
        m = re.search(r"Decryption completed in\s+([0-9.]+)\s*s", out)
        if m: res["time_decrypt"] = float(m.group(1))

    if res["g_bits"] is None:
        m = re.search(r"g_bits=(\d+)", out)
        if m: res["g_bits"] = int(m.group(1))
    if res["pubkey_bits"] is None:
        m = re.search(r"pubkey_bits=(\d+)", out)
        if m: res["pubkey_bits"] = int(m.group(1))
    if res["c1_bits"] is None:
        m = re.search(r"c1_bits=(\d+)", out)
        if m: res["c1_bits"] = int(m.group(1))
    if res["c2_bits"] is None:
        m = re.search(r"c2_bits=(\d+)", out)
        if m: res["c2_bits"] = int(m.group(1))
    return res

def calculate_t(n):
    return int(k*n/50)

def calculate_beta(n):
    return 2 + math.ceil(math.log2(n))

def run_once(exe, n, t, z, beta, seed, use_csv=True, timeout=900):
    args = ["--n", str(n), "--t", str(t), "--z", str(z), "--beta", str(calculate_beta(n)),
            "--seed", str(seed)]
    if use_csv:
        args.append("--csv")
    cmd = [exe] + args
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    out = proc.stdout.decode("utf-8", errors="replace")
    parsed = parse_exec_output(out)
    parsed.update({"n": n, "t": t, "z": z, "beta": calculate_beta(n), "seed": seed, "rc": proc.returncode})
    return parsed

def median_or_none(xs):
    xs = [x for x in xs if x is not None]
    return statistics.median(xs) if xs else None

# ------------- jobs -------------
def build_jobs(mode, param, Ns, Ts, Zs, Betas, runs, seed):
    seeds = [seed + i for i in range(runs)]
    jobs = []
    if mode == "sweep":
        if param not in {"n","t","z","beta"}:
            raise SystemExit("Con --mode sweep debes indicar --param entre n,t,z,beta")
        base_n, base_t, base_z, base_b = Ns[0], Ts[0], Zs[0], Betas[0]
        values = {"n": Ns, "t": Ts, "z": Zs, "beta": Betas}[param]
        for v in values:
            for s in seeds:
                n, t, z, b = base_n, base_t, base_z, base_b
                if param == "n": n = v
                elif param == "t": t = v
                elif param == "z": z = v
                else: b = v
                jobs.append((n, t, z, b, s))
    else:  # grid
        for (n, t, z, b) in itertools.product(Ns, Ts, Zs, Betas):
            for s in seeds:
                jobs.append((n, t, z, b, s))
    return jobs

# ------------- csv -------------
def write_csv(path, rows, header):
    with open(path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=header)
        w.writeheader()
        for r in rows:
            w.writerow({k: r.get(k) for k in header})

# ------------- plots -------------
def make_plots(plots_dir, aggs, param_for_plots):
    try:
        import matplotlib.pyplot as plt
    except Exception:
        print("matplotlib no disponible, no se generan plots")
        return
    os.makedirs(plots_dir, exist_ok=True)

    # 1) keygen vs param
    xs = [a[param_for_plots] for a in aggs if a["median_keygen_s"] is not None]
    ys = [a["median_keygen_s"] for a in aggs if a["median_keygen_s"] is not None]
    if xs:
        plt.figure()
        plt.scatter(xs, ys)
        plt.xlabel(param_for_plots)
        plt.ylabel("KeyGen time s (mediana)")
        plt.title(f"KeyGen vs {param_for_plots}")
        plt.grid(True)
        plt.savefig(os.path.join(plots_dir, f"keygen_vs_{param_for_plots}.png"), bbox_inches="tight")
        plt.close()

    # 2) pubkey_bits vs param
    xs = [a[param_for_plots] for a in aggs if a["median_pubkey_bits"] is not None]
    ys = [a["median_pubkey_bits"] for a in aggs if a["median_pubkey_bits"] is not None]
    if xs:
        plt.figure()
        plt.scatter(xs, ys)
        plt.xlabel(param_for_plots)
        plt.ylabel("Public key bits (mediana)")
        plt.title(f"Public key size vs {param_for_plots}")
        plt.grid(True)
        plt.savefig(os.path.join(plots_dir, f"pubkey_vs_{param_for_plots}.png"), bbox_inches="tight")
        plt.close()

    # 3) (enc+dec) vs pubkey_bits
    xs = []
    ys = []
    for a in aggs:
        if a["median_pubkey_bits"] is None:
            continue
        total = (a["median_enc_s"] or 0.0) + (a["median_dec_s"] or 0.0)
        xs.append(a["median_pubkey_bits"])
        ys.append(total)
    if xs:
        plt.figure()
        plt.scatter(xs, ys)
        plt.xlabel("Public key bits (mediana)")
        plt.ylabel("Encrypt+Decrypt time s (mediana)")
        plt.title("Encrypt+Decrypt vs public key size")
        plt.grid(True)
        plt.savefig(os.path.join(plots_dir, "encdec_vs_pubkeybits.png"), bbox_inches="tight")
        plt.close()

# ------------- main -------------
def main():
    p = argparse.ArgumentParser(description="Runner de experimentos GLN con --check-g")
    p.add_argument("--exe", required=True, help="Ruta del ejecutable, ej: ~/experiments/experiment_g_conditions")

    # modo
    p.add_argument("--mode", choices=["sweep","grid","preset"], required=True)
    p.add_argument("--param", choices=["n","t","z","beta"], help="Parámetro a barrer en sweep")
    p.add_argument("--preset", choices=list(PRESETS.keys()))

    # listas
    p.add_argument("--Ns", help="lista n separada por comas")
    p.add_argument("--Ts", help="lista t separada por comas")
    p.add_argument("--Zs", help="lista z separada por comas")
    p.add_argument("--Betas", help="lista beta separada por comas")

    # bases para sweep
    p.add_argument("--base-n", type=int, default=128)
    p.add_argument("--base-t", type=int, default=10)
    p.add_argument("--base-z", type=int, default=256)
    p.add_argument("--base-beta", type=int, default=3)

    # control
    p.add_argument("--runs", type=int, default=3)
    p.add_argument("--seed", type=int, default=42)
    p.add_argument("--threads", type=int, default=1)

    # salidas
    p.add_argument("--out", default="results.csv")
    p.add_argument("--out-agg", default=None)
    p.add_argument("--plots", default=None)
    args = p.parse_args()

    # parse listas
    def parse_list(s):
        return [int(x.strip()) for x in s.split(",")] if s else None

    Ns = parse_list(args.Ns)
    Ts = parse_list(args.Ts)
    Zs = parse_list(args.Zs)
    Betas = parse_list(args.Betas)

    # preset
    if args.mode == "preset":
        if not args.preset:
            raise SystemExit("con --mode preset debes indicar --preset")
        pr = PRESETS[args.preset]
        Ns = Ns or pr["Ns"]
        Ts = Ts or pr["Ts"]
        Zs = Zs or pr["Zs"]
        Betas = Betas or pr["Betas"]
        args.mode = pr["mode"]
        if "param" in pr and not args.param:
            args.param = pr["param"]
        if not args.runs:
            args.runs = pr["runs"]

    # defaults si faltan
    Ns = Ns or [args.base_n]
    Ts = Ts or [args.base_t]
    Zs = Zs or [args.base_z]
    Betas = Betas or [args.base_beta]

    # construir jobs
    jobs = build_jobs(args.mode, args.param, Ns, Ts, Zs, Betas, args.runs, args.seed)
    if not jobs:
        print("no hay trabajos para ejecutar", file=sys.stderr)
        sys.exit(1)

    exe_path = os.path.expanduser(args.exe)
    if not os.path.exists(exe_path):
        print(f"ejecutable no encontrado: {exe_path}", file=sys.stderr)
        sys.exit(2)

    # correr
    rows = []
    with ThreadPoolExecutor(max_workers=max(1, args.threads)) as pool:
        futs = [pool.submit(run_once, exe_path, *job) for job in jobs]
        for i, fut in enumerate(as_completed(futs), 1):
            r = fut.result()
            rows.append(r)
            print(f"[{i}/{len(jobs)}] n={r.get('n')} t={r.get('t')} z={r.get('z')} beta={r.get('beta')} "
                  f"keygen={r.get('time_keygen')} enc={r.get('time_encrypt')} dec={r.get('time_decrypt')} g_bits={r.get('g_bits')} "
                  f"pubkey_bits={r.get('pubkey_bits')} c1_bits={r.get('c1_bits')} c2_bits={r.get('c2_bits')} ok={r.get('ok')} rc={r.get('rc')}")

    # CSV crudo
    raw_header = [
        "n","t","z","beta","seed",
        "time_keygen","time_encrypt","time_decrypt","ok",
        "g_bits","pubkey_bits","c1_bits","c2_bits"
    ]
    write_csv(args.out, rows, raw_header)
    print(f"CSV crudo: {args.out}")

    # agregado por configuración (sin seed)
    by_cfg = {}
    for r in rows:
        key = (r.get("n"), r.get("t"), r.get("z"), r.get("beta"))
        by_cfg.setdefault(key, []).append(r)

    aggs = []
    for (n, t, z, b), lst in sorted(by_cfg.items()):
        aggs.append({
            "n": n, "t": t, "z": z, "beta": b,
            "g_bits": median_or_none([x.get("g_bits") for x in lst]),
            "median_keygen_s": median_or_none([x.get("time_keygen") for x in lst]),
            "median_enc_s": median_or_none([x.get("time_encrypt") for x in lst]),
            "median_dec_s": median_or_none([x.get("time_decrypt") for x in lst]),
            "median_pubkey_bits": median_or_none([x.get("pubkey_bits") for x in lst]),
            "max_pubkey_bits": max([x.get("pubkey_bits") for x in lst]),
            "median_c1_bits": median_or_none([x.get("c1_bits") for x in lst]),
            "median_c2_bits": median_or_none([x.get("c2_bits") for x in lst]),
            "ok_rate": median_or_none([1 if x.get("ok") else 0 for x in lst]),
            "runs": len(lst),
        })

    out_agg = args.out_agg or args.out.replace(".csv","") + "_aggregated.csv"
    agg_header = ["n","t","z","beta","g_bits","median_keygen_s","median_enc_s","median_dec_s",
                  "median_pubkey_bits","max_pubkey_bits","median_c1_bits","median_c2_bits","runs","ok_rate"]
    write_csv(out_agg, aggs, agg_header)
    print(f"CSV agregado: {out_agg}")

    if args.plots:
        param_for_plots = args.param if args.mode == "sweep" and args.param else "n"
        aggs_for_plot = []
        for a in aggs:
            d = dict(a)
            d[param_for_plots] = a[param_for_plots]
            aggs_for_plot.append(d)
        make_plots(args.plots, aggs_for_plot, param_for_plots)
        print(f"plots en: {args.plots}")

if __name__ == "__main__":
    main()
