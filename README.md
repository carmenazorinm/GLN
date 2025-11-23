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

## Experimento principal: main

El ejecutable principal es main. A partir de unos parámetros n, t, z y beta, el programa:

- genera parámetros y claves

- cifra y descifra un mensaje aleatorio

- mide los tiempos de cada fase

- calcula varias métricas (tamaño de la clave pública, número de bits de seguridad, densidad, etc.)

Ejemplo de ejecución:
```bash
cd build
./main --n 10 --t 3 --z 1024 --beta 1 --seed 123
```

Salida típica:
```bash
Parameters: n=10 t=3 z=1024 beta=1 |g|≈71 bits seed=123
KeyGen completed in 0.002854 s
Plaintext (sparse) generated (showing non-zero entries): [2:404, 4:568, 9:956]
Encryption completed in 0.000006 s
Ciphertext: c1 = 946181556283540302817300  c2 = 1928
Decryption completed in 0.000200 s
Recovered plaintext non-zero entries: [2:404, 4:568, 9:956]
[OK] decrypt(encrypt(e)) == e 
SUMMARY: n=10 t=3 z=1024 g_bits=22 c1_bits=24 c2_bits=4 pubkey_bits=210 keygen_s=0.002854 enc_s=0.000006 dec_s=0.000200 ok=1 n_primes=120.871613 sec_triples=18 sec_brute_force=6 density=0.140845
```

### Opciones disponibles

Las opciones completas se pueden consultar con --help:
```bash
Usage: ./main [options]

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
  ./main --n 100 --t 10 --z 1024 --beta 3 --seed 12345 --csv
```

Esto quiere decir que también se puede ejecutar el ataque por tríos sobre la clave privada. IMPORTANTE: este ataque puede tardar mucho tiempo.

---

## Script de experimentos: run_experiments.py

Para lanzar múltiples experimentos con distintas configuraciones de parámetros se proporciona el script de Python experiments/run_experiments.py.

Ejemplo de uso con el preset mini:
```bash
cd experiments
python3 run_experiments.py --preset demo --output demos_results.csv
```

Salida típica:
```bash
Preset 'demo': 6 configuraciones a ejecutar.
Usando hasta 8 hilos.

[INFO] Completadas 6/6 configuraciones

Guardadas 6 líneas CSV en demos_results.csv
```

### Uso del script
```
usage: experimento.py [-h] --preset {big,demo} [--exe EXE] [--output OUTPUT] [--threads THREADS]

Lanzar main en batch y guardar CSV.

options:
  -h, --help           show this help message and exit
  --preset {big,demo}  Nombre del preset a usar (por ejemplo: big).
  --exe EXE            Ruta al ejecutable (por defecto ../buiild/main).
  --output OUTPUT      Nombre del fichero CSV de salida.
  --threads THREADS    Número máximo de hilos (experimentos en paralelo), por defecto 8.
```

El preset mini definido en el script es:
```
"demo": {
        "Ns":    [50],
        "Ws":    [0.4,0.6],        # pesos t/n
        "Zs":    [pow(2,15),pow(2,20),pow(2,25)],
        "Seeds": [123],
    },
```

El archivo generado como output guarda la salida CSV recibida por el ejecutable en cada configuración probada.


## Ataque LLL: run_lll_attack.py

Para experimentar con ataques de baja densidad (tipo Lagarias–Odlyzko) sobre GLN se proporciona el script run_lll_attack.py, que llama al binario C++ y usa SageMath para aplicar LLL.

Prerrequisito: instalar librería sagemath.
```
pip install sagemath
```


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
sage -python llll_attack_robust.py --exe ../build/main --preset demo --out results.csv
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
