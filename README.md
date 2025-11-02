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

El experimento principal trata de, dados unos parámetross n, t, z y beta, calcular los tiempos de cada tarea. Pra ejecutarlo:
```
cd build
./experiment_g_conditions --n 50 --t 10 --z 1024 --beta 1 --seed 123 --check-g
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

