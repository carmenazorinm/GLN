#!/usr/bin/env python3
# gln_lattice_attack_grid.py
# Ejecuta con: sage -python gln_lattice_attack_grid.py ...

import argparse, json, subprocess, sys, csv, time, itertools

try:
    from sage.all import matrix, block_matrix, identity_matrix, zero_matrix, ZZ
except Exception:
    print("Ejecuta con Sage: sage -python gln_lattice_attack_grid.py ...", file=sys.stderr)
    sys.exit(1)

# ---------------------------
# PRESETS: edita aquí lo que quieras barrer
# ---------------------------
PRESETS = {
    "demo": {
        "ns":   [64],
        "ts":   [8, 10, 15,30,40,50],
        "zs":   [1<<14, 1<<18, 1<<22],
        "betas":[3],
        "seeds":[13,124,198,176,109,122,456,768,776,1255],        # puedes añadir más
        "use_sum": True,           # usa restricción de suma
        "A": None                  # si None, se usa A = z-1
    },
    # Ejemplo extra:
    # "bkz_zone": {...}
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
        "--check-g",
        "--json"
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
def run_one(exe, n, t, z, beta, seed, use_sum, A=None):
    # A por defecto: z-1
    A_eff = (z - 1) if A is None else A

    # 1) instancia desde C++
    h, c1, c2, pt_true, density = run_cpp_and_get_instance(exe, n, t, z, beta, seed)

    # 2) ataque
    x_rec, method, attack_time_s = try_attack(h, c1, c2, t, A_eff, use_sum=use_sum)

    # 3) métricas
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
            try:
                row = run_one(args.exe, n, t, z, beta, seed, use_sum=use_sum, A=A_preset)
            except Exception as e:
                row = {
                    "n": n, "t": t, "z": z, "beta": beta, "seed": seed,
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

    # escribir CSV
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
