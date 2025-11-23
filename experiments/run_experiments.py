#!/usr/bin/env python3
import itertools
import subprocess
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import sys
import math
from pathlib import Path

PRESETS = {
    "big": {
        "Ns":    [50,100,150,200,250,300,350,400],
        "Ws":    [0.2,0.4,0.6,0.8],        # pesos t/n
        "Zs":    [pow(2,10),pow(2,15),pow(2,20),pow(2,25),pow(2,30),pow(2,35),pow(2,40)],
        "Seeds": [123,124,345,543,234,123,987,654,321,259],
    },
    "demo": {
        "Ns":    [50],
        "Ws":    [0.4,0.6],        # pesos t/n
        "Zs":    [pow(2,15),pow(2,20),pow(2,25)],
        "Seeds": [123],
    },
}


def build_command(exe_path, n, t, z, beta, seed):
    return [
        exe_path,
        "--n", str(n),
        "--t", str(t),
        "--z", str(z),
        "--beta", str(beta),
        "--seed", str(seed),
        "--csv",
        "--quiet",
    ]


def run_single_experiment(cmd):
    """
    Ejecuta un experimento (un comando) y devuelve la línea CSV
    que imprime el binario (o None si falla).
    """
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            check=False   # no abortar todo si un experimento falla
        )
    except Exception as e:
        sys.stderr.write(f"[ERROR] Fallo al ejecutar {cmd}: {e}\n")
        return None

    if result.returncode != 0:
        sys.stderr.write(
            f"[WARN] Comando {' '.join(cmd)} devolvió código {result.returncode}\n"
        )
        sys.stderr.write(result.stderr + "\n")
        return None

    lines = [
        line for line in result.stdout.splitlines()
        if line.startswith("CSV,")
    ]
    if not lines:
        sys.stderr.write(
            f"[WARN] No se encontró línea 'CSV,' en la salida de {' '.join(cmd)}\n"
        )
        return None

    return lines[0]


def main():
    parser = argparse.ArgumentParser(
        description="Lanzar experiment_g_conditions en batch y guardar CSV."
    )
    parser.add_argument(
        "--preset",
        required=True,
        choices=PRESETS.keys(),
        help="Nombre del preset a usar (por ejemplo: big)."
    )
    parser.add_argument(
        "--exe",
        default="../build/experiment_g_conditions",
        help="Ruta al ejecutable (por defecto ./experiment_g_conditions)."
    )
    parser.add_argument(
        "--output",
        default="results.csv",
        help="Nombre del fichero CSV de salida."
    )
    parser.add_argument(
        "--threads",
        type=int,
        default=8,
        help="Número máximo de hilos (experimentos en paralelo)."
    )
    args = parser.parse_args()

    preset = PRESETS[args.preset]
    exe_path = args.exe

    # Comprobamos que el ejecutable existe
    if not Path(exe_path).exists():
        sys.stderr.write(f"[ERROR] No se encuentra el ejecutable: {exe_path}\n")
        sys.exit(1)

    Ns    = preset["Ns"]
    Ws    = preset["Ws"]     # pesos t/n
    Zs    = preset["Zs"]
    Seeds = preset["Seeds"]

    # Construimos todas las combinaciones (n, w, z, beta, seed)
    combos = list(itertools.product(Ns, Ws, Zs, Seeds))
    total  = len(combos)

    print(f"Preset '{args.preset}': {total} configuraciones a ejecutar.")
    print(f"Usando hasta {args.threads} hilos.\n")

    commands = []
    for (n, w, z, seed) in combos:
        t = int(w * n)

        if t < 1:
            t = 1
        if t > n:
            t = n
        
        beta = 2+math.ceil(math.log2(n))

        commands.append(build_command(exe_path, n, t, z, beta, seed))

    csv_lines = []
    # Ejecutamos en paralelo
    with ThreadPoolExecutor(max_workers=args.threads) as executor:
        future_to_cmd = {
            executor.submit(run_single_experiment, cmd): cmd
            for cmd in commands
        }

        for i, future in enumerate(as_completed(future_to_cmd), start=1):
            cmd = future_to_cmd[future]
            line = future.result()
            if line is not None:
                csv_lines.append(line)

            # Progreso en stderr
            if i % 50 == 0 or i == total:
                sys.stderr.write(f"[INFO] Completadas {i}/{total} configuraciones\n")

    # Guardamos todas las líneas CSV en el fichero de salida
    if not csv_lines:
        sys.stderr.write("[WARN] No se han obtenido líneas CSV.\n")
    else:
        with open(args.output, "w") as f:
            for line in csv_lines:
                f.write(line.rstrip("\n") + "\n")

        print(f"\nGuardadas {len(csv_lines)} líneas CSV en {args.output}")


if __name__ == "__main__":
    main()
