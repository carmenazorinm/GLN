# GLN: Knapsack-based Public Key Cryptosystem

## Descripción

GLN es un criptosistema de clave pública basado en mochila que utiliza el **Weighted Modular Subset Sum Problem (WMSSP)** como base para su seguridad.

El nombre GLN proviene de sus autores originales: **Gómez-Torrecillas, Lobillo y Navarro**.

Este repositorio contiene una implementación en C++ de las siguientes tareas:

- Generación de parámetros seguros  
- Generación de claves  
- Cifrado de mensajes  
- Descifrado de mensajes  
- Ataque por tríos (ataque a la clave privada)

---

## Compilación

Clonar el repositorio junto con los submódulos, generar la build y compilar:

```bash
git clone --recurse-submodules https://github.com/carmenazorinm/GLN.git
cd GLN
cmake -S . -B build
cmake --build build -j
```

Ejecutar los tests:
```bash
ctest --test-dir build
```

---

## Experimento principal: experiment_g_conditions

El ejecutable principal es experiment_g_conditions. A partir de unos parámetros n, t, z y beta, el programa:

- genera parámetros y claves

- cifra y descifra un mensaje aleatorio

- mide los tiempos de cada fase

- calcula varias métricas (tamaño de la clave pública, número de bits de seguridad, densidad, etc.)

Ejemplo de ejecución:
```bash
cd build
./experiment_g_conditions --n 10 --t 3 --z 1024 --beta 1 --seed 123
```

Salida típica:
```bash
Parameters: n=10 t=3 z=1024 beta=1 |g|≈71 bits seed=123
KeyGen completed in 0.002536 s
Plaintext (sparse) generated (showing non-zero entries): [2:404, 4:568, 9:956]
Encryption completed in 0.000006 s
Ciphertext: c1 = 946181556283540302817300  c2 = 1928
Decryption completed in 0.000203 s
Recovered plaintext non-zero entries: [2:404, 4:568, 9:956]
[OK] decrypt(encrypt(e)) == e 
SUMMARY: n=10 t=3 z=1024 g_bits=22 c1_bits=24 c2_bits=4 pubkey_bits=210 keygen_s=0.002536 enc_s=0.000006 dec_s=0.000203 ok=1 n_primes=120.871613 density=0.140845
```

### Opciones disponibles

Las opciones completas se pueden consultar con --help:
```bash
Usage: ./experiment_g_conditions [options]

Options:
  --n <int>         length n (default 32)
  --t <int>         weight t (default 6)
  --z <int>         alphabet size z (default 256)
  --beta <int>      beta parameter for primes (default 3)
  --seed <int>      RNG seed (default 42)

Attack (triples) options:
  --attack          run attack_triples from gln/attacks.hpp

Output options:
  --csv             print a machine-friendly CSV result line (one-liner)
  --quiet           reduce human-readable output (still prints CSV if --csv)
  --help            show this help

Example:
  ./experiment_g_conditions --n 100 --t 10 --z 1024 --beta 3 --seed 12345 --csv
```

---

## Script de experimentos: run_experiments.py

Para lanzar múltiples experimentos con distintas configuraciones de parámetros se proporciona el script de Python experiments/run_experiments.py.

Ejemplo de uso con el preset mini:
```bash
cd experiments
python3 run_experiments.py --exe ../build/experiment_g_conditions --mode preset --preset mini
```

Salida típica:
```bash
[1/6] n=32 t=6 z=256 beta=7 keygen=0.008395 enc=9e-06 dec=0.000348 g_bits=55 pubkey_bits=1757 c1_bits=58 c2_bits=3 ok=True rc=0
[2/6] n=32 t=6 z=256 beta=7 keygen=0.007529 enc=9e-06 dec=0.000322 g_bits=56 pubkey_bits=1759 c1_bits=58 c2_bits=4 ok=True rc=0
[3/6] n=32 t=6 z=256 beta=7 keygen=0.007343 enc=9e-06 dec=0.000364 g_bits=56 pubkey_bits=1763 c1_bits=58 c2_bits=3 ok=True rc=0
[4/6] n=64 t=6 z=256 beta=8 keygen=0.023208 enc=9e-06 dec=0.000465 g_bits=59 pubkey_bits=3758 c1_bits=62 c2_bits=4 ok=True rc=0
[5/6] n=64 t=6 z=256 beta=8 keygen=0.022321 enc=1.3e-05 dec=0.000851 g_bits=59 pubkey_bits=3752 c1_bits=62 c2_bits=3 ok=True rc=0
[6/6] n=64 t=6 z=256 beta=8 keygen=0.02261 enc=1.2e-05 dec=0.000831 g_bits=59 pubkey_bits=3759 c1_bits=62 c2_bits=3 ok=True rc=0
CSV crudo: results.csv
CSV agregado: results_aggregated.csv
```

### Uso del script
```
usage: run_experiments.py [-h] --exe EXE --mode {sweep,grid,preset}
                          [--param {n,t,z,beta}] [--preset {mini,fast,big}]
                          [--Ns NS] [--Ts TS] [--Zs ZS] [--Betas BETAS]
                          [--base-n BASE_N] [--base-t BASE_T]
                          [--base-z BASE_Z] [--base-beta BASE_BETA]
                          [--runs RUNS] [--seed SEED] [--threads THREADS]
                          [--out OUT] [--out-agg OUT_AGG] [--plots PLOTS]

Runner de experimentos GLN con --check-g

options:
  -h, --help            show this help message and exit
  --exe EXE             Ruta del ejecutable, ej: ~/experiments/experiment_g_conditions
  --mode {sweep,grid,preset}
  --param {n,t,z,beta}  Parámetro a barrer en sweep
  --preset {mini,fast,big}
  --Ns NS               lista n separada por comas
  --Ts TS               lista t separada por comas
  --Zs ZS               lista z separada por comas
  --Betas BETAS         lista beta separada por comas
  --base-n BASE_N
  --base-t BASE_T
  --base-z BASE_Z
  --base-beta BASE_BETA
  --runs RUNS
  --seed SEED
  --threads THREADS
  --out OUT
  --out-agg OUT_AGG
  --plots PLOTS
```

El preset mini definido en el script es:
```
"mini": {
    "Ns": [32, 64],
    "Ts": [6],
    "Zs": [256],
    "Betas": [3],
    "mode": "sweep",
    "param": "n",
    "runs": 2
}
```

Esto significa:

- Se ejecuta el binario indicado en --exe para todas las combinaciones de parámetros listadas.

- Con mode = "sweep" y param = "n" se realiza un barrido del parámetro n.

- runs = 2 indica que cada configuración se repite dos veces.

El script también puede:

- generar gráficas con la opción --plots directorio_plots

- ejecutarse en paralelo con la opción --threads N

Por defecto se usa 1 thread y se realizan 3 ejecuciones (runs=3) por configuración si no se indica lo contrario.

## Ataque LLL: run_lll_attack.py

Para experimentar con ataques de baja densidad (tipo Lagarias–Odlyzko) sobre GLN se proporciona el script run_lll_attack.py, que llama al binario C++ y usa SageMath para aplicar LLL.

### Uso:

```
usage: run_lll_attack.py [-h] --exe EXE [--preset PRESET] --out OUT
                         [--n N] [--t T] [--z Z] [--beta BETA] [--seed SEED]
                         [--A A] [--no-sum]

Barrido de ataque Lagarias-Odlyzko para GLN con presets y CSV

options:
  -h, --help       show this help message and exit
  --exe EXE        ruta al binario C++ que imprime JSON
  --preset PRESET  nombre del preset (demo, n50, ...)
  --out OUT        ruta del CSV de salida
  --n N
  --t T
  --z Z
  --beta BETA
  --seed SEED
  --A A
  --no-sum
```

Ejemplo de uso:
```
cd experiments
sage -python llll_attack_robust.py --exe ../build/experiment_g_conditions --preset demo --out results.csv
```

Salida típica:
```
[info] Ejecutando preset 'demo' con 6 configuraciones...
n: 64 - t: 8 - z: 16384 
    - attack success: 0
n: 64 - t: 10 - z: 16384 
    - attack success: 0
n: 64 - t: 15 - z: 16384 
    - attack success: 0
n: 64 - t: 30 - z: 16384 
    - attack success: 1
n: 64 - t: 40 - z: 16384 
    - attack success: 1
n: 64 - t: 50 - z: 16384 
    - attack success: 1
[ok] Guardado CSV en ./results/results.csv con 6 filas.
```

Este script permite:

- explorar distintos parámetros n, t, z, beta

- medir el éxito del ataque en función de la densidad de la instancia

- guardar los resultados en CSV para análisis posterior
