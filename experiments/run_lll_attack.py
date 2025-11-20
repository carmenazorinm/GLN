#!/usr/bin/env python3
# gln_lattice_attack_grid.py
# Ejecuta con: sage -python llll_attack_robust.py --exe ../build/experiment_g_conditions --preset demo --out results.csv

import argparse, json, subprocess, sys, csv, time, itertools, math


try:
    from sage.all import matrix, block_matrix, identity_matrix, zero_matrix, ZZ
except Exception:
    print("Ejecuta con Sage: sage -python gln_lattice_attack_grid.py ...", file=sys.stderr)
    sys.exit(1)

def calculate_beta(n):
    return 2 + math.ceil(math.log2(n))

PRESETS = {
    "demo": {
        "ns":   [64],
        "ts":   [8, 10, 15,30,40,50],
        "zs":   [1<<14, 1<<18, 1<<22],
        "betas":[3],
        "seeds":[13,124,198,176,109,122,456,768,776,1255],        
        "use_sum": True,           
        "A": None                 
    },
    "n50": {
        "ns":   [50],
        "ts":   [0.4*50,0.6*50,0.8*50],
        "zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "betas": [3],
        "seeds": [13,124,198,176,109,122,456,768,776,1255],        
        "use_sum": True,           
        "A": None                 
    }
    ,
    "n50_extra": {
        "ns":   [50],
        "ts":   [0.1*50,0.15*50,0.2*50,0.25*50,0.3*50,0.35*50,0.4*50,0.45*50,0.5*50,0.55*50,0.6*50,0.65*50, 0.7*50,0.75*50,0.8*50,0.85*50,0.9*50,0.95*50],
        "zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "betas": [1,2,3,4,5],
        "seeds": [13,124,198,176,109,122,456,768,776,1255],        
        "use_sum": True,           
        "A": None                 
    },
    "n100": {
        "ns":   [100],
        "ts":   [0.4*100,0.6*100,0.8*100],
        "zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "betas": [3],
        "seeds": [13,124,198,176,109,122,456,768,776,1255],        
        "use_sum": True,           
        "A": None                 
    },
    "n100_extra": {
        "ns":   [100],
        "ts":   [0.1*100,0.15*100,0.2*100,0.25*100,0.3*100,0.35*100,0.4*100,0.45*100,0.5*100,0.55*100,0.6*100,0.65*100, 0.7*100,0.75*100,0.8*100,0.85*100,0.9*100,0.95*100],
        "zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "betas": [1,2,3,4,5],
        "seeds": [13,124,198,176,109,122,456,768,776,1255],        
        "use_sum": True,           
        "A": None                 
    },
    "n200": {
        "ns":   [200],
        "ts":   [0.4*200,0.6*200,0.8*200],
        "zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "betas": [3],
        "seeds": [13,124,198,176,109,122,456,768,776,1255],        
        "use_sum": True,           
        "A": None                 
    },
    "n75_extra": {
        "ns":   [75],
        "ts":   [0.1*75,0.15*75,0.2*75,0.25*75,0.3*75,0.35*75,0.4*75,0.45*75,0.5*75,0.55*75,0.6*75,0.65*75, 0.7*75,0.75*75,0.8*75,0.85*75,0.9*75,0.95*75],
        "zs": [pow(2,10),pow(2,12),pow(2,14),pow(2,16),pow(2,18),pow(2,20),pow(2,22),pow(2,24),pow(2,26),pow(2,28),pow(2,30),pow(2,32),pow(2,34),pow(2,36),pow(2,38),pow(2,40)],
        "betas": [1,2,3,4,5],
        "seeds": [13,124,198,176,109,122,456,768,776,1255],        
        "use_sum": True,           
        "A": None                 
    },
    
}

# ---------------------------
# Binario C++ -> instancia
# ---------------------------
def run_cpp_and_get_instance(exe, n, t, z, beta, seed):
    """
    Se asume que el binario imprime JSON con:
      { "h":[...], "ciphertext": {"c1":"...", "c2": ...}, "plaintext":[...]?, "d": <densidad> }
    """
    cmd = [
        exe,
        "--n", str(n),
        "--t", str(t),
        "--z", str(z),
        "--beta", str(beta),
        "--seed", str(seed),
        "--json",
        "--quiet"
    ]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        raise RuntimeError(f"binario fallo: {res.stderr.strip()}")
    data = json.loads(res.stdout)

    h = [int(x) for x in data["h"]]
    c1 = int(data["ciphertext"]["c1"])
    c2 = int(data["ciphertext"]["c2"])
    pt = data.get("plaintext")
    if pt is not None:
        pt = [int(v) for v in pt]
    density = float(data.get("d", "nan"))
    return h, c1, c2, pt, density

# ---------------------------
# Checks
# ---------------------------
def check_alphabet(vec, A):
    return all(0 <= int(v) <= int(A) for v in vec)

def count_weight(vec):
    return sum(1 for v in vec if int(v) != 0)

# ---------------------------
# Ataques
# ---------------------------
def lll_attack_two_constraints(h, c1, c2, t, A):
    """
    Base por columnas con dos restricciones:
        B = [[ I_n ,     0  ],
             [  h  ,   -c1  ],
             [  1  ,   -c2  ]]
    """
    n = len(h)
    I = identity_matrix(ZZ, n)
    zero_col = zero_matrix(ZZ, n, 1)
    h_row = matrix(ZZ, [h])
    c1_cell = matrix(ZZ, [[-int(c1)]])
    ones_row = matrix(ZZ, [[1]*n])
    c2_cell = matrix(ZZ, [[-int(c2)]])
    B = block_matrix([[I,        zero_col],
                      [h_row,    c1_cell ],
                      [ones_row, c2_cell ]])

    t0 = time.time()
    R = (B.transpose()).LLL(algorithm='NTL:LLL')
    elapsed = time.time() - t0

    for row in R.rows():
        x = [int(row[i]) for i in range(n)]
        if not check_alphabet(x, A):     continue
        if count_weight(x) != int(t):    continue
        if sum(x) != int(c2):            continue
        if sum(x[j]*int(h[j]) for j in range(n)) == int(c1):
            return x, elapsed
    return None, elapsed

def lll_attack_one_constraint(h, c1, t, A, c2=None):
    """
    Base con una sola restriccion:
        B = [[ I_n ,  0 ],
             [  h  , -c1]]
    """
    n = len(h)
    I = identity_matrix(ZZ, n)
    zero_col = zero_matrix(ZZ, n, 1)
    h_row = matrix(ZZ, [h])
    c1_cell = matrix(ZZ, [[-int(c1)]])
    B = block_matrix([[I,        zero_col],
                      [h_row,    c1_cell ]])

    t0 = time.time()
    R = (B.transpose()).LLL(algorithm='NTL:LLL')
    elapsed = time.time() - t0

    for row in R.rows():
        x = [int(row[i]) for i in range(n)]
        if not check_alphabet(x, A):     continue
        if count_weight(x) != int(t):    continue
        if c2 is not None and sum(x) != int(c2):  continue
        if sum(x[j]*int(h[j]) for j in range(n)) == int(c1):
            return x, elapsed
    return None, elapsed

def try_attack(h, c1, c2, t, A, use_sum=True):
    """
    Devuelve: (x_rec, method, attack_time_s)
    method in {"two", "one", "none"}
    """
    total = 0.0
    if use_sum:
        x, t_a = lll_attack_two_constraints(h, c1, c2, t, A)
        total += t_a
        if x is not None:
            return x, "two", total
    x, t_b = lll_attack_one_constraint(h, c1, t, A, c2=c2)
    total += t_b
    if x is not None:
        return x, "one", total
    return None, "none", total

# ---------------------------
# Grid runner
# ---------------------------
def calculate_beta(n):
    return 2 + math.ceil(math.log2(n))

def run_one(exe, n, t, z, beta, seed, use_sum, A=None):
    A_eff = (z - 1) if A is None else A

    h, c1, c2, pt_true, density = run_cpp_and_get_instance(exe, n, t, z, beta, seed)

    x_rec, method, attack_time_s = try_attack(h, c1, c2, t, A_eff, use_sum=use_sum)

    success = x_rec is not None
    weight_ok = sum_ok = eq_ok = pt_match = False
    if success:
        weight_ok = (count_weight(x_rec) == t)
        sum_ok    = (sum(x_rec) == c2)
        eq_ok     = (sum(x_rec[i]*h[i] for i in range(len(h))) == c1)
        pt_match  = (pt_true is not None and pt_true == x_rec)

    row = {
        "n": n, "t": t, "z": z, "beta": beta, "seed": seed,
        "A": A_eff, "density": density, "len_h": len(h),
        "attack_success": int(success),
        "weight_ok": int(weight_ok), "sum_ok": int(sum_ok), "eq_ok": int(eq_ok),
        "pt_match": (int(pt_match) if pt_true is not None else ""),
        "attack_time_s": f"{attack_time_s:.6f}",
        "method": method
    }
    return row

def write_csv_header(writer):
    writer.writerow([
        "n","t","z","beta","seed","A","density","len_h",
        "attack_success","weight_ok","sum_ok","eq_ok","pt_match",
        "attack_time_s","method"
    ])

def main():
    ap = argparse.ArgumentParser(description="Barrido de ataque Lagarias-Odlyzko para GLN con presets y CSV")
    ap.add_argument("--exe", required=True, help="ruta al binario C++ que imprime JSON")
    ap.add_argument("--preset", default=None, help="nombre del preset (ver PRESETS)")
    ap.add_argument("--out", required=True, help="ruta del CSV de salida")
    # modo single-run opcional (si no usas preset)
    ap.add_argument("--n", type=int)
    ap.add_argument("--t", type=int)
    ap.add_argument("--z", type=int)
    ap.add_argument("--beta", type=int)
    ap.add_argument("--seed", type=int, default=12345)
    ap.add_argument("--A", type=int, default=None)
    ap.add_argument("--no-sum", action="store_true")
    args = ap.parse_args()

    rows = []
    if args.preset:
        if args.preset not in PRESETS:
            print(f"Preset '{args.preset}' no existe. Edita PRESETS en el script.", file=sys.stderr)
            sys.exit(2)
        P = PRESETS[args.preset]
        ns = P["ns"]; ts = P["ts"]; zs = P["zs"]; betas = P["betas"]; seeds = P.get("seeds", [args.seed])
        use_sum = bool(P.get("use_sum", True))
        A_preset = P.get("A", None)
        total_cfg = len(ns)*len(ts)*len(zs)*len(betas)*len(seeds)
        print(f"[info] Ejecutando preset '{args.preset}' con {total_cfg} configuraciones...")
        for n, t, z, beta, seed in itertools.product(ns, ts, zs, betas, seeds):
            print(f"n: {n} - t: {t} - z: {z} ")
            try:
                row = run_one(args.exe, n, t, z, calculate_beta(n), seed, use_sum=use_sum, A=A_preset)
            except Exception as e:
                row = {
                    "n": n, "t": t, "z": z, "beta": calculate_beta(n), "seed": seed,
                    "A": (z-1 if A_preset is None else A_preset),
                    "density": "",
                    "len_h": "",
                    "attack_success": 0,
                    "weight_ok": 0, "sum_ok": 0, "eq_ok": 0, "pt_match": "",
                    "attack_time_s": "", "method": f"error: {str(e)[:80]}"
                }
            rows.append(row)
    else:
        # single run
        if None in (args.n, args.t, args.z, args.beta):
            print("Faltan n,t,z,beta o usa --preset", file=sys.stderr)
            sys.exit(2)
        try:
            row = run_one(args.exe, args.n, args.t, args.z, args.beta, args.seed, use_sum=(not args.no_sum), A=args.A)
        except Exception as e:
            row = {
                "n": args.n, "t": args.t, "z": args.z, "beta": args.beta, "seed": args.seed,
                "A": (args.z-1 if args.A is None else args.A),
                "density": "",
                "len_h": "",
                "attack_success": 0,
                "weight_ok": 0, "sum_ok": 0, "eq_ok": 0, "pt_match": "",
                "attack_time_s": "", "method": f"error: {str(e)[:80]}"
            }
        rows.append(row)

    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        write_csv_header(w)
        for r in rows:
            w.writerow([
                r["n"], r["t"], r["z"], r["beta"], r["seed"], r["A"], r["density"], r["len_h"],
                r["attack_success"], r["weight_ok"], r["sum_ok"], r["eq_ok"], r["pt_match"],
                r["attack_time_s"], r["method"]
            ])

    print(f"[ok] Guardado CSV en {args.out} con {len(rows)} filas.")

if __name__ == "__main__":
    main()
