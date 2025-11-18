## Descripción
GLN es un criptosistema basado en mochila que usa el Weighted Modular Subset Sum Problem como base para su segruidad.
Las siglas del criptosistema vienen dadas por los creadores: Gómez-Torrecillas, Lobillo y Navarro.

Este repositorio es una implementación en C++ de las siguientes tareas:
- Generación de parámetros seguros
- Generación de claves
- Encriptación del mensaje
- Decriptación del mensaje
- Ataques por trios

## Uso
Para usar el repositorio:
```
git clone --recurse-submodules https://github.com/carmenazorinm/GLN.git
cd GLN
cmake -S . -B build
cmake --build build -j
```

Para ejecutar los tests:
```
ctest --test-dir build
```

El experimento principal trata de, dados unos parámetross n, t, z y beta, calcular los tiempos de cada tarea. Para ejecutarlo:
```
cd build
./experiment_g_conditions --n 50 --t 10 --z 1024 --beta 1 --seed 123
```

El resultado es el siguiente:
```
Parameters: n=50 t=10 z=1024 beta=1 |g|≈229 bits seed=123
KeyGen completed in 0.031103 s
Plaintext (sparse) generated (showing non-zero entries): [2:699, 5:68, 7:323, 19:1022, 20:89, 26:894, 27:797, 31:220, 39:386, 44:136]
Encryption completed in 0.000018 s
Ciphertext: c1 = 1566225023997457778203886700684872914096425253572333497666374806794181252  c2 = 4634
Decryption completed in 0.000839 s
Recovered plaintext non-zero entries: [2:699, 5:68, 7:323, 19:1022, 20:89, 26:894, 27:797, 31:220, 39:386, 44:136]
[OK] decrypt(encrypt(e)) == e ✅
SUMMARY: n=50 t=10 z=1024 g_bits=69 c1_bits=73 c2_bits=4 pubkey_bits=3442 keygen_s=0.031103 enc_s=0.000018 dec_s=0.000839 ok=1
Cantidad primos disponibles: 2^1.442695 - 2^1.442695 = 120.871613
```

Para distintos tipos de output existen las flags: --csv, --json, --quiet.

Para ejecutar experimentos con distintos parámetros se puede utilizar elscript de python run_experiments.py generado con ayuda de ChatGPT.
```
cd experiments
python3 run_experiments.py --exe ../build/experiment_g_conditions --mode preset --preset mini
```

donde el preset mini se ve en el script como
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
Estos significa que se ejeucta el ejecutable indicado con todas las combinaciones delos parametros indicados. con mode indicamos que estamos haciendo un barrido del parametro n y hacemos este experimento 2 veces indicado con run.
este script también puede generar plots con la flag --plots directorio_plots y se puede ejecutar en varios threads con --thread  n-threads.
Por defecto se ejecuta con 1 cread y se hacen 3 runs de cad configuracion.
